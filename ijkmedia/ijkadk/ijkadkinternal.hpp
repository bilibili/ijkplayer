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


#define IJKADK_FIND_JAVA_CLASS(env__, var__, classsign__) \
    do { \
    	var__ = (*env__)->FindClass(env__, classsign__); \
        if (!(var__)) { \
            ALOGE("FindClass failed: %s", classsign__); \
            return -1; \
        } \
    } while(0);


#define IJKADK_FIND_JAVA_METHOD(env__, var__, clazz__, funcsign__, retsign__) \
    do { \
	    (var__) = (*env__)->GetMethodID((env__), (clazz__), (funcsign__), (retsign__)); \
	    if (!(var__)) { \
	    	ALOGE("GetMethodID failed: %s", funcsign__); \
            return -1; \
	    } \
	} while(0);


namespace ijkadk {

class IJKLocalRef {
public:
    IJKLocalRef(jobject obj);
    ~IJKLocalRef();
    jobject operator->(){return mObject;}

    void    attach(jobject obj);
    jobject detach();
    jobject get() {return mObject;}
    void    release();

private:
    IJKLocalRef(const IJKLocalRef&);
    IJKLocalRef& operator=(const IJKLocalRef&);


    jobject mObject;
};

} // end ::ijkadk

#endif /* IJKADK__IJKADKINTERNAL_HPP */
