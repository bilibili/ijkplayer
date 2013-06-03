#! /bin/sh

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
    8?*)
        # we don't use 4.4.3 because it doesn't handle threads correctly.
        if test -d ${ANDROID_NDK}/toolchains/arm-linux-androideabi-4.7
        # if gcc 4.7 is present, it's there for all the archs (x86, mips, arm)
        then
            echo "NDKr$FF_NDK_REL detected"
        else
            echo "You need the NDKv8e or later"
            exit 1
        fi
    ;;
    7|8|*)
        echo "You need the NDKv8e or later"
        exit 1
    ;;
esac

FF_BUILD_ROOT=`pwd`
FF_SOURCE=$FF_BUILD_ROOT/ffmpeg-armv7a
FF_ANDROID_PLATFORM=android-9
FF_GCC_VER=4.7

FF_CFG_FLAGS="--target-os=linux --enable-small"

FF_EXTRA_CFLAGS=
FF_EXTRA_LDFLAGS=

#----- armv7a begin -----
FF_BUILD_NAME=ffmpeg-armv7a
FF_CROSS_PREFIX=arm-linux-androideabi

FF_CFG_FLAGS="$FF_CFG_FLAGS --arch=arm"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-neon"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-thumb"

FF_EXTRA_CFLAGS="$FF_EXTRA_CFLAGS -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp -mthumb"
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
export CC="ccache ${FF_CROSS_PREFIX}-gcc"
export LD=${FF_CROSS_PREFIX}-ld
export AR=${FF_CROSS_PREFIX}-ar
export STRIP=${FF_CROSS_PREFIX}-strip

FF_CFLAGS="-O3 -Wall -pipe -fpic -fasm \
    -ffast-math \
    -fstrict-aliasing -Werror=strict-aliasing \
    -fmodulo-sched -fmodulo-sched-allow-regmoves \
    -Wno-psabi -Wa,--noexecstack \
    -D__ARM_ARCH_5__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5TE__ \
    -DANDROID -DNDEBUG"

# --enable-thumb is OK
#FF_CFLAGS="$FF_CFLAGS -mthumb"

# not necessary
#FF_CFLAGS="$FF_CFLAGS -finline-limit=300"

FF_CFG_FLAGS="$FF_CFG_FLAGS \
    --prefix=$FF_PREFIX \
    --enable-cross-compile \
    --cross-prefix=${FF_CROSS_PREFIX}- \
    --enable-shared \
    --disable-symver \
    --disable-doc \
    --disable-ffplay \
    --disable-ffmpeg \
    --disable-ffprobe \
    --disable-ffserver \
    --disable-avdevice \
    --disable-avfilter \
    --disable-encoders \
    --disable-muxers \
    --disable-bsfs \
    --disable-filters \
    --disable-devices \
    --disable-everything \
    --enable-protocols  \
    --enable-parsers \
    --enable-demuxers \
    --disable-demuxer=sbg \
    --enable-decoders \
    --enable-network \
    --enable-swscale  \
    --enable-asm \
    --enable-version3"

echo $FF_CFG_FLAGS

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
rm libavcodec/log2_tab.o
rm libavutil/log2_tab.o
rm libswresample/log2_tab.o
$CC -lm -lz -shared --sysroot=$FF_SYSROOT -Wl,--no-undefined -Wl,-z,noexecstack $FF_EXTRA_LDFLAGS \
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
$STRIP --strip-unneeded $FF_PREFIX/libffmpeg.so

