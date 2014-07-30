/*****************************************************************************
 * ijkadk_java_nio_ByteBuffer.cpp
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

#include "ijkadk_java_nio_ByteBuffer.hpp"

using namespace ::ijkadk::java::nio;

typedef struct ByteBufferClass
{
    jclass      clazz;
} ByteBufferClass;
static ByteBufferClass gClazz;

int ByteBuffer::loadClass(JNIEnv *env)
{
    ADKJniClassLoadHelper helper(env);
    gClazz.clazz = helper.findClassAsGlobalRef("Ljava/nio/ByteBuffer;");

    return helper.getIntResult();
}
