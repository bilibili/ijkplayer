/*****************************************************************************
 * android_bundle.h
 *****************************************************************************
 *
 * copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#ifndef IJKSDL_ANDROID__ANDROID_BUNDLE_H
#define IJKSDL_ANDROID__ANDROID_BUNDLE_H

#include "ijksdl_inc_internal_android.h"

int ASDK_Bundle__loadClass(JNIEnv *env);

jobject ASDK_Bundle__init(JNIEnv *env);
int     ASDK_Bundle__getInt_c(JNIEnv *env, jobject thiz, const char *key, int defaultValue);
void    ASDK_Bundle__putInt_c(JNIEnv *env, jobject thiz, const char *key, int value);
void    ASDK_Bundle__putParcelableArrayList_c(JNIEnv *env, jobject thiz, const char *key, jobject value);

void    ASDK_Bundle__getString_cbuf(JNIEnv *env, jobject thiz, const char *key, char *value, size_t size);
void    ASDK_Bundle__putString_c(JNIEnv *env, jobject thiz, const char *key, const char *value);

#endif
