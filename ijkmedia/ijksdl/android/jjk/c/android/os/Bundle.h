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

#ifndef JJK__android_os_Bundle__H
#define JJK__android_os_Bundle__H

#include "ijksdl/android/jjk/internal/jjk_internal.h"

jobject JJKC_Bundle__Bundle(JNIEnv *env);
jobject JJKC_Bundle__Bundle__catchAll(JNIEnv *env);
jobject JJKC_Bundle__Bundle__asGlobalRef__catchAll(JNIEnv *env);
jint JJKC_Bundle__getInt(JNIEnv *env, jobject thiz, jstring key, jint defaultValue);
jint JJKC_Bundle__getInt__catchAll(JNIEnv *env, jobject thiz, jstring key, jint defaultValue);
jint JJKC_Bundle__getInt__withCString(JNIEnv *env, jobject thiz, const char *key_cstr__, jint defaultValue);
jint JJKC_Bundle__getInt__withCString__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__, jint defaultValue);
void JJKC_Bundle__putInt(JNIEnv *env, jobject thiz, jstring key, jint value);
void JJKC_Bundle__putInt__catchAll(JNIEnv *env, jobject thiz, jstring key, jint value);
void JJKC_Bundle__putInt__withCString(JNIEnv *env, jobject thiz, const char *key_cstr__, jint value);
void JJKC_Bundle__putInt__withCString__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__, jint value);
jstring JJKC_Bundle__getString(JNIEnv *env, jobject thiz, jstring key);
jstring JJKC_Bundle__getString__catchAll(JNIEnv *env, jobject thiz, jstring key);
jstring JJKC_Bundle__getString__asGlobalRef__catchAll(JNIEnv *env, jobject thiz, jstring key);
const char *JJKC_Bundle__getString__asCBuffer(JNIEnv *env, jobject thiz, jstring key, char *out_buf, int out_len);
const char *JJKC_Bundle__getString__asCBuffer__catchAll(JNIEnv *env, jobject thiz, jstring key, char *out_buf, int out_len);
jstring JJKC_Bundle__getString__withCString(JNIEnv *env, jobject thiz, const char *key_cstr__);
jstring JJKC_Bundle__getString__withCString__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__);
jstring JJKC_Bundle__getString__withCString__asGlobalRef__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__);
const char *JJKC_Bundle__getString__withCString__asCBuffer(JNIEnv *env, jobject thiz, const char *key_cstr__, char *out_buf, int out_len);
const char *JJKC_Bundle__getString__withCString__asCBuffer__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__, char *out_buf, int out_len);
void JJKC_Bundle__putString(JNIEnv *env, jobject thiz, jstring key, jstring value);
void JJKC_Bundle__putString__catchAll(JNIEnv *env, jobject thiz, jstring key, jstring value);
void JJKC_Bundle__putString__withCString(JNIEnv *env, jobject thiz, const char *key_cstr__, const char *value_cstr__);
void JJKC_Bundle__putString__withCString__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__, const char *value_cstr__);
void JJKC_Bundle__putParcelableArrayList(JNIEnv *env, jobject thiz, jstring key, jobject value);
void JJKC_Bundle__putParcelableArrayList__catchAll(JNIEnv *env, jobject thiz, jstring key, jobject value);
void JJKC_Bundle__putParcelableArrayList__withCString(JNIEnv *env, jobject thiz, const char *key_cstr__, jobject value);
void JJKC_Bundle__putParcelableArrayList__withCString__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__, jobject value);
int JJK_loadClass__JJKC_Bundle(JNIEnv *env);

#endif//JJK__android_os_Bundle__H
