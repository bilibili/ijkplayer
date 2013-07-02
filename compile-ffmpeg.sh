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
FF_ANDROID_PLATFORM=android-9
FF_GCC_VER=4.7



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
export CC="ccache ${FF_CROSS_PREFIX}-gcc"
export LD=${FF_CROSS_PREFIX}-ld
export AR=${FF_CROSS_PREFIX}-ar
export STRIP=${FF_CROSS_PREFIX}-strip

FF_CFLAGS="-O3 -Wall -pipe \
    -std=c99 \
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

#--------------------
# Standard options:
FF_CFG_FLAGS="$FF_CFG_FLAGS --prefix=$FF_PREFIX"

# Licensing options:
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-gpl"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-version3"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-nonfree"

# Configuration options:
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-static"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-shared"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-small"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-runtime-cpudetect"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-gray"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-swscale-alpha"

# Program options:
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-programs"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-ffmpeg"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-ffplay"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-ffprobe"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-ffserver"

# Documentation options:
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-doc"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-htmlpages"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-manpages"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-podpages"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-txtpages"

# Component options:
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-avdevice"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-avcodec"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-avformat"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-avutil"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-swresample"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-swscale"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-postproc"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-avfilter"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-avresample"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-pthreads"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-w32threads"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-os2threads"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-network"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-dct"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-dwt"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-lsp"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-lzo"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-mdct"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-rdft"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-fft"

# Hardware accelerators:
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-dxva2"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-vaapi"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-vda"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-vdpau"

# Individual component options:
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-everything"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-encoders"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoders"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-hwaccels"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-muxers"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxers"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-parsers"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-bsfs"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-protocols"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-devices"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-filters"

# External library support:
# ...

# Advanced options (experts only):
FF_CFG_FLAGS="$FF_CFG_FLAGS --cross-prefix=${FF_CROSS_PREFIX}-"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-cross-compile"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --sysroot=PATH"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --sysinclude=PATH"
FF_CFG_FLAGS="$FF_CFG_FLAGS --target-os=$FF_TAGET_OS"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --target-exec=CMD"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --target-path=DIR"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --toolchain=NAME"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --nm=NM"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --ar=AR"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --as=AS"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --yasmexe=EXE"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --cc=CC"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --cxx=CXX"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --dep-cc=DEPCC"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --ld=LD"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --host-cc=HOSTCC"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --host-cflags=HCFLAGS"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --host-cppflags=HCPPFLAGS"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --host-ld=HOSTLD"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --host-ldflags=HLDFLAGS"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --host-libs=HLIBS"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --host-os=OS"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --extra-cflags=ECFLAGS"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --extra-cxxflags=ECFLAGS"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --extra-ldflags=ELDFLAGS"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --extra-libs=ELIBS"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --extra-version=STRING"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --optflags=OPTFLAGS"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --build-suffix=SUFFIX"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --malloc-prefix=PREFIX"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --progs-suffix=SUFFIX"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --arch=ARCH"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --cpu=CPU"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-pic"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-sram"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-thumb"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-symver"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-hardcoded-tables"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-safe-bitstream-reader"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-memalign-hack"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-lto"

# Optimization options (experts only):
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-asm"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-altivec"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-amd3dnow"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-amd3dnowext"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-mmx"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-mmxext"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-sse"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-sse2"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-sse3"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-ssse3"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-sse4"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-sse42"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-avx"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-fma4"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-armv5te"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-armv6"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-armv6t2"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-vfp"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-neon"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-vis"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-inline-asm"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-yasm"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-mips32r2"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-mipsdspr1"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-mipsdspr2"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-mipsfpu"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-fast-unaligned"

# Developer options (useful when working on FFmpeg itself):
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-coverage"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-debug"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-debug=LEVEL"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-optimizations"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-extra-warnings"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-stripping"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --assert-level=level"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-memory-poisoning"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --valgrind=VALGRIND"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-ftrapv"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --samples=PATH"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-xmm-clobber-test"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-random"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-random"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-random=LIST"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-random=LIST"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --random-seed=VALUE"

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
cp $FF_PREFIX/libffmpeg.so $FF_PREFIX/libffmpeg-release.so
$STRIP --strip-unneeded $FF_PREFIX/libffmpeg-release.so

