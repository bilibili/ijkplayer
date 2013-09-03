#! /bin/sh

# This script is based on projects below
# https://github.com/kolyvan/kxmovie
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

I386_FF_CFG_FLAGS=
I386_FF_CFG_FLAGS="$I386_FF_CFG_FLAGS --arch=i386"
I386_FF_CFG_FLAGS="$I386_FF_CFG_FLAGS --cpu=i386"
I386_FF_CFG_FLAGS="$I386_FF_CFG_FLAGS --disable-asm"
I386_FF_CFG_FLAGS="$I386_FF_CFG_FLAGS --disable-mmx"
I386_FF_CFG_FLAGS="$I386_FF_CFG_FLAGS --assert-level=2"

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

export COMMON_FF_CFG_FLAGS=
source $FF_BUILD_ROOT/../tools/ffmpeg-common-profiles.sh
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
elif [ $FF_TARGET == "i386" ]; then
    echo "i386"
    FF_BUILD_NAME="ffmpeg-i386"
    FF_PLATFORM_NAME="$IPHONE_SIM_FF_PLATFORM_NAME"
    FF_SDK_NAME="$IPHONE_SIM_FF_SDK_NAME"

    FF_CFG_FLAGS="$FF_CFG_FLAGS $I386_FF_CFG_FLAGS"

    FF_EXTRA_CFLAGS=" $FF_EXTRA_CFLAGS  -arch i386"
    FF_EXTRA_LDFLAGS="$FF_EXTRA_LDFLAGS -arch i386"
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
FF_EXTRA_LDFLAGS="$FF_EXTRA_LDFLAGS --isysroot=$FF_SDK_ROOT -L$FF_LIB_ROOT"


mkdir -p $FF_PREFIX
#--------------------
echo "\n--------------------"
echo "[*] check ffmpeg env"
echo "--------------------"
FF_CFG_FLAGS="$FF_CFG_FLAGS          $COMMON_FF_CFG_FLAGS"
FF_CFG_FLAGS="$FF_CFG_FLAGS --prefix=$FF_PREFIX"
FF_CFG_FLAGS="$FF_CFG_FLAGS --target-os=$FF_TAGET_OS"


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
