#! /usr/bin/env bash
#
# Copyright (C) 2020-present befovy <befovy@gmail.com>
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
FF_ANDROID_PLATFORM=android-14


FF_BUILD_NAME=
FF_SOURCE=
FF_CROSS_PREFIX=

FF_CFG_FLAGS=
FF_PLATFORM_CFG_FLAGS=

FF_EXTRA_CFLAGS=
FF_EXTRA_LDFLAGS=



#--------------------
echo ""
echo "--------------------"
echo "[*] make NDK standalone toolchain"
echo "--------------------"
. ./tools/do-detect-env.sh
FF_MAKE_TOOLCHAIN_FLAGS=$IJK_MAKE_TOOLCHAIN_FLAGS
FF_MAKE_FLAGS=$IJK_MAKE_FLAG
FF_GCC_VER=$IJK_GCC_VER
FF_GCC_64_VER=$IJK_GCC_64_VER

ANDROID_ABI=
CMAKE_ANDROID_ARCH_ABI=

#----- armv7a begin -----
if [ "$FF_ARCH" = "armv7a" ]; then
    FF_BUILD_NAME=libsrt-armv7a
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME
	
    FF_CROSS_PREFIX=arm-linux-androideabi
	FF_TOOLCHAIN_NAME=${FF_CROSS_PREFIX}-${FF_GCC_VER}

    FF_PLATFORM_CFG_FLAGS="android-armv7"

    ANDROID_ABI=armeabi-v7a
    CMAKE_ANDROID_ARCH_ABI=armeabi-v7a
    CMAKE_ANDROID_API=14

elif [ "$FF_ARCH" = "armv5" ]; then
    FF_BUILD_NAME=libsrt-armv5
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME
	
    FF_CROSS_PREFIX=arm-linux-androideabi
	FF_TOOLCHAIN_NAME=${FF_CROSS_PREFIX}-${FF_GCC_VER}

    FF_PLATFORM_CFG_FLAGS="android"
    CMAKE_ANDROID_API=14
    ANDROID_ABI=armeabi
    CMAKE_ANDROID_ARCH_ABI=armeabi
elif [ "$FF_ARCH" = "x86" ]; then
    FF_BUILD_NAME=libsrt-x86
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME
	
    FF_CROSS_PREFIX=i686-linux-android
	FF_TOOLCHAIN_NAME=x86-${FF_GCC_VER}

    FF_PLATFORM_CFG_FLAGS="android-x86"
    CMAKE_ANDROID_API=14
    ANDROID_ABI=x86
    CMAKE_ANDROID_ARCH_ABI=x86
    # FF_CFG_FLAGS="$FF_CFG_FLAGS no-asm"

elif [ "$FF_ARCH" = "x86_64" ]; then
    FF_ANDROID_PLATFORM=android-21
    CMAKE_ANDROID_API=21
    FF_BUILD_NAME=libsrt-x86_64
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME

    FF_CROSS_PREFIX=x86_64-linux-android
    FF_TOOLCHAIN_NAME=${FF_CROSS_PREFIX}-${FF_GCC_64_VER}

    FF_PLATFORM_CFG_FLAGS="linux-x86_64"
    ANDROID_ABI=x86_64
    CMAKE_ANDROID_ARCH_ABI=x86_64
elif [ "$FF_ARCH" = "arm64" ]; then
    FF_ANDROID_PLATFORM=android-21
    CMAKE_ANDROID_API=21
    FF_BUILD_NAME=libsrt-arm64
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME

    FF_CROSS_PREFIX=aarch64-linux-android
    FF_TOOLCHAIN_NAME=${FF_CROSS_PREFIX}-${FF_GCC_64_VER}

    FF_PLATFORM_CFG_FLAGS="linux-aarch64"

    ANDROID_ABI=arm64-v8a
    CMAKE_ANDROID_ARCH_ABI=arm64-v8a
else
    echo "unknown architecture $FF_ARCH";
    exit 1
fi

FF_TOOLCHAIN_PATH=$FF_BUILD_ROOT/build/toolchain-$FF_ARCH

FF_SYSROOT=$FF_TOOLCHAIN_PATH/sysroot
FF_PREFIX=$FF_BUILD_ROOT/build/output-$FF_ARCH

mkdir -p $FF_PREFIX
# mkdir -p $FF_SYSROOT


#--------------------
echo ""
echo "--------------------"
echo "[*] make NDK standalone toolchain"
echo "--------------------"
. ./tools/do-detect-env.sh
FF_MAKE_TOOLCHAIN_FLAGS=$IJK_MAKE_TOOLCHAIN_FLAGS
FF_MAKE_FLAGS=$IJK_MAKE_FLAG


