#! /bin/sh

IJK_FFMPEG_COMMIT=ijk-r0.0.5-dev
IJK_FFMPEG_LOCAL_REPO=extra/ffmpeg

set -e


echo "== pull ffmpeg base =="
if [ ! -d ${IJK_FFMPEG_LOCAL_REPO} ]; then
    git clone git://git.videolan.org/ffmpeg.git ${IJK_FFMPEG_LOCAL_REPO}
else
    cd ${IJK_FFMPEG_LOCAL_REPO}
    git pull
    cd -
fi


echo "== pull ffmpeg fork =="
if [ ! -d "android/ffmpeg-armv7a" ]; then
    git clone --reference "${IJK_FFMPEG_LOCAL_REPO}" https://github.com/bbcallen/FFmpeg.git android/ffmpeg-armv7a
    cd android/ffmpeg-armv7a

# do not depend on the reference repo
    git repack -a
else
    echo "== pull ffmpeg base =="
    cd android/ffmpeg-armv7a
    git fetch --all
fi

git checkout ${IJK_FFMPEG_COMMIT}
cd -
