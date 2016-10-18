#! /usr/bin/env bash
#
# Copyright (C) 2013-2014 Zhang Rui <bbcallen@gmail.com>
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

set -e

if [ -z "$ANDROID_NDK" -o -z "$ANDROID_NDK" ]; then
    echo "You must define ANDROID_NDK, ANDROID_SDK before starting."
    echo "They must point to your NDK and SDK directories.\n"
    exit 1
fi


REQUEST_TARGET=$1
ACT_ABI_32="armv5 armv7a x86"
ACT_ABI_64="armv5 armv7a arm64 x86 x86_64"
ACT_ABI_ALL=$ACT_ABI_64


IJK_VERSION=`../version.sh show`



wget_uploaded_version() {
    PARAM_TARGET=$1

    VERSION_URL="https://jcenter.bintray.com/tv/danmaku/ijk/media/ijkplayer-$PARAM_TARGET/$IJK_VERSION"
    echo "VERSION_URL="$VERSION_URL
    wget $VERSION_URL -O /dev/null
}

do_build_native() {
    PARAM_TARGET=$1

    wget_uploaded_version $PARAM_TARGET && echo "ijkplayer-$PARAM_TARGET $IJK_VERSION already uploaded, skip..." && return 0

    cd contrib
    ./compile-ffmpeg.sh $PARAM_TARGET
    cd ..
    ./compile-ijk.sh $PARAM_TARGET
    cd ijkplayer
    ./gradlew :ijkplayer-$PARAM_TARGET:bintrayUpload
    cd ..
}

do_build_java() {
    wget_uploaded_version java && echo "ijkplayer-$PARAM_TARGET $IJK_VERSION already uploaded, skip..." && return 0

    cd ijkplayer
    ./gradlew :ijkplayer-java:bintrayUpload
    cd ..
}

do_build_exo() {
    wget_uploaded_version exo && echo "ijkplayer-$PARAM_TARGET $IJK_VERSION already uploaded, skip..." && return 0

    cd ijkplayer
    ./gradlew :ijkplayer-exo:bintrayUpload
    cd ..
}

case "$REQUEST_TARGET" in
    armv5|armv7a|arm64|x86|x86_64)
        do_build_native $REQUEST_TARGET
    ;;
    java)
        do_build_java
    ;;
    exo)
        do_build_exo
    ;;
    all32)
        do_build_java
        for ABI in $ACT_ABI_32
        do
            do_build_native "$ABI";
        done
        do_build_exo
    ;;
    all|all64)
        do_build_java
        for ABI in $ACT_ABI_64
        do
            do_build_native "$ABI";
        done
        do_build_exo
    ;;
    *)
        echo "Usage:"
        echo "  build-on-travis.sh armv5|armv7a|arm64|x86|x86_64"
        echo "  build-on-travis.sh java"
        echo "  build-on-travis.sh exo"
        echo "  build-on-travis.sh all|all32"
        echo "  build-on-travis.sh all64"
    ;;
esac

