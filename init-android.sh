#! /bin/sh

IJK_FFMPEG_UPSTREAM=git://git.videolan.org/ffmpeg.git
IJK_FFMPEG_FORK=https://github.com/bbcallen/FFmpeg.git
IJK_FFMPEG_COMMIT=ijk-r0.0.5-dev
IJK_FFMPEG_LOCAL_REPO=extra/ffmpeg

set -e
TOOLS=tools

echo "== pull ffmpeg base =="
sh $TOOLS/pull-repo-base.sh $IJK_FFMPEG_UPSTREAM $IJK_FFMPEG_LOCAL_REPO

echo "== pull ffmpeg fork =="
sh $TOOLS/pull-repo-ref.sh $IJK_FFMPEG_FORK android/ffmpeg-armv7a ${IJK_FFMPEG_LOCAL_REPO}

cd android/ffmpeg-armv7a
git checkout ${IJK_FFMPEG_COMMIT}
cd -
