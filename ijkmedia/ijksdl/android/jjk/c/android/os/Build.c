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

#include "Build.h"

typedef struct JJKC_android_os_Build__VERSION {
    jclass id;

    jfieldID field_SDK_INT;
} JJKC_android_os_Build__VERSION;
static JJKC_android_os_Build__VERSION class_JJKC_android_os_Build__VERSION;

typedef struct JJKC_android_os_Build {
    jclass id;

} JJKC_android_os_Build;
static JJKC_android_os_Build class_JJKC_android_os_Build;

jint JJKC_android_os_Build__VERSION__SDK_INT__get(JNIEnv *env)
{
    return (*env)->GetStaticIntField(env, class_JJKC_android_os_Build__VERSION.id, class_JJKC_android_os_Build__VERSION.field_SDK_INT);
}

jint JJKC_android_os_Build__VERSION__SDK_INT__get__catchAll(JNIEnv *env)
{
    jint ret_value = JJKC_android_os_Build__VERSION__SDK_INT__get(env);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

void JJKC_android_os_Build__VERSION__SDK_INT__set(JNIEnv *env, jint value)
{
    (*env)->SetStaticIntField(env, class_JJKC_android_os_Build__VERSION.id, class_JJKC_android_os_Build__VERSION.field_SDK_INT, value);
}

void JJKC_android_os_Build__VERSION__SDK_INT__set__catchAll(JNIEnv *env, jint value)
{
    JJKC_android_os_Build__VERSION__SDK_INT__set(env, value);
    JJK_ExceptionCheck__catchAll(env);
}

int JJK_loadClass__JJKC_android_os_Build__VERSION(JNIEnv *env)
{
    int         ret                   = -1;
    const char *JJK_UNUSED(name)      = NULL;
    const char *JJK_UNUSED(sign)      = NULL;
    jclass      JJK_UNUSED(class_id)  = NULL;
    int         JJK_UNUSED(api_level) = 0;

    sign = "android/os/Build$VERSION";
    class_JJKC_android_os_Build__VERSION.id = JJK_FindClass__asGlobalRef__catchAll(env, sign);
    if (class_JJKC_android_os_Build__VERSION.id == NULL)
        goto fail;

    class_id = class_JJKC_android_os_Build__VERSION.id;
    name     = "SDK_INT";
    sign     = "I";
    class_JJKC_android_os_Build__VERSION.field_SDK_INT = JJK_GetStaticFieldID__catchAll(env, class_id, name, sign);
    if (class_JJKC_android_os_Build__VERSION.field_SDK_INT == NULL)
        goto fail;

    ALOGD("JJKLoader: OK: '%s' loaded\n", "android.os.Build$VERSION");
    ret = 0;
fail:
    return ret;
}

int JJK_loadClass__JJKC_android_os_Build(JNIEnv *env)
{
    int         ret                   = -1;
    const char *JJK_UNUSED(name)      = NULL;
    const char *JJK_UNUSED(sign)      = NULL;
    jclass      JJK_UNUSED(class_id)  = NULL;
    int         JJK_UNUSED(api_level) = 0;

    sign = "android/os/Build";
    class_JJKC_android_os_Build.id = JJK_FindClass__asGlobalRef__catchAll(env, sign);
    if (class_JJKC_android_os_Build.id == NULL)
        goto fail;

    ret = JJK_loadClass__JJKC_android_os_Build__VERSION(env);
    if (ret)
        goto fail;

    ALOGD("JJKLoader: OK: '%s' loaded\n", "android.os.Build");
    ret = 0;
fail:
    return ret;
}
