/*****************************************************************************
 * ijkadkinternal.hpp
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

#ifndef IJKADK__IJKADKINTERNAL_HPP
#define IJKADK__IJKADKINTERNAL_HPP

#include <stdint.h>
#include <jni.h>
#include "ijkutil/ijkutil.h"
#include "ijkadkutils.h"


#define DEPRECATED_IJKADK_FIND_JAVA_CLASS(env__, var__, classsign__) \
    do { \
    	var__ = (*env__)->FindClass(env__, classsign__); \
        if (!(var__)) { \
            ALOGE("FindClass failed: %s", classsign__); \
            return -1; \
        } \
    } while(0);


#define DEPRECATEDIJKADK_FIND_JAVA_METHOD(env__, var__, clazz__, funcsign__, retsign__) \
    do { \
	    (var__) = (*env__)->GetMethodID((env__), (clazz__), (funcsign__), (retsign__)); \
	    if (!(var__)) { \
	    	ALOGE("GetMethodID failed: %s", funcsign__); \
            return -1; \
	    } \
	} while(0);


namespace ijkadk {
class ADKJniClassLoadHelper
{
public:
    ADKJniClassLoadHelper(JNIEnv *env): mEnv(env), mClazz(NULL), mErrorCount(0) {;}

    // return GlobalRef
    jclass findClassAsGlobalRef(const char *signature)
    {
        assert(!mClazz);
        jclass clazz = mEnv->FindClass(signature);
        if (!clazz) {
            ++mErrorCount;
            mClazz = NULL;
        } else {
            mClazz = mEnv->NewGlobalRef(clazz);
        }

        return mClazz;
    }

    jmethodID getMethodID(const char *name, const char *signature)
    {
        jmethodID method = mEnv->GetMethodID(mClazz, name, signature);
        if (!mClazz)
            ++mErrorCount;
        return method;
    }

    int getIntResult()
    {
        if (mErrorCount != 0)
            return -1;

        return 0;
    }

private:
    JNIEnv *mEnv;
    jclass mClazz; // do not own this
    int    mErrorCount;
};
} // end ::ijkadk

#endif /* IJKADK__IJKADKINTERNAL_HPP */
