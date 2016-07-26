#! /usr/bin/env bash
#
# Copyright (C) 2014 Miguel Botón <waninkoko@gmail.com>
# Copyright (C) 2014 Zhang Rui <bbcallen@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#--------------------
set -e

if [ -z "$ANDROID_NDK" ]; then
    echo "You must define ANDROID_NDK before starting."
    echo "They must point to your NDK directories.\n"
    exit 1
fi

#--------------------
# common defines
FF_ARCH=$1
if [ -z "$FF_ARCH" ]; then
    echo "You must specific an architecture 'arm, armv7a, x86, ...'.\n"
    exit 1
fi


FF_BUILD_ROOT=`pwd`

FF_BUILD_NAME=
FF_SOURCE=
FF_CROSS_PREFIX=

FF_CFG_FLAGS=
FF_PLATFORM_CFG_FLAGS=

FF_EXTRA_CFLAGS=
FF_EXTRA_LDFLAGS=

FF_CMAKE_ABI=
FF_CMAKE_EXTRA_FLAGS=

#----- armv7a begin -----
if [ "$FF_ARCH" = "armv7a" ]; then
    FF_BUILD_NAME=libsoxr-armv7a
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME

    FF_CMAKE_ABI="armeabi-v7a with NEON"
    FF_CMAKE_EXTRA_FLAGS="-DHAVE_WORDS_BIGENDIAN_EXITCODE=1 -DWITH_SIMD=0"

elif [ "$FF_ARCH" = "armv5" ]; then
    FF_BUILD_NAME=libsoxr-armv5
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME

    FF_CMAKE_ABI="armeabi"
    FF_CMAKE_EXTRA_FLAGS="-DHAVE_WORDS_BIGENDIAN_EXITCODE=1 -DWITH_SIMD=0"

elif [ "$FF_ARCH" = "x86" ]; then
    FF_BUILD_NAME=libsoxr-x86
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME

    FF_CMAKE_ABI="x86"
    FF_CMAKE_EXTRA_FLAGS="-DHAVE_WORDS_BIGENDIAN_EXITCODE=1"

elif [ "$FF_ARCH" = "x86_64" ]; then
    FF_ANDROID_PLATFORM=android-21

    FF_BUILD_NAME=libsoxr-x86_64
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME

    FF_CMAKE_ABI="x86_64"

elif [ "$FF_ARCH" = "arm64" ]; then
    FF_ANDROID_PLATFORM=android-21

    FF_BUILD_NAME=libsoxr-arm64
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME

    FF_CMAKE_ABI="arm64-v8a"

else
    echo "unknown architecture $FF_ARCH";
    exit 1
fi

FF_PREFIX=$FF_BUILD_ROOT/build/$FF_BUILD_NAME/output
FF_CMAKE_BUILD_DIR=$FF_BUILD_ROOT/build/$FF_BUILD_NAME/build

mkdir -p $FF_PREFIX
mkdir -p $FF_CMAKE_BUILD_DIR

#--------------------
echo ""
echo "--------------------"
echo "[*] configurate libsoxr"
echo "--------------------"
cd $FF_CMAKE_BUILD_DIR
FF_CMAKE_CFG_FLAGS="-DCMAKE_TOOLCHAIN_FILE=${FF_SOURCE}/android.toolchain.cmake -DANDROID_NDK=$ANDROID_NDK -DBUILD_EXAMPLES=0 -DBUILD_LSR_TESTS=0 -DBUILD_SHARED_LIBS=0 -DBUILD_TESTS=0 -DCMAKE_BUILD_TYPE=Release -DWITH_LSR_BINDINGS=0 -DWITH_OPENMP=0 -DWITH_PFFFT=0"
echo "cmake $FF_CMAKE_CFG_FLAGS -DANDROID_ABI=$FF_CMAKE_ABI -DCMAKE_INSTALL_PREFIX=$FF_PREFIX"
cmake $FF_CMAKE_CFG_FLAGS $FF_CMAKE_EXTRA_FLAGS -DANDROID_ABI=$FF_CMAKE_ABI -DCMAKE_INSTALL_PREFIX=$FF_PREFIX $FF_SOURCE


#--------------------
echo ""
echo "--------------------"
echo "[*] compile libsoxr"
echo "--------------------"
make -j4
make install
