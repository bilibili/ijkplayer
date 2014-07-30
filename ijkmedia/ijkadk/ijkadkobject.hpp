/*****************************************************************************
 * ijkadkobject.hpp
 *****************************************************************************
 *
 * copyright (c) 2013-2014 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef IJKADK__IJKADKOBJECT_HPP
#define IJKADK__IJKADKOBJECT_HPP

#include <stdint.h>
#include <jni.h>
#include <sys/atomics.h>
#include "ijkadkinc.h"
#include "ijkadkutils.h"

namespace ijkadk {

template <class T>
class ADKLocalRef
{
public:
    ~ADKLocalRef() {release();}

    ADKLocalRef(): mObject(0) {;}
    ADKLocalRef(const T& obj): mObject(0)
    {
        attach(obj);
    }

    ADKLocalRef(ADKLocalRef<T>& obj): mObject(0)
    {
        attach(obj.detach());
    }

public:
    bool operator!() {return !mObject;}
    T    operator->() {return mObject;}

    void attach(T obj)
    {
        release();
        mObject = obj;
    }

    T detach()
    {
        T obj = mObject;
        mObject = NULL;
        return obj;
    }

    T get() const
    {
        return mObject;
    }

    void release() {
        JNIEnv *env = ijkadk_get_env();
        if (mObject != NULL) {
            env->DeleteLocalRef(mObject);
            mObject = NULL;
        }
    }

private: // do not overload these operator
    // T &operator*();
    // T **operator&();
    ADKLocalRef<T>& operator=(const ADKLocalRef<T>& obj);
    ADKLocalRef<T>& operator=(T obj);

private:
    T mObject;
};


template <class T>
class ADKPtr
{
public:
    ~ADKPtr() {release();}

    ADKPtr(): mObject(0) {;}
    ADKPtr(T* obj): mObject(0)
    {
        assign(obj);
    }

    ADKPtr(const ADKPtr<T>& obj): mObject(0)
    {
        assign(obj.get());
    }

public:
    bool operator!() {return !mObject;}
    T *operator->() {return mObject;}
    ADKPtr<T>& operator=(const ADKPtr<T>& obj)
    {
        return assign(obj.get());
    }

    ADKPtr<T>& operator=(T *obj)
    {
        return assign(obj);
    }

    ADKPtr<T>& assign(T *obj) {
        if (obj)
            obj->adk_addRef();
        attach(obj);
        return *this;
    }

    void attach(T *obj)
    {
        release();
        mObject = obj;
    }

    T *detach()
    {
        T* obj = mObject;
        mObject = NULL;
        return obj;
    }

    T *get() const
    {
        return mObject;
    }

    const char *getObjectClassName() const
    {
        return mObject->getObjectClassName();
    }

    void release() {
        if (mObject != NULL) {
            mObject->adk_release();
            mObject = NULL;
        }
    }

private: // do not overload these operator
    // T &operator*();
    // T **operator&();

private:
    T *mObject;
};



template <class T>
class ADKObjectImpl: public T
{
public:
    ADKObjectImpl(): mRefCount(0) {;}
    ADKObjectImpl(jobject thiz): mRefCount(0) {T::init(thiz);}

    virtual ~ADKObjectImpl() {;}

    virtual unsigned long adk_addRef()
    {
        return __atomic_inc(&mRefCount);
    }

    virtual unsigned long adk_release()
    {
        long ret = __atomic_dec(&mRefCount);
        if (0 == ret)
            delete this;

        return ret;
    }

    virtual const char *getObjectClassName()
    {
        return T::getClassName();
    }

private:
    volatile int mRefCount;
};



class ADKObject
{
public:
    virtual ~ADKObject();

    virtual unsigned long adk_addRef() = 0;
    virtual unsigned long adk_release() = 0;
    virtual const char   *getObjectClassName() = 0;

protected:
    ADKObject(): mThiz(NULL) {;}
    void init(jobject thiz);

protected:
    static JNIEnv *getJNIEnv();
    jobject getThiz() {return mThiz;}

private:
    jobject mThiz;

private:
    ADKObject(const ADKObject&);
    ADKObject& operator=(const ADKObject&);
};

} // end ::ijkadk

#define ADK_OBJ_BEGIN(class__)                              \
    static ADKPtr<class__> create()                         \
    {                                                       \
        class__ *self = new ADKObjectImpl<class__>();       \
        return self;                                        \
    }                                                       \
    static ADKPtr<class__> create(jobject obj)              \
    {                                                       \
        class__ *self = new ADKObjectImpl<class__>();       \
        self->init(obj);                                    \
        return self;                                        \
    }                                                       \
	static const char *getClassName()                       \
    {                                                       \
        return "";                                          \
    }

#define ADK_OBJ_END()

#endif /* IJKADK__IJKADKOBJECT_HPP */
