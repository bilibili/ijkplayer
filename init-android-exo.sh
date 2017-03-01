#! /usr/bin/env bash
#
# Copyright (C) 2013-2015 Zhang Rui <bbcallen@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

IJK_EXO_UPSTREAM=https://github.com/google/ExoPlayer.git
IJK_EXO_FORK=https://github.com/google/ExoPlayer.git
IJK_EXO_COMMIT=r1.5.9
IJK_EXO_LOCAL_REPO=extra/ExoPlayer

set -e
TOOLS=tools

echo "== pull ExoPlayer base =="
sh $TOOLS/pull-repo-base.sh $IJK_EXO_UPSTREAM $IJK_EXO_LOCAL_REPO

echo "== pull ExoPlayer fork =="
cd extra/ExoPlayer
git checkout ${IJK_EXO_COMMIT} -B ijkplayer
cd -

SRC_EXO_DIR=extra/ExoPlayer/demo/src/main/java/com/google/android/exoplayer/demo
DST_EXO_DIR=android/ijkplayer/ijkplayer-exo/src/main/java/tv/danmaku/ijk/media/exo/demo

mkdir -p $DST_EXO_DIR/player

function install_java()
{
    JAVA_FILE=$1
    cat $SRC_EXO_DIR/$JAVA_FILE \
    | sed "s/^package com.google.android.exoplayer.demo/package tv.danmaku.ijk.media.exo.demo/g" \
    | sed "s/^import com.google.android.exoplayer.demo/import tv.danmaku.ijk.media.exo.demo/g" \
    | sed "s/@link/link/g" \
    > $DST_EXO_DIR/$JAVA_FILE
}

install_java player/DashRendererBuilder.java
install_java player/DemoPlayer.java
install_java player/ExtractorRendererBuilder.java
install_java player/HlsRendererBuilder.java
install_java player/SmoothStreamingRendererBuilder.java
install_java EventLogger.java
install_java SmoothStreamingTestMediaDrmCallback.java
# install_java WidevineTestMediaDrmCallback.java
