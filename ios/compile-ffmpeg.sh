#! /bin/sh


FF_TARGET=$1
set -e


if [ $FF_TARGET == "armv7" -o $FF_TARGET == "armv7s" -o $FF_TARGET == "i386" ]; then
    sh tools/do-compile-ffmpeg.sh $FF_TARGET
elif [ $FF_TARGET == "all" ]; then
    sh tools/do-compile-ffmpeg.sh armv7
    sh tools/do-compile-ffmpeg.sh armv7s
    sh tools/do-compile-ffmpeg.sh i386
elif [ $FF_TARGET == "clean" ]; then
    cd ffmpeg-armv7 && git clean -xdf && cd -
    cd ffmpeg-armv7s && git clean -xdf && cd -
    cd ffmpeg-i386 && git clean -xdf && cd -
else
    echo "You must specific target 'all, clean, armv7, armv7s, i386, ...'.\n"
    exit 1
fi
