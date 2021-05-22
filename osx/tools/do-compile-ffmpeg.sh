#! /usr/bin/env bash
#
# Copyright (C) 2013-2014 Bilibili
# Copyright (C) 2013-2014 Zhang Rui <bbcallen@gmail.com>
# Copyright (C) 2018-2019 Befovy <befovy@gmail.com>
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

# This script is based on projects below
# https://github.com/kolyvan/kxmovie
# https://github.com/yixia/FFmpeg-Android
# http://git.videolan.org/?p=vlc-ports/android.git;a=summary
# https://github.com/kewlbear/FFmpeg-iOS-build-script/

#--------------------
echo "===================="
echo "[*] check host"
echo "===================="
set -e

#--------------------
# include


#--------------------
# common defines
FF_ARCH=$1
FF_BUILD_OPT=$2
echo "FF_ARCH=$FF_ARCH"
echo "FF_BUILD_OPT=$FF_BUILD_OPT"
if [ -z "$FF_ARCH" ]; then
    echo "You must specific an architecture 'i386, x86_64, ...'.\n"
    exit 1
fi


FF_BUILD_ROOT=`pwd`/contrib
FF_TAGET_OS="darwin"


# ffmpeg build params
export COMMON_FF_CFG_FLAGS=
source $FF_BUILD_ROOT/../../config/module-mac.sh

FFMPEG_CFG_FLAGS=
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS $COMMON_FF_CFG_FLAGS"

# Optimization options (experts only):
# FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --disable-armv5te"
# FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --disable-armv6"
# FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --disable-armv6t2"

# Advanced options (experts only):
# FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-cross-compile"
# --disable-symver may indicate a bug
# FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --disable-symver"

# Developer options (useful when working on FFmpeg itself):
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --disable-stripping"

##
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --arch=$FF_ARCH"
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --target-os=$FF_TAGET_OS"
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-static"
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --disable-shared"
FFMPEG_EXTRA_CFLAGS=

# i386, x86_64
FFMPEG_CFG_FLAGS_SIMULATOR=
FFMPEG_CFG_FLAGS_SIMULATOR="$FFMPEG_CFG_FLAGS_SIMULATOR --disable-asm"
FFMPEG_CFG_FLAGS_SIMULATOR="$FFMPEG_CFG_FLAGS_SIMULATOR --disable-mmx"
FFMPEG_CFG_FLAGS_SIMULATOR="$FFMPEG_CFG_FLAGS_SIMULATOR --assert-level=2"

# armv7, armv7s, arm64
FFMPEG_CFG_FLAGS_ARM=
FFMPEG_CFG_FLAGS_ARM="$FFMPEG_CFG_FLAGS_ARM --enable-pic"
FFMPEG_CFG_FLAGS_ARM="$FFMPEG_CFG_FLAGS_ARM --enable-neon"
case "$FF_BUILD_OPT" in
    debug)
        FFMPEG_CFG_FLAGS_ARM="$FFMPEG_CFG_FLAGS_ARM --disable-optimizations"
        FFMPEG_CFG_FLAGS_ARM="$FFMPEG_CFG_FLAGS_ARM --enable-debug"
        FFMPEG_CFG_FLAGS_ARM="$FFMPEG_CFG_FLAGS_ARM --disable-small"
    ;;
    *)
        FFMPEG_CFG_FLAGS_ARM="$FFMPEG_CFG_FLAGS_ARM --enable-optimizations"
        FFMPEG_CFG_FLAGS_ARM="$FFMPEG_CFG_FLAGS_ARM --enable-debug"
        FFMPEG_CFG_FLAGS_ARM="$FFMPEG_CFG_FLAGS_ARM --enable-small"
    ;;
esac

FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-pic"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-pthreads"

FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-yasm"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-asm"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-inline-asm"



FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-protocols"
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-decoders"
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-demuxers"
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-encoder=mjpeg"
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-hwaccels"

FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-videotoolbox"

echo "build_root: $FF_BUILD_ROOT"

#--------------------
echo "===================="
echo "[*] check gas-preprocessor"
echo "===================="
FF_TOOLS_ROOT="$FF_BUILD_ROOT/../extra"
# export PATH="$FF_TOOLS_ROOT/gas-preprocessor:$PATH"
# echo "gasp: $FF_TOOLS_ROOT/gas-preprocessor/gas-preprocessor.pl"

#--------------------
echo "===================="
echo "[*] config arch $FF_ARCH"
echo "===================="

FF_BUILD_NAME="unknown"
FF_XCRUN_PLATFORM="MacOSX"
FF_XCRUN_OSVERSION=
FF_GASPP_EXPORT=
FF_DEP_OPENSSL_INC=
FF_DEP_OPENSSL_LIB=
FF_XCODE_BITCODE=

if [ "$FF_ARCH" = "i386" ]; then
    FF_BUILD_NAME="ffmpeg-i386"
    FF_XCRUN_OSVERSION="-mmacosx-version-min=10.10"
    FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS $FFMPEG_CFG_FLAGS_SIMULATOR"
elif [ "$FF_ARCH" = "x86_64" ]; then
    FF_BUILD_NAME="ffmpeg-x86_64"
    FF_XCRUN_OSVERSION="-mmacosx-version-min=10.10"
    FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS $FFMPEG_CFG_FLAGS_SIMULATOR"
else
    echo "unknown architecture $FF_ARCH";
    exit 1
fi

echo "build_name: $FF_BUILD_NAME"
echo "platform:   $FF_XCRUN_PLATFORM"
echo "osversion:  $FF_XCRUN_OSVERSION"

