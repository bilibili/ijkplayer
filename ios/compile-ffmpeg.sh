#! /bin/sh


UNI_BUILD_ROOT=`pwd`
FF_TARGET=$1
set -e




do_lipo () {
    LIB_FILE=$1
    lipo -create \
        -arch armv7 $UNI_BUILD_ROOT/build/ffmpeg-armv7/output/lib/$LIB_FILE \
        -arch armv7 $UNI_BUILD_ROOT/build/ffmpeg-armv7s/output/lib/$LIB_FILE \
        -arch i386 $UNI_BUILD_ROOT/build/ffmpeg-i386/output/lib/$LIB_FILE \
        -output $UNI_BUILD_ROOT/build/universal/lib/$LIB_FILE
}


if [ "$FF_TARGET" == "armv7" -o "$FF_TARGET" == "armv7s" -o "$FF_TARGET" == "i386" ]; then
    sh tools/do-compile-ffmpeg.sh $FF_TARGET
elif [ "$FF_TARGET" == "all" ]; then
    sh tools/do-compile-ffmpeg.sh armv7
    sh tools/do-compile-ffmpeg.sh armv7s
    sh tools/do-compile-ffmpeg.sh i386

    mkdir -p $UNI_BUILD_ROOT/build/universal/lib
    do_lipo libavcodec.a
    do_lipo libavformat.a
    do_lipo libavutil.a
    do_lipo libswresample.a
    do_lipo libswscale.a

    cp -R $UNI_BUILD_ROOT/build/ffmpeg-armv7/output/include $UNI_BUILD_ROOT/build/universal/

elif [ "$FF_TARGET" == "clean" ]; then
    cd ffmpeg-armv7 && git clean -xdf && cd -
    cd ffmpeg-armv7s && git clean -xdf && cd -
    cd ffmpeg-i386 && git clean -xdf && cd -
else
    echo "You must specific target 'all, clean, armv7, armv7s, i386, ...'.\n"
    exit 1
fi