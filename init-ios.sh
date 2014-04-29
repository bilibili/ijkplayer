#! /usr/bin/env bash

IJK_FFMPEG_UPSTREAM=git://git.videolan.org/ffmpeg.git
IJK_FFMPEG_FORK=https://github.com/bbcallen/FFmpeg.git
IJK_FFMPEG_COMMIT=ijk-r0.1.0-dev
IJK_FFMPEG_LOCAL_REPO=extra/ffmpeg

# gas-preprocessor backup
# https://github.com/bbcallen/gas-preprocessor.git

set -e
TOOLS=tools

echo "== pull gas-preprocessor base =="
sh $TOOLS/pull-repo-base.sh git://git.libav.org/gas-preprocessor.git extra/gas-preprocessor

echo "== pull ffmpeg base =="
sh $TOOLS/pull-repo-base.sh $IJK_FFMPEG_UPSTREAM ${IJK_FFMPEG_LOCAL_REPO}

echo "== pull ffmpeg fork armv7 =="
sh $TOOLS/pull-repo-ref.sh $IJK_FFMPEG_FORK ios/ffmpeg-armv7 ${IJK_FFMPEG_LOCAL_REPO}

cd ios/ffmpeg-armv7
git checkout ${IJK_FFMPEG_COMMIT}
cd -

echo "== pull ffmpeg fork armv7s =="
sh $TOOLS/pull-repo-ref.sh $IJK_FFMPEG_FORK ios/ffmpeg-armv7s ${IJK_FFMPEG_LOCAL_REPO}

cd ios/ffmpeg-armv7s
git checkout ${IJK_FFMPEG_COMMIT}
cd -

echo "== pull ffmpeg fork arm64 =="
sh $TOOLS/pull-repo-ref.sh $IJK_FFMPEG_FORK ios/ffmpeg-arm64 ${IJK_FFMPEG_LOCAL_REPO}

cd ios/ffmpeg-arm64
git checkout ${IJK_FFMPEG_COMMIT}
cd -

echo "== pull ffmpeg fork i386 =="
sh $TOOLS/pull-repo-ref.sh $IJK_FFMPEG_FORK ios/ffmpeg-i386 ${IJK_FFMPEG_LOCAL_REPO}

cd ios/ffmpeg-i386
git checkout ${IJK_FFMPEG_COMMIT}
cd -
