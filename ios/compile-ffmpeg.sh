#! /usr/bin/env bash

#----------
# modify for your build tool
# export HAVE_IOS7_SDK=1 to prefer ios sdk7

FF_ALL_ARCHS_IOS6_SDK="armv7 armv7s i386"
FF_ALL_ARCHS_IOS7_SDK="armv7 armv7s arm64 i386"

FF_ALL_ARCHS=$FF_ALL_ARCHS_IOS6_SDK
if [ "$HAVE_IOS7_SDK" == "1" ]; then
    FF_ALL_ARCHS=$FF_ALL_ARCHS_IOS7_SDK
fi

#----------
UNI_BUILD_ROOT=`pwd`
UNI_TMP="$UNI_BUILD_ROOT/tmp"
UNI_TMP_LLVM_VER_FILE="$UNI_TMP/llvm.ver.txt"
FF_TARGET=$1
set -e

#----------
FF_LIBS="libavcodec libavformat libavutil libswscale libswresample"

#----------
echo_archs() {
    echo "===================="
    echo "[*] check xcode version"
    echo "===================="
    echo "HAVE_IOS7_SDK = $HAVE_IOS7_SDK"
    echo "FF_ALL_ARCHS = $FF_ALL_ARCHS"
}

do_lipo () {
    LIB_FILE=$1
    LIPO_FLAGS=
    for ARCH in $FF_ALL_ARCHS
    do
        LIPO_FLAGS="$LIPO_FLAGS $UNI_BUILD_ROOT/build/ffmpeg-$ARCH/output/lib/$LIB_FILE"
    done

    lipo -create $LIPO_FLAGS -output $UNI_BUILD_ROOT/build/universal/lib/$LIB_FILE
    lipo -info $UNI_BUILD_ROOT/build/universal/lib/$LIB_FILE
}

do_lipo_all () {
    mkdir -p $UNI_BUILD_ROOT/build/universal/lib
    echo "lipo archs: $FF_ALL_ARCHS"
    for FF_LIB in $FF_LIBS
    do
        do_lipo "$FF_LIB.a";
    done

    cp -R $UNI_BUILD_ROOT/build/ffmpeg-armv7/output/include $UNI_BUILD_ROOT/build/universal/
}

#----------
if [ "$FF_TARGET" = "armv7" -o "$FF_TARGET" = "armv7s" -o "$FF_TARGET" = "arm64" ]; then
    echo_archs
    sh tools/do-compile-ffmpeg.sh $FF_TARGET
elif [ "$FF_TARGET" = "i386" -o "$FF_TARGET" = "x86_64" ]; then
    echo_archs
    sh tools/do-compile-ffmpeg.sh $FF_TARGET
elif [ "$FF_TARGET" = "lipo" ]; then
    echo_archs
    do_lipo_all
elif [ "$FF_TARGET" = "all" ]; then
    echo_archs
    for ARCH in $FF_ALL_ARCHS
    do
        sh tools/do-compile-ffmpeg.sh $ARCH
    done

    do_lipo_all
elif [ "$FF_TARGET" == "check" ]; then
    echo_archs
elif [ "$FF_TARGET" == "clean" ]; then
    echo_archs
    for ARCH in $FF_ALL_ARCHS
    do
        cd ffmpeg-$ARCH && git clean -xdf && cd -
    done
else
    echo "Usage:"
    echo "  compile-ffmpeg.sh armv7|armv7s|arm64|i386|x86_64"
    echo "  compile-ffmpeg.sh lipo"
    echo "  compile-ffmpeg.sh all"
    echo "  compile-ffmpeg.sh clean"
    echo "  compile-ffmpeg.sh check"
    exit 1
fi
