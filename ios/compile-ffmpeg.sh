#! /bin/sh

# This script is based on projects below
# https://github.com/yixia/FFmpeg-Android
# http://git.videolan.org/?p=vlc-ports/android.git;a=summary

#--------------------
echo "===================="
echo "[*] check env"
echo "===================="
set -e


FF_TARGET=$1
if [ -z "$FF_TARGET" ]; then
    echo "You must specific target 'armv7, armv7s, ...'.\n"
    exit 1
fi


XCODE_ROOT="/Applications/Xcode4.6.3.app/Contents/Developer/Platforms"


FF_BUILD_ROOT=`pwd`
FF_TAGET_OS="darwin"


FF_CFG_FLAGS=
FF_EXTRA_CFLAGS=
FF_EXTRA_LDFLAGS=
FF_PLATFORM_NAME=
FF_SDK_NAME=


echo "===================="
echo "[*] config target"
echo "===================="
#----- common begin -----
FF_TOOLS_ROOT="$FF_BUILD_ROOT/../extra"
# FIXME: REMOVE ME
GAS_PREP_PATH="$FF_BUILD_ROOT/../extra/gas-preprocessor/gas-preprocessor.pl"

IPHONE_IOS_FF_PLATFORM_NAME="iPhoneOS.platform"
IPHONE_SIM_FF_PLATFORM_NAME="iPhoneSimulator.platform"

IPHONE_IOS_FF_SDK_NAME="iPhoneOS6.1.sdk"
IPHONE_SIM_FF_SDK_NAME="iPhoneSimulator6.1.sdk"

ARMV7_FF_CFG_FLAGS=
ARMV7_FF_CFG_FLAGS="$ARMV7_FF_CFG_FLAGS --arch=arm"
ARMV7_FF_CFG_FLAGS="$ARMV7_FF_CFG_FLAGS --cpu=cortex-a8"
ARMV7_FF_CFG_FLAGS="$ARMV7_FF_CFG_FLAGS --enable-pic"
ARMV7_FF_CFG_FLAGS="$ARMV7_FF_CFG_FLAGS --enable-neon"
ARMV7_FF_CFG_FLAGS="$ARMV7_FF_CFG_FLAGS --enable-optimizations"
ARMV7_FF_CFG_FLAGS="$ARMV7_FF_CFG_FLAGS --disable-debug"
ARMV7_FF_CFG_FLAGS="$ARMV7_FF_CFG_FLAGS --enable-small"

ARMV7_FF_EXTRA_CFLAGS="-mfpu=neon -mfloat-abi=softfp -mvectorize-with-neon-quad"
ARMV7_FF_EXTRA_LDFLAGS="-Wl"
#----- common end -----


if [ $FF_TARGET == "armv7" ]; then
    echo "armv7"
    FF_BUILD_NAME="ffmpeg-armv7"
    FF_PLATFORM_NAME="$IPHONE_IOS_FF_PLATFORM_NAME"
    FF_SDK_NAME="$IPHONE_IOS_FF_SDK_NAME"

    FF_CFG_FLAGS="$FF_CFG_FLAGS $ARMV7_FF_CFG_FLAGS"

    FF_EXTRA_CFLAGS=" $FF_EXTRA_CFLAGS  $ARMV7_FF_EXTRA_CFLAGS  -arch armv7"
    FF_EXTRA_LDFLAGS="$FF_EXTRA_LDFLAGS $ARMV7_FF_EXTRA_LDFLAGS -arch armv7"
elif [ $FF_TARGET == "armv7s" ]; then
    echo "armv7s"
    FF_BUILD_NAME="ffmpeg-armv7s"
    FF_PLATFORM_NAME="$IPHONE_IOS_FF_PLATFORM_NAME"
    FF_SDK_NAME="$IPHONE_IOS_FF_SDK_NAME"

    FF_CFG_FLAGS="$FF_CFG_FLAGS $ARMV7_FF_CFG_FLAGS"

    FF_EXTRA_CFLAGS=" $FF_EXTRA_CFLAGS  $ARMV7_FF_EXTRA_CFLAGS  -arch armv7s"
    FF_EXTRA_LDFLAGS="$FF_EXTRA_LDFLAGS $ARMV7_FF_EXTRA_LDFLAGS -arch armv7s"
