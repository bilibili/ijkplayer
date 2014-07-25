#! /usr/bin/env bash

IJK_FFMPEG_UPSTREAM=git://git.videolan.org/ffmpeg.git
IJK_FFMPEG_FORK=https://github.com/bbcallen/FFmpeg.git
IJK_FFMPEG_COMMIT=ijk-r0.1.3-dev
IJK_FFMPEG_LOCAL_REPO=extra/ffmpeg

# gas-preprocessor backup
# https://github.com/bbcallen/gas-preprocessor.git

set -e
TOOLS=tools

echo "== pull gas-preprocessor base =="
sh $TOOLS/pull-repo-base.sh git://git.libav.org/gas-preprocessor.git extra/gas-preprocessor

echo "== pull ffmpeg base =="
sh $TOOLS/pull-repo-base.sh $IJK_FFMPEG_UPSTREAM $IJK_FFMPEG_LOCAL_REPO

function pull_fork()
{
    echo "== pull ffmpeg fork $1 =="
    sh $TOOLS/pull-repo-ref.sh $IJK_FFMPEG_FORK ios/ffmpeg-$1 ${IJK_FFMPEG_LOCAL_REPO}
    cd ios/ffmpeg-$1
    git checkout ${IJK_FFMPEG_COMMIT}
    cd -
}

pull_fork "armv7"
pull_fork "armv7s"
pull_fork "arm64"
pull_fork "i386"
pull_fork "x86_64"
