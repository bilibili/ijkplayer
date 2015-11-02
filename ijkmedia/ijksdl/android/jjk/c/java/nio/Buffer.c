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

#include "Buffer.h"

typedef struct JJKC_java_nio_Buffer {
    jclass id;
} JJKC_java_nio_Buffer;
static JJKC_java_nio_Buffer class_JJKC_java_nio_Buffer;

int JJK_loadClass__JJKC_java_nio_Buffer(JNIEnv *env)
{
    int         ret                   = -1;
    const char *JJK_UNUSED(name)      = NULL;
    const char *JJK_UNUSED(sign)      = NULL;
    jclass      JJK_UNUSED(class_id)  = NULL;
    int         JJK_UNUSED(api_level) = 0;

    sign = "java/nio/Buffer";
    class_JJKC_java_nio_Buffer.id = JJK_FindClass__asGlobalRef__catchAll(env, sign);
    if (class_JJKC_java_nio_Buffer.id == NULL)
        goto fail;

    ALOGD("JJKLoader: OK: '%s' loaded\n", "java.nio.Buffer");
    ret = 0;
fail:
    return ret;
}