else
    echo "unknown target $FF_TARGET";
    exit 1
fi


echo "===================="
echo "[*] make ios toolchain"
echo "===================="

FF_SOURCE="$FF_BUILD_ROOT/$FF_BUILD_NAME"
FF_PREFIX="$FF_BUILD_ROOT/build/$FF_BUILD_NAME/output"

FF_PLATFORM_ROOT="$XCODE_ROOT/$FF_PLATFORM_NAME"
FF_SDK_ROOT="$FF_PLATFORM_ROOT/Developer/SDKs/$FF_SDK_NAME"
FF_LIB_ROOT="$FF_PLATFORM_ROOT/Developer/SDKs/$FF_SDK_NAME/usr/lib/system"

FF_CC_FLAG="$FF_PLATFORM_ROOT/Developer/usr/bin/gcc"
# FIXME: REMOVE ME
FF_AS_FLAG="$GAS_PREP_PATH $FF_CC_FLAG"
echo "SDK: $FF_SDK_ROOT"


export PATH="$FF_TOOLS_ROOT/gas-preprocessor:$PATH"

FF_CFG_FLAGS="$FF_CFG_FLAGS --cc=$FF_CC_FLAG"
# FIXME: REMOVE ME
# FF_CFG_FLAGS="$FF_CFG_FLAGS --as=\"$FF_AS_FLAG\""
FF_CFG_FLAGS="$FF_CFG_FLAGS --sysroot=$FF_SDK_ROOT"
FF_EXTRA_LDFLAGS="$FF_EXTRA_LDFLAGS -L$FF_LIB_ROOT"


mkdir -p $FF_PREFIX
#--------------------
echo "\n--------------------"
echo "[*] check ffmpeg env"
echo "--------------------"
FF_CFLAGS="-O3 -Wall -pipe \
    -std=c99 \
    -ffast-math \
    -fstrict-aliasing -Werror=strict-aliasing \
    -Wa,--noexecstack \
    -DNDEBUG"
FF_CFLAGS=

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
# FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-small"
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

FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-decoders"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=aac"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=aac_latm"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=ac3"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=ape"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=flac"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=flv"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=h261"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=h263"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=h263i"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=h263p"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=h264"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=mp3"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=mp3float"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=mp3on4"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=mp3on4float"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=mpeg4"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=msmpeg4v1"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=msmpeg4v2"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=msmpeg4v3"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=pcm_*"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=ra_144"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=ra_288"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=ralf"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=rv10"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=rv20"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=rv30"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=rv40"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=theora"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=vc1"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=vorbis"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=vp3"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=vp5"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=vp6"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=vp6a"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=vp6f"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=vp8"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=webp"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=wmalossless"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=wmapro"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=wmav1"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=wmav2"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=wmv1"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=wmv2"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-decoder=wmv3"

FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-hwaccels"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-muxers"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-muxer=mpegts"

FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-demuxers"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=aac"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=ac3"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=ape"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=asf"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=concat"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=data"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=flac"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=flv"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=h261"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=h263"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=h264"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=hls"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=latm"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=loas"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=m4v"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=matroska"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=mov"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=mp3"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=mpegps"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=mpegts"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=mpegtsraw"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=mpegvideo"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=ogg"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=pcm_*"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=rm"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=rtp"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=rtsp"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=swf"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=tta"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=vc1"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=vc1t"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=wav"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=xmv"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-demuxer=xwma"

FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-parsers"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=aac"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=acc_latm"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=ac3"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=adx"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=bmp"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=cavsvideo"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=cook"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=dca"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=dirac"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=dnxhd"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=dvbsub"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=dvd_nav"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=dvdsub"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=flac"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=gsm"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=h261"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=h263"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=h264"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=mjpeg"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=mlp"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=mpeg4video"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=mpegaudip"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=mpegvideo"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=png"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=pnm"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=rv30"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=rv40"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=tak"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=vc1"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=vorbis"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=vp3"
# FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-parser=vp8"

FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-bsfs"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-protocols"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-devices"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-filters"

# External library support:
# ...

# Advanced options (experts only):
# FF_CFG_FLAGS="$FF_CFG_FLAGS --cross-prefix=${FF_CROSS_PREFIX}-"
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
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-armv5te"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-armv6"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-armv6t2"
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
    echo "config: $FF_CFG_FLAGS"
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
