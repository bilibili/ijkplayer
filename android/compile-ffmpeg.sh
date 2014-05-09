#! /usr/bin/env bash

# This script is based on projects below
# https://github.com/yixia/FFmpeg-Android
# http://git.videolan.org/?p=vlc-ports/android.git;a=summary

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

FF_EXTRA_CFLAGS=
FF_EXTRA_LDFLAGS=

#----- armv7a begin -----
FF_TAGET_OS=linux
FF_BUILD_NAME=ffmpeg-armv7a
FF_SOURCE=$FF_BUILD_ROOT/$FF_BUILD_NAME
FF_CROSS_PREFIX=arm-linux-androideabi

FF_CFG_FLAGS="$FF_CFG_FLAGS --arch=arm --cpu=cortex-a8"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-neon"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-thumb"

FF_EXTRA_CFLAGS="$FF_EXTRA_CFLAGS -march=armv7-a -mcpu=cortex-a8 -mfpu=vfpv3-d16 -mfloat-abi=softfp -mthumb"
FF_EXTRA_LDFLAGS="$FF_EXTRA_LDFLAGS -Wl,--fix-cortex-a8"
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
echo "[*] check ffmpeg env"
echo "--------------------"
export PATH=$FF_TOOLCHAIN_PATH/bin:$PATH
#export CC="ccache ${FF_CROSS_PREFIX}-gcc"
export CC="${FF_CROSS_PREFIX}-gcc"
export LD=${FF_CROSS_PREFIX}-ld
export AR=${FF_CROSS_PREFIX}-ar
export STRIP=${FF_CROSS_PREFIX}-strip

FF_CFLAGS="-O3 -Wall -pipe \
    -std=c99 \
    -ffast-math \
    -fstrict-aliasing -Werror=strict-aliasing \
    -Wno-psabi -Wa,--noexecstack \
    -DANDROID -DNDEBUG"

# cause av_strlcpy crash with gcc4.7, gcc4.8
# -fmodulo-sched -fmodulo-sched-allow-regmoves

# --enable-thumb is OK
#FF_CFLAGS="$FF_CFLAGS -mthumb"

# not necessary
#FF_CFLAGS="$FF_CFLAGS -finline-limit=300"

export COMMON_FF_CFG_FLAGS=
source $FF_BUILD_ROOT/../tools/ffmpeg-common-profiles.sh

FF_CFG_FLAGS="$FF_CFG_FLAGS $COMMON_FF_CFG_FLAGS"

#--------------------
# Standard options:
FF_CFG_FLAGS="$FF_CFG_FLAGS --prefix=$FF_PREFIX"

# Advanced options (experts only):
FF_CFG_FLAGS="$FF_CFG_FLAGS --cross-prefix=${FF_CROSS_PREFIX}-"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-cross-compile"
FF_CFG_FLAGS="$FF_CFG_FLAGS --target-os=$FF_TAGET_OS"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-pic"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-symver"

# Optimization options (experts only):
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-asm"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-inline-asm"

#--------------------
echo "\n--------------------"
echo "[*] configurate ffmpeg"
echo "--------------------"
cd $FF_SOURCE
if [ -f "./config.h" ]; then
    echo 'reuse configure'
else
    ./configure $FF_CFG_FLAGS \
        --extra-cflags="$FF_CFLAGS $FF_EXTRA_CFLAGS" \
        --extra-ldflags="$FF_EXTRA_LDFLAGS"
    make clean
fi

#--------------------
echo "\n--------------------"
echo "[*] compile ffmpeg"
echo "--------------------"
cp config.* $FF_PREFIX
make $FF_MAKEFLAGS
make install

#--------------------
echo "\n--------------------"
echo "[*] link ffmpeg"
echo "--------------------"
$CC -lm -lz -shared --sysroot=$FF_SYSROOT -Wl,--no-undefined -Wl,-z,noexecstack $FF_EXTRA_LDFLAGS \
    compat/*.o \
    libavutil/*.o \
    libavutil/arm/*.o \
    libavcodec/*.o \
    libavcodec/arm/*.o \
    libavformat/*.o \
    libswresample/*.o \
    libswresample/arm/*.o \
    libswscale/*.o \
    -o $FF_PREFIX/libffmpeg.so

cp $FF_PREFIX/libffmpeg.so $FF_PREFIX/libffmpeg-debug.so
cp $FF_PREFIX/libffmpeg.so $FF_PREFIX/libffmpeg-release.so
$STRIP --strip-unneeded $FF_PREFIX/libffmpeg-release.so

echo "\n--------------------"
echo "[*] Finished"
echo "--------------------"
echo "# to continue to build ijkplayer, run script below,"
echo "sh compile-ijk.sh "
