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

namespace ijkadk {

class ADKObject
{
public:
    virtual ~ADKObject();

    virtual unsigned long addRef() = 0;
    virtual unsigned long release() = 0;

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

template <class T>
class ADKObjectImpl: public T
{
public:
    ADKObjectImpl(): mRefCount(0) {;}
    virtual ~ADKObjectImpl() {;}

    unsigned long addRef()
    {
        return __atomic_inc(&mRefCount);
    }

    unsigned long release()
    {
        long ret = __atomic_dec(&mRefCount);
        if (0 == ret)
            delete this;

        return ret;
    }

private:
    volatile int mRefCount;
};

template <class T>
class ADKPtr
{
public:
    ADKPtr(): mObject(0) {;}
    ~ADKPtr() {release();}
    ADKPtr(T* obj)
    {
        mObject = obj;
        if (mObject != NULL)
            mObject->addRef();
    }

    ADKPtr(const ADKPtr<T>& obj)
    {
        mObject = obj.mObject;
        if (mObject != NULL)
            mObject->addRef();
    }

public:
    T &operator*() {return *mObject;}
    T *operator->() {return mObject;}
    ADKPtr<T>& operator=(const ADKPtr<T>& obj)
    {
        attach(obj.get());
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

    void release() {
        if (mObject != NULL) {
            mObject->release();
            mObject = NULL;
        }
    }

private:
    T *mObject;
};

} // end ::ijkadk

#define ADK_OBJ_BEGIN(class__) \
    static class__ *create() \
    { \
        class__ *obj = new ADKObjectImpl<class__>(); \
        if (obj != NULL) { \
            obj->addRef(); \
        } \
        return obj; \
    }

#define ADK_OBJ_END()

#endif /* IJKADK__IJKADKOBJECT_HPP */
