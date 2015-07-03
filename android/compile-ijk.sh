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

if [ -z "$ANDROID_NDK" -o -z "$ANDROID_NDK" ]; then
    echo "You must define ANDROID_NDK, ANDROID_SDK before starting."
    echo "They must point to your NDK and SDK directories.\n"
    exit 1
fi

REQUEST_TARGET=$1
ALL_ABI="armv5 armv7a x86 arm64-v8a"

do_ndk_build () {
    PARAM_TARGET=$1
    case "$PARAM_TARGET" in
        armv7a)
            cd ijkmediaplayer/jni
            $ANDROID_NDK/ndk-build
            cd -
        ;;
        armv5)
            cd ijkmediaplayer-armv5/jni
            $ANDROID_NDK/ndk-build
            cd -
        ;;
        x86)
            cd ijkmediaplayer-x86/jni
            $ANDROID_NDK/ndk-build
            cd -
        ;;
        arm64-v8a)
            cd ijkmediaplayer-arm64-v8a/jni
            $ANDROID_NDK/ndk-build
            cd -
        ;;
    esac
}


case "$REQUEST_TARGET" in
    "")
        do_ndk_build armv7a;
    ;;
    armv5|armv7a|x86|arm64-v8a)
        do_ndk_build $REQUEST_TARGET;
    ;;
    all)
        for ABI in $ALL_ABI
        do
            do_ndk_build "$ABI";
        done
    ;;
    *)
        echo "Usage:"
        echo "  compile-ijk.sh armv5|armv7a|x86|arm64-v8a"
        echo "  compile-ijk.sh all"
    ;;
esac

