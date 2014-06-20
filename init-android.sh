#! /usr/bin/env bash

IJK_FFMPEG_UPSTREAM=git://git.videolan.org/ffmpeg.git
IJK_FFMPEG_FORK=https://github.com/bbcallen/FFmpeg.git
IJK_FFMPEG_COMMIT=ijk-r0.1.2-dev
IJK_FFMPEG_LOCAL_REPO=extra/ffmpeg

IJK_OPENSSL_UPSTREAM=https://github.com/openssl/openssl
IJK_OPENSSL_FORK=https://github.com/bbcallen/openssl.git
IJK_OPENSSL_COMMIT=OpenSSL_1_0_1g
IJK_OPENSSL_LOCAL_REPO=extra/openssl

set -e
TOOLS=tools



echo "== pull ffmpeg base =="
sh $TOOLS/pull-repo-base.sh $IJK_FFMPEG_UPSTREAM $IJK_FFMPEG_LOCAL_REPO

echo "== pull ffmpeg fork armv7a =="
sh $TOOLS/pull-repo-ref.sh $IJK_FFMPEG_FORK android/ffmpeg-armv7a ${IJK_FFMPEG_LOCAL_REPO}
cd android/ffmpeg-armv7a
git checkout ${IJK_FFMPEG_COMMIT}
cd -

echo "== pull ffmpeg fork armv5 =="
sh $TOOLS/pull-repo-ref.sh $IJK_FFMPEG_FORK android/ffmpeg-armv5 ${IJK_FFMPEG_LOCAL_REPO}
cd android/ffmpeg-armv5
git checkout ${IJK_FFMPEG_COMMIT}
cd -

echo "== pull ffmpeg fork x86 =="
sh $TOOLS/pull-repo-ref.sh $IJK_FFMPEG_FORK android/ffmpeg-x86 ${IJK_FFMPEG_LOCAL_REPO}
cd android/ffmpeg-x86
git checkout ${IJK_FFMPEG_COMMIT}
cd -



echo "== pull openssl base =="
sh $TOOLS/pull-repo-base.sh $IJK_OPENSSL_UPSTREAM $IJK_OPENSSL_LOCAL_REPO

echo "== pull openssl fork =="
sh $TOOLS/pull-repo-ref.sh $IJK_OPENSSL_FORK android/openssl-armv7a ${IJK_OPENSSL_LOCAL_REPO}


