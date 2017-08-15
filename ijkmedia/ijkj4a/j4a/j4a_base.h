/*
 * copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of jni4android.
 *
 * jni4android is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * jni4android is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with jni4android; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef J4A_BASE_H
#define J4A_BASE_H

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <jni.h>
#include <android/log.h>

#ifndef J4A_UNUSED
#define J4A_UNUSED(x) x __attribute__((unused))
#endif

#define J4A_LOG_TAG "J4A"
#define J4A_VLOGV(...)  __android_log_vprint(ANDROID_LOG_VERBOSE,   J4A_LOG_TAG, __VA_ARGS__)
#define J4A_VLOGD(...)  __android_log_vprint(ANDROID_LOG_DEBUG,     J4A_LOG_TAG, __VA_ARGS__)
#define J4A_VLOGI(...)  __android_log_vprint(ANDROID_LOG_INFO,      J4A_LOG_TAG, __VA_ARGS__)
#define J4A_VLOGW(...)  __android_log_vprint(ANDROID_LOG_WARN,      J4A_LOG_TAG, __VA_ARGS__)
#define J4A_VLOGE(...)  __android_log_vprint(ANDROID_LOG_ERROR,     J4A_LOG_TAG, __VA_ARGS__)

#define J4A_ALOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,    J4A_LOG_TAG, __VA_ARGS__)
#define J4A_ALOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,      J4A_LOG_TAG, __VA_ARGS__)
#define J4A_ALOGI(...)  __android_log_print(ANDROID_LOG_INFO,       J4A_LOG_TAG, __VA_ARGS__)
#define J4A_ALOGW(...)  __android_log_print(ANDROID_LOG_WARN,       J4A_LOG_TAG, __VA_ARGS__)
#define J4A_ALOGE(...)  __android_log_print(ANDROID_LOG_ERROR,      J4A_LOG_TAG, __VA_ARGS__)

#define J4A_FUNC_FAIL_TRACE()               do {J4A_ALOGE("%s: failed\n", __func__);} while (0)
#define J4A_FUNC_FAIL_TRACE1(x__)           do {J4A_ALOGE("%s: failed: %s\n", __func__, x__);} while (0)
#define J4A_FUNC_FAIL_TRACE2(x1__, x2__)    do {J4A_ALOGE("%s: failed: %s %s\n", __func__, x1__, x2__);} while (0)

#define J4A_LOAD_CLASS(class__) \
    do { \
        ret = J4A_loadClass__J4AC_##class__(env); \
        if (ret) \
            goto fail; \
    } while (0)

/********************
 * Exception Handle
 ********************/

bool J4A_ExceptionCheck__throwAny(JNIEnv *env);
bool J4A_ExceptionCheck__catchAll(JNIEnv *env);
int  J4A_ThrowExceptionOfClass(JNIEnv* env, jclass clazz, const char* msg);
int  J4A_ThrowException(JNIEnv* env, const char* class_sign, const char* msg);
int  J4A_ThrowIllegalStateException(JNIEnv *env, const char* msg);

/********************
 * References
 ********************/
jclass J4A_NewGlobalRef__catchAll(JNIEnv *env, jobject obj);
void   J4A_DeleteLocalRef(JNIEnv *env, jobject obj);
void   J4A_DeleteLocalRef__p(JNIEnv *env, jobject *obj);
void   J4A_DeleteGlobalRef(JNIEnv *env, jobject obj);
void   J4A_DeleteGlobalRef__p(JNIEnv *env, jobject *obj);

void   J4A_ReleaseStringUTFChars(JNIEnv *env, jstring str, const char *c_str);
void   J4A_ReleaseStringUTFChars__p(JNIEnv *env, jstring str, const char **c_str);

/********************
 * Class Load
 ********************/

int    J4A_LoadAll__catchAll(JNIEnv *env);
jclass J4A_FindClass__catchAll(JNIEnv *env, const char *class_sign);
jclass J4A_FindClass__asGlobalRef__catchAll(JNIEnv *env, const char *class_sign);

jmethodID J4A_GetMethodID__catchAll(JNIEnv *env, jclass clazz, const char *method_name, const char *method_sign);
jmethodID J4A_GetStaticMethodID__catchAll(JNIEnv *env, jclass clazz, const char *method_name, const char *method_sign);

jfieldID J4A_GetFieldID__catchAll(JNIEnv *env, jclass clazz, const char *field_name, const char *method_sign);
jfieldID J4A_GetStaticFieldID__catchAll(JNIEnv *env, jclass clazz, const char *field_name, const char *method_sign);

/********************
 * Misc Functions
 ********************/

jbyteArray J4A_NewByteArray__catchAll(JNIEnv *env, jsize capacity);
jbyteArray J4A_NewByteArray__asGlobalRef__catchAll(JNIEnv *env, jsize capacity);

int J4A_GetSystemAndroidApiLevel(JNIEnv *env);

#endif//J4A_INTERNAL_H