FF_MAKE_TOOLCHAIN_FLAGS="$FF_MAKE_TOOLCHAIN_FLAGS --install-dir=$FF_TOOLCHAIN_PATH"
FF_TOOLCHAIN_TOUCH="$FF_TOOLCHAIN_PATH/touch"
if [ ! -f "$FF_TOOLCHAIN_TOUCH" ]; then
    $ANDROID_NDK/build/tools/make-standalone-toolchain.sh \
        $FF_MAKE_TOOLCHAIN_FLAGS \
        --platform=$FF_ANDROID_PLATFORM \
        --toolchain=$FF_TOOLCHAIN_NAME
    touch $FF_TOOLCHAIN_TOUCH;
fi


#--------------------
echo ""
echo "--------------------"
echo "[*] check libsrt env"
echo "--------------------"
export PATH=$FF_TOOLCHAIN_PATH/bin:$PATH

export COMMON_FF_CFG_FLAGS=


FF_CFG_FLAGS="$FF_CFG_FLAGS $COMMON_FF_CFG_FLAGS"

#--------------------
# Standard options:
# FF_CFG_FLAGS="$FF_CFG_FLAGS zlib-dynamic"
# FF_CFG_FLAGS="$FF_CFG_FLAGS no-shared"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --openssldir=$FF_PREFIX"

export PKG_CONFIG_PATH="${FF_PREFIX}/lib/pkgconfig"

FF_CFG_FLAGS="$FF_CFG_FLAGS --cmake-system-name=Android"
FF_CFG_FLAGS="$FF_CFG_FLAGS --android-toolchain=gcc"
FF_CFG_FLAGS="$FF_CFG_FLAGS --with-compiler-prefix=${FF_CROSS_PREFIX}-"
FF_CFG_FLAGS="$FF_CFG_FLAGS --with-target-path=$FF_TOOLCHAIN_PATH"

FF_CFG_FLAGS="$FF_CFG_FLAGS --android-abi=$ANDROID_ABI"
FF_CFG_FLAGS="$FF_CFG_FLAGS --cmake-android-arch-abi=$CMAKE_ANDROID_ARCH_ABI"
FF_CFG_FLAGS="$FF_CFG_FLAGS --android-stl=c++_static"
FF_CFG_FLAGS="$FF_CFG_FLAGS --cmake-android-api=$CMAKE_ANDROID_API"

# FF_CFG_FLAGS="$FF_CFG_FLAGS --cmake-android-standalone-toolchain=${FF_TOOLCHAIN_PATH}"
FF_CFG_FLAGS="$FF_CFG_FLAGS --cmake-prefix-path=${FF_PREFIX}"
FF_CFG_FLAGS="$FF_CFG_FLAGS --cmake-install-prefix=${FF_PREFIX}"
FF_CFG_FLAGS="$FF_CFG_FLAGS --use-openssl-pc=off"
FF_CFG_FLAGS="$FF_CFG_FLAGS --openssl-include-dir=${FF_PREFIX}/include"
FF_CFG_FLAGS="$FF_CFG_FLAGS --openssl-ssl-library=${FF_PREFIX}/lib/libssl.a"
FF_CFG_FLAGS="$FF_CFG_FLAGS --openssl-crypto-library=${FF_PREFIX}/lib/libcrypto.a"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-shared=off --enable-c++11=off"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-static=on"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-apps=off"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-c++-deps=on"
FF_CFG_FLAGS="$FF_CFG_FLAGS --use-static-libstdc++=on"


export CFLAGS="$CFLAGS -fPIE -fPIC"
export LDFLAGS="$LDFLAGS -pie"

#--------------------
echo ""
echo "--------------------"
echo "[*] configurate libsrt"
echo "--------------------"
cd $FF_SOURCE

git clean -fx

#if [ -f "./Makefile" ]; then
#    echo 'reuse configure'
#else
    echo "./configure $FF_CFG_FLAGS"
    ./configure $FF_CFG_FLAGS
#        --extra-cflags="$FF_CFLAGS $FF_EXTRA_CFLAGS" \
#        --extra-ldflags="$FF_EXTRA_LDFLAGS"
#fi

#--------------------
echo ""
echo "--------------------"
echo "[*] compile libsrt"
echo "--------------------"
make depend
make $FF_MAKE_FLAGS
make install

sed -i '' 's|-lsrt   |-lsrt -lc -lm -ldl -lcrypto -lssl -lstdc++|g' ${FF_PREFIX}/lib/pkgconfig/srt.pc