#--------------------
echo "===================="
echo "[*] make osx toolchain $FF_BUILD_NAME"
echo "===================="

FF_BUILD_SOURCE="$FF_BUILD_ROOT/$FF_BUILD_NAME"
FF_BUILD_PREFIX="$FF_BUILD_ROOT/build/"

FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --prefix=$FF_BUILD_PREFIX"

mkdir -p $FF_BUILD_PREFIX

echo "build_source: $FF_BUILD_SOURCE"
echo "build_prefix: $FF_BUILD_PREFIX"

#--------------------
echo "\n--------------------"
echo "[*] configurate ffmpeg"
echo "--------------------"
FF_XCRUN_SDK=`echo $FF_XCRUN_PLATFORM | tr '[:upper:]' '[:lower:]'`
FF_XCRUN_CC="xcrun -sdk $FF_XCRUN_SDK clang"

if [ "$FF_ARCH" = "arm64" ]
then
    FF_AS="gas-preprocessor.pl -arch aarch64 -- $FF_XCRUN_CC"
else
    FF_AS="gas-preprocessor.pl -- $FF_XCRUN_CC"
fi

FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS $FFMPEG_CFG_CPU"

FFMPEG_CFLAGS=
FFMPEG_CFLAGS="$FFMPEG_CFLAGS $FF_XCRUN_OSVERSION"
FFMPEG_CFLAGS="$FFMPEG_CFLAGS -arch $FF_ARCH"
FFMPEG_CFLAGS="$FFMPEG_CFLAGS $FFMPEG_EXTRA_CFLAGS"
FFMPEG_CFLAGS="$FFMPEG_CFLAGS $FF_XCODE_BITCODE"
FFMPEG_LDFLAGS="$FFMPEG_CFLAGS"


# --------------------
echo "----------------------"
echo "[*] check OpenSSL"
echo "----------------------"
FFMPEG_DEP_OPENSSL_INC=/usr/local/opt/openssl/include
FFMPEG_DEP_OPENSSL_LIB=/usr/local/opt/openssl/lib
#--------------------
# with openssl
# brew install openssl

export PKG_CONFIG_PATH="${FF_BUILD_ROOT}/build/lib/pkgconfig"
echo "PKG_CONFIG_PATH ${PKG_CONFIG_PATH}"
mkdir -p ${PKG_CONFIG_PATH}

if [ -f "${FFMPEG_DEP_OPENSSL_LIB}/libssl.dylib" ]; then
    FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-openssl"
    FFMPEG_CFLAGS="$FFMPEG_CFLAGS -I${FFMPEG_DEP_OPENSSL_INC}"
    FFMPEG_DEP_LIBS="$FFMPEG_CFLAGS -L${FFMPEG_DEP_OPENSSL_LIB} -lssl -lcrypto"
    cp -f /usr/local/opt/openssl/lib/pkgconfig/* ${PKG_CONFIG_PATH}
else
   echo "openssl not found"
fi


FFMPEG_DEP_LIBSRT_INC=${FF_BUILD_ROOT}/build/libsrt-x86_64/output/include
FFMPEG_DEP_LIBSRT_LIB=${FF_BUILD_ROOT}/build/libsrt-x86_64/output/lib

if [ -f "${FF_BUILD_ROOT}/build/lib/libsrt.a" ]; then
    echo "detect libsrt"

    FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-libsrt"

    FFMPEG_CFLAGS="$FFMPEG_CFLAGS -I${FFMPEG_DEP_LIBSRT_INC}"
    FFMPEG_DEP_LIBS="$FFMPEG_CFLAGS -L${FFMPEG_DEP_LIBSRT_LIB} -lsrt"

fi



#--------------------
echo "\n--------------------"
echo "[*] configure"
echo "----------------------"

if [ ! -d $FF_BUILD_SOURCE ]; then
    echo ""
    echo "!! ERROR"
    echo "!! Can not find FFmpeg directory for $FF_BUILD_NAME"
    echo "!! Run 'sh init-osx.sh' first"
    echo ""
    exit 1
fi

# xcode configuration
export DEBUG_INFORMATION_FORMAT=dwarf-with-dsym


# export CFLAGS="-g -O2 -mmacosx-version-min=10.10 -arch x86_64"

cd $FF_BUILD_SOURCE
if [ -f "./config.h" ]; then
    echo 'reuse configure'
else
    echo "config: $FFMPEG_CFG_FLAGS $FF_XCRUN_CC"
    echo "extra-cflags ${FFMPEG_CFLAGS} ${FF_EXTRA_CFLAGS}"
    echo "cc ${FF_XCRUN_CC}"
    ./configure \
        $FFMPEG_CFG_FLAGS \
        --cc="$FF_XCRUN_CC" \
        $FFMPEG_CFG_CPU \
        --extra-cflags="$FFMPEG_CFLAGS $FF_EXTRA_CFLAGS" \
        --extra-cxxflags="$FFMPEG_CFLAGS" \
        --extra-ldflags="$FFMPEG_LDFLAGS $FFMPEG_DEP_LIBS"
    make clean
fi

#--------------------
echo "\n--------------------"
echo "[*] compile ffmpeg"
echo "--------------------"
cp config.* $FF_BUILD_PREFIX
make -j3 $FF_GASPP_EXPORT
make install
mkdir -p $FF_BUILD_PREFIX/include/libffmpeg
cp -f config.h $FF_BUILD_PREFIX/include/libffmpeg/config.h
