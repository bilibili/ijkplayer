/*****************************************************************************
 * ijkadk_java_nio_ByteBuffer.hpp
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

#ifndef IJKADK__IJKADK_JAVA_NIO_BYTEBUFFER_HPP
#define IJKADK__IJKADK_JAVA_NIO_BYTEBUFFER_HPP

#include "ijkadkobject.hpp"

namespace ijkadk {
namespace java {
namespace nio {

class ByteBuffer: public ::ijkadk::ADKObject
{
public:
    ADK_OBJ_BEGIN(ByteBuffer);
    ADK_OBJ_END();

protected:
    ByteBuffer() {;}
    ~ByteBuffer();

public:
    static int loadClass(JNIEnv *env);
};

} // end ::ijkadk::nio::ByteBuffer
} // end ::ijkadk::nio
} // end ::ijkadk

#endif /* IJKADK__IJKADK_JAVA_NIO_BYTEBUFFER_HPP */
