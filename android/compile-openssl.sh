#! /usr/bin/env bash
#
# Copyright (C) 2013-2014 Zhang Rui <bbcallen@gmail.com>
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
echo "===================="
echo "[*] check env"
echo "===================="
set -e

if [ -z "$ANDROID_NDK" -o -z "$ANDROID_SDK" ]; then
    echo "You must define ANDROID_NDK, ANDROID_SDK before starting."
    echo "They must point to your NDK and SDK directories.\n"
    exit 1
fi

# try to detect NDK version
FF_NDK_REL=$(grep -o '^r[0-9]*.*' $ANDROID_NDK/RELEASE.TXT 2>/dev/null|cut -b2-)
case "$FF_NDK_REL" in
    9?*)
        # we don't use 4.4.3 because it doesn't handle threads correctly.
        if test -d ${ANDROID_NDK}/toolchains/arm-linux-androideabi-4.8
        # if gcc 4.8 is present, it's there for all the archs (x86, mips, arm)
        then
            echo "NDKr$FF_NDK_REL detected"
        else
            echo "You need the NDKr9 or later"
            exit 1
        fi
    ;;
    7|8|*)
        echo "You need the NDKr9 or later"
        exit 1
    ;;
esac

FF_BUILD_ROOT=`pwd`
FF_ANDROID_PLATFORM=android-9
FF_GCC_VER=4.8



FF_TAGET_OS=
FF_BUILD_NAME=
FF_SOURCE=
FF_CROSS_PREFIX=

FF_CFG_FLAGS=
FF_PLATFORM_CFG_FLAGS=

FF_EXTRA_CFLAGS=
FF_EXTRA_LDFLAGS=

#----- armv7a begin -----
FF_TAGET_OS=linux
FF_BUILD_NAME=openssl-armv7a
FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME
FF_CROSS_PREFIX=arm-linux-androideabi

FF_PLATFORM_CFG_FLAGS="android-armv7"
#----- armv7a end -----

FF_TOOLCHAIN_NAME=${FF_CROSS_PREFIX}-${FF_GCC_VER}
FF_TOOLCHAIN_PATH=$FF_BUILD_ROOT/build/$FF_BUILD_NAME/toolchain

FF_SYSROOT=$FF_TOOLCHAIN_PATH/sysroot
FF_PREFIX=$FF_BUILD_ROOT/build/$FF_BUILD_NAME/output

mkdir -p $FF_PREFIX
mkdir -p $FF_SYSROOT

#--------------------
echo "\n--------------------"
echo "[*] make NDK standalone toolchain"
echo "--------------------"
UNAMES=$(uname -s)
FF_MAKE_TOOLCHAIN_FLAGS="--install-dir=$FF_TOOLCHAIN_PATH"
if [ "$UNAMES" == "Darwin" ]; then
    echo "build on darwin-x86_64"
    FF_MAKE_TOOLCHAIN_FLAGS="$FF_MAKE_TOOLCHAIN_FLAGS --system=darwin-x86_64"
    FF_MAKE_FLAG=-j`sysctl -n machdep.cpu.thread_count`
fi

FF_MAKEFLAGS=
if which nproc >/dev/null
then
    FF_MAKEFLAGS=-j`nproc`
elif [ "$UNAMES" == "Darwin" ] && which sysctl >/dev/null
then
    FF_MAKEFLAGS=-j`sysctl -n machdep.cpu.thread_count`
fi


$ANDROID_NDK/build/tools/make-standalone-toolchain.sh \
    $FF_MAKE_TOOLCHAIN_FLAGS \
    --platform=$FF_ANDROID_PLATFORM \
    --toolchain=$FF_TOOLCHAIN_NAME

#--------------------
echo "\n--------------------"
echo "[*] check openssl env"
echo "--------------------"
export PATH=$FF_TOOLCHAIN_PATH/bin:$PATH

export COMMON_FF_CFG_FLAGS=

FF_CFG_FLAGS="$FF_CFG_FLAGS $COMMON_FF_CFG_FLAGS"

#--------------------
# Standard options:
FF_CFG_FLAGS="$FF_CFG_FLAGS zlib-dynamic"
FF_CFG_FLAGS="$FF_CFG_FLAGS no-shared"
FF_CFG_FLAGS="$FF_CFG_FLAGS --openssldir=$FF_PREFIX"
FF_CFG_FLAGS="$FF_CFG_FLAGS --cross-compile-prefix=${FF_CROSS_PREFIX}-"
FF_CFG_FLAGS="$FF_CFG_FLAGS $FF_PLATFORM_CFG_FLAGS"

#--------------------
echo "\n--------------------"
echo "[*] configurate openssl"
echo "--------------------"
cd $FF_SOURCE
#if [ -f "./Makefile" ]; then
#    echo 'reuse configure'
#else
    echo "./Configure $FF_CFG_FLAGS"
    ./Configure $FF_CFG_FLAGS
#        --extra-cflags="$FF_CFLAGS $FF_EXTRA_CFLAGS" \
#        --extra-ldflags="$FF_EXTRA_LDFLAGS"
#fi

#--------------------
echo "\n--------------------"
echo "[*] compile openssl"
echo "--------------------"
make depend
make
make install

#--------------------
echo "\n--------------------"
echo "[*] link openssl"
echo "--------------------"

echo "\n--------------------"
echo "[*] Finished"
echo "--------------------"
echo "# to continue to build openssl, run script below,"
echo "sh compile-openssl.sh "
echo "# to continue to build ffmpeg, run script below,"
echo "sh compile-ffmpeg.sh "
echo "# to continue to build ijkplayer, run script below,"
echo "sh compile-ijk.sh "
