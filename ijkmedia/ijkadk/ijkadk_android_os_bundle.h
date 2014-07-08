/*****************************************************************************
 * ijkadk_android_os_bundle.h
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

#ifndef IJKADK__IJKADK_ANDROID_OS_BUNDLE_H
#define IJKADK__IJKADK_ANDROID_OS_BUNDLE_H

#include <stdint.h>
#include <jni.h>


// load_class
int ijkadk_android_os_Bundle__loadClass(JNIEnv *env);


typedef struct ijkadk_android_os_Bundle ijkadk_android_os_Bundle;


ijkadk_android_os_Bundle *
ijkadk_android_os_Bundle__init(JNIEnv *env);
ijkadk_android_os_Bundle *
ijkadk_android_os_Bundle__initWithObject(JNIEnv *env, jobject java_bundle);
void
ijkadk_android_os_Bundle__destroyP(JNIEnv *env, ijkadk_android_os_Bundle **p_bundle);

void
ijkadk_android_os_Bundle__putInt(JNIEnv *env, ijkadk_android_os_Bundle *bundle, const char *key, int value);
int
ijkadk_android_os_Bundle__getInt(JNIEnv *env, ijkadk_android_os_Bundle *bundle, const char *key, int default_value);

void
ijkadk_android_os_Bundle__putString(JNIEnv *env, ijkadk_android_os_Bundle *bundle, const char *key, const char *value);
const char *
ijkadk_android_os_Bundle__getString(JNIEnv *env, ijkadk_android_os_Bundle *bundle, const char *key);


#endif /* IJKADK__IJKADK_ANDROID_OS_BUNDLE_H */
