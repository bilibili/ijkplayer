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


typedef struct ijkBundle ijkBundle;


ijkBundle *
ijkBundle_init(JNIEnv *env, jobject java_bundle /* = NULL */ );
void
ijkBundle_destroyP(JNIEnv *env, ijkBundle **p_bundle);

void
ijkBundle_putInt(JNIEnv *env, ijkBundle *bundle, const char *key, int value);
int
ijkBundle_getInt(JNIEnv *env, ijkBundle *bundle, const char *key, int default_value);

void
ijkBundle_putString(JNIEnv *env, ijkBundle *bundle, const char *key, const char *value);
const char *
ijkBundle_getString(JNIEnv *env, ijkBundle *bundle, const char *key);


#endif /* IJKADK__IJKADK_ANDROID_OS_BUNDLE_H */
