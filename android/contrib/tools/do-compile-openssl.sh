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
FF_ANDROID_PLATFORM=android-19


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


#----- armv7a begin -----
if [ "$FF_ARCH" = "armv7a" ]; then
    FF_ANDROID_PLATFORM=android-16
    FF_BUILD_NAME=openssl-armv7a
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME
	
    FF_CROSS_PREFIX=arm-linux-androideabi
	FF_TOOLCHAIN_NAME=${FF_CROSS_PREFIX}-${FF_GCC_VER}

    FF_PLATFORM_CFG_FLAGS="android-arm"
    FF_EXTRA_CFLAGS="-D__ANDROID_API__=16"

elif [ "$FF_ARCH" = "armv5" ]; then
    FF_ANDROID_PLATFORM=android-16
    FF_BUILD_NAME=openssl-armv5
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME
	
    FF_CROSS_PREFIX=arm-linux-androideabi
	FF_TOOLCHAIN_NAME=${FF_CROSS_PREFIX}-${FF_GCC_VER}

    FF_PLATFORM_CFG_FLAGS="android-arm"
    FF_EXTRA_CFLAGS="-D__ANDROID_API__=16"

elif [ "$FF_ARCH" = "x86" ]; then
    FF_ANDROID_PLATFORM=android-16
    FF_BUILD_NAME=openssl-x86
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME
	
    FF_CROSS_PREFIX=i686-linux-android
	FF_TOOLCHAIN_NAME=x86-${FF_GCC_VER}

    #can't work
    #FF_PLATFORM_CFG_FLAGS="android-x86"

    FF_PLATFORM_CFG_FLAGS="linux-x86"

    FF_CFG_FLAGS="$FF_CFG_FLAGS no-asm"
    FF_EXTRA_CFLAGS="-D__ANDROID_API__=16"

elif [ "$FF_ARCH" = "x86_64" ]; then
    FF_ANDROID_PLATFORM=android-21
    FF_BUILD_NAME=openssl-x86_64
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME

    FF_CROSS_PREFIX=x86_64-linux-android
    FF_TOOLCHAIN_NAME=${FF_CROSS_PREFIX}-${FF_GCC_64_VER}

    #can't work
    FF_PLATFORM_CFG_FLAGS="android-x86_64"

    FF_PLATFORM_CFG_FLAGS="linux-x86_64"
    FF_EXTRA_CFLAGS="-D__ANDROID_API__=21"

elif [ "$FF_ARCH" = "arm64" ]; then
    FF_ANDROID_PLATFORM=android-21
    FF_BUILD_NAME=openssl-arm64
    FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME

    FF_CROSS_PREFIX=aarch64-linux-android
    FF_TOOLCHAIN_NAME=${FF_CROSS_PREFIX}-${FF_GCC_64_VER}

    FF_PLATFORM_CFG_FLAGS="linux-aarch64"
    FF_EXTRA_CFLAGS="-D__ANDROID_API__=21"

else
    echo "unknown architecture $FF_ARCH";
    exit 1
fi

FF_TOOLCHAIN_PATH=$FF_BUILD_ROOT/build/$FF_BUILD_NAME/toolchain

FF_SYSROOT=$FF_TOOLCHAIN_PATH/sysroot
FF_PREFIX=$FF_BUILD_ROOT/build/$FF_BUILD_NAME/output

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
echo "[*] check openssl env"
echo "--------------------"


if [ "$FF_ARCH" = "armv7a" ] || [ "$FF_ARCH" = "armv5" ];then
    #workaround for openssl armv7a
    # https://github.com/openssl/openssl/issues/7398
    # https://github.com/openssl/openssl/issues/8100
    #openssl build script need to known ANDROID_NDK_HOME exactly for armv7a
    export ANDROID_NDK_HOME=$ANDROID_NDK
    #don't use standalone toolchain for armv7a but use NDK toolchain directly
    export PATH=$ANDROID_NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin:$PATH
else
    export PATH=$FF_TOOLCHAIN_PATH/bin:$PATH
fi

export COMMON_FF_CFG_FLAGS=

FF_CFG_FLAGS="$FF_CFG_FLAGS $COMMON_FF_CFG_FLAGS"

#--------------------
# Standard options:
FF_CFG_FLAGS="$FF_CFG_FLAGS zlib-dynamic"
FF_CFG_FLAGS="$FF_CFG_FLAGS no-shared"
FF_CFG_FLAGS="$FF_CFG_FLAGS --prefix=$FF_PREFIX"
FF_CFG_FLAGS="$FF_CFG_FLAGS --openssldir=$FF_PREFIX"
FF_CFG_FLAGS="$FF_CFG_FLAGS --cross-compile-prefix=${FF_CROSS_PREFIX}-"
FF_CFG_FLAGS="$FF_CFG_FLAGS $FF_PLATFORM_CFG_FLAGS"

#--------------------
echo ""
echo "--------------------"
echo "[*] configurate openssl"
echo "--------------------"
cd $FF_SOURCE
#if [ -f "./Makefile" ]; then
#    echo 'reuse configure'
#else
    echo "./Configure $FF_CFG_FLAGS $FF_EXTRA_CFLAGS"
    ./Configure $FF_CFG_FLAGS $FF_EXTRA_CFLAGS
#        --extra-cflags="$FF_CFLAGS $FF_EXTRA_CFLAGS" \
#        --extra-ldflags="$FF_EXTRA_LDFLAGS"
#fi

#--------------------
echo ""
echo "--------------------"
echo "[*] compile openssl"
echo "--------------------"
make depend
CPU_COUNTS=`cat /proc/cpuinfo | grep "processor" | wc | awk '{print int($1)}'`
echo "make -j${CPU_COUNTS} $FF_MAKE_FLAGS"
make -j${CPU_COUNTS} $FF_MAKE_FLAGS
make install_sw

#--------------------
echo ""
echo "--------------------"
echo "[*] link openssl"
echo "--------------------"
