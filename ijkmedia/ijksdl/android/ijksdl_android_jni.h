/*****************************************************************************
 * ijksdl_android_jni.h
 *****************************************************************************
 *
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#ifndef IJKSDL_ANDROID__IJKSDL_ANDROID_JNI_H
#define IJKSDL_ANDROID__IJKSDL_ANDROID_JNI_H

#include <jni.h>
#include "j4a/j4a_base.h"
#include "j4a/j4a_allclasses.h"

#define IJK_API_1_BASE                      1   // 1.0
#define IJK_API_2_BASE_1_1                  2   // 1.1
#define IJK_API_3_CUPCAKE                   3   // 1.5
#define IJK_API_4_DONUT                     4   // 1.6
#define IJK_API_5_ECLAIR                    5   // 2.0
#define IJK_API_6_ECLAIR_0_1                6   // 2.0.1
#define IJK_API_7_ECLAIR_MR1                7   // 2.1
#define IJK_API_8_FROYO                     8   // 2.2
#define IJK_API_9_GINGERBREAD               9   // 2.3
#define IJK_API_10_GINGERBREAD_MR1          10  // 2.3.3
#define IJK_API_11_HONEYCOMB                11  // 3.0
#define IJK_API_12_HONEYCOMB_MR1            12  // 3.1
#define IJK_API_13_HONEYCOMB_MR2            13  // 3.2
#define IJK_API_14_ICE_CREAM_SANDWICH       14  // 4.0
#define IJK_API_15_ICE_CREAM_SANDWICH_MR1   15  // 4.0.3
#define IJK_API_16_JELLY_BEAN               16  // 4.1
#define IJK_API_17_JELLY_BEAN_MR1           17  // 4.2
#define IJK_API_18_JELLY_BEAN_MR2           18  // 4.3
#define IJK_API_19_KITKAT                   19  // 4.4
#define IJK_API_20_KITKAT_WATCH             20  // 4.4W
#define IJK_API_21_LOLLIPOP                 21  // 5.0
#define IJK_API_22_LOLLIPOP_MR1             22  // 5.1
#define IJK_API_23_M                        23  // 6.0

JavaVM *SDL_JNI_GetJvm();

jint    SDL_JNI_SetupThreadEnv(JNIEnv **p_env);
void    SDL_JNI_DetachThreadEnv();

int      SDL_JNI_ThrowException(JNIEnv *env, const char *exception, const char* msg);
int      SDL_JNI_ThrowIllegalStateException(JNIEnv *env, const char* msg);

jobject SDL_JNI_NewObjectAsGlobalRef(JNIEnv *env, jclass clazz, jmethodID methodID, ...);

void    SDL_JNI_DeleteGlobalRefP(JNIEnv *env, jobject *obj_ptr);
void    SDL_JNI_DeleteLocalRefP(JNIEnv *env, jobject *obj_ptr);

int     SDL_Android_GetApiLevel();

#define IJK_FIND_JAVA_CLASS(env__, var__, classsign__) \
    do { \
        jclass clazz = (*env__)->FindClass(env__, classsign__); \
        if (J4A_ExceptionCheck__catchAll(env) || !(clazz)) { \
            ALOGE("FindClass failed: %s", classsign__); \
            return -1; \
        } \
        var__ = (*env__)->NewGlobalRef(env__, clazz); \
        if (J4A_ExceptionCheck__catchAll(env) || !(var__)) { \
            ALOGE("FindClass::NewGlobalRef failed: %s", classsign__); \
            (*env__)->DeleteLocalRef(env__, clazz); \
            return -1; \
        } \
        (*env__)->DeleteLocalRef(env__, clazz); \
    } while(0);

#define JNI_CHECK_GOTO(condition__, env__, exception__, msg__, label__) \
    do { \
        if (!(condition__)) { \
            if (exception__) { \
                SDL_JNI_ThrowException(env__, exception__, msg__); \
            } \
            goto label__; \
        } \
    }while(0)

#define JNI_CHECK_RET_VOID(condition__, env__, exception__, msg__) \
    do { \
        if (!(condition__)) { \
            if (exception__) { \
                SDL_JNI_ThrowException(env__, exception__, msg__); \
            } \
            return; \
        } \
    }while(0)

#define JNI_CHECK_RET(condition__, env__, exception__, msg__, ret__) \
    do { \
        if (!(condition__)) { \
            if (exception__) { \
                SDL_JNI_ThrowException(env__, exception__, msg__); \
            } \
            return ret__; \
        } \
    }while(0)

#endif
