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

#ifndef JJK__java_util_ArrayList__H
#define JJK__java_util_ArrayList__H

#include "ijksdl/android/jjk/internal/jjk_internal.h"

jobject JJKC_ArrayList__ArrayList(JNIEnv *env);
jobject JJKC_ArrayList__ArrayList__catchAll(JNIEnv *env);
jobject JJKC_ArrayList__ArrayList__asGlobalRef__catchAll(JNIEnv *env);
jboolean JJKC_ArrayList__add(JNIEnv *env, jobject thiz, jobject object);
jboolean JJKC_ArrayList__add__catchAll(JNIEnv *env, jobject thiz, jobject object);
int JJK_loadClass__JJKC_ArrayList(JNIEnv *env);

#endif//JJK__java_util_ArrayList__H
