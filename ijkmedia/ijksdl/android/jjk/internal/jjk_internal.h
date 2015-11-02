/*
 * copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
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

#ifndef JJK_INTERNAL_H
#define JJK_INTERNAL_H

#include <string.h>
#include <stdbool.h>
#include <jni.h>
#include "ijksdl/ijksdl_log.h"

#define JJK_FUNC_FAIL_TRACE()               do {ALOGE("%s: failed\n", __func__);} while (0)
#define JJK_FUNC_FAIL_TRACE1(x__)           do {ALOGE("%s: failed: %s\n", __func__, x__);} while (0)
#define JJK_FUNC_FAIL_TRACE2(x1__, x2__)    do {ALOGE("%s: failed: %s %s\n", __func__, x1__, x2__);} while (0)

#ifndef JJK_UNUSED
#define JJK_UNUSED(x) x __attribute__((unused))
#endif

/********************
 * Exception Handle
 ********************/

bool JJK_ExceptionCheck__throwAny(JNIEnv *env);
bool JJK_ExceptionCheck__catchAll(JNIEnv *env);
int  JJK_ThrowExceptionOfClass(JNIEnv* env, jclass clazz, const char* msg);
int  JJK_ThrowException(JNIEnv* env, const char* class_sign, const char* msg);
int  JJK_ThrowIllegalStateException(JNIEnv *env, const char* msg);

/********************
 * References
 ********************/
jclass JJK_NewGlobalRef__catchAll(JNIEnv *env, jobject obj);
void   JJK_DeleteLocalRef(JNIEnv *env, jobject obj);
void   JJK_DeleteLocalRef__p(JNIEnv *env, jobject *obj);
void   JJK_DeleteGlobalRef(JNIEnv *env, jobject obj);
void   JJK_DeleteGlobalRef__p(JNIEnv *env, jobject *obj);

void   JJK_ReleaseStringUTFChars(JNIEnv *env, jstring str, const char *c_str);
void   JJK_ReleaseStringUTFChars__p(JNIEnv *env, jstring str, const char **c_str);

/********************
 * Class Load
 ********************/

int    JJK_LoadAll__catchAll(JNIEnv *env);
jclass JJK_FindClass__catchAll(JNIEnv *env, const char *class_sign);
jclass JJK_FindClass__asGlobalRef__catchAll(JNIEnv *env, const char *class_sign);

jmethodID JJK_GetMethodID__catchAll(JNIEnv *env, jclass clazz, const char *method_name, const char *method_sign);
jmethodID JJK_GetStaticMethodID__catchAll(JNIEnv *env, jclass clazz, const char *method_name, const char *method_sign);

jfieldID JJK_GetFieldID__catchAll(JNIEnv *env, jclass clazz, const char *field_name, const char *method_sign);
jfieldID JJK_GetStaticFieldID__catchAll(JNIEnv *env, jclass clazz, const char *field_name, const char *method_sign);

/********************
 * Misc Functions
 ********************/

jbyteArray JJK_NewByteArray__catchAll(JNIEnv *env, jsize capacity);
jbyteArray JJK_NewByteArray__asGlobalRef__catchAll(JNIEnv *env, jsize capacity);

int JJK_GetSystemAndroidApiLevel();

#endif//JJK_INTERNAL_H
