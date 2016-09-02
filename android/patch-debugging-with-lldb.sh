#! /usr/bin/env bash
#
# Copyright (C) 2013-2014 Chen Hui <calmer91@gmail.com>
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

patch_enable () {
    PARAM_TARGET=$1
    case "$PARAM_TARGET" in
        armv5|armv7a|arm64|x86|x86_64)
            git apply -- android/patches/0001-gitignore-ignore-.externalNativeBuild.patch
            echo "git apply ==> patches/0001-gitignore-ignore-.externalNativeBuild.patch"
            git apply -- android/patches/0002-gradle-upgrade-build-tool-to-2.2.0-beta2.patch
            echo "git apply ==> patches/0002-gradle-upgrade-build-tool-to-2.2.0-beta2.patch"
            git apply -- android/patches/0003-$PARAM_TARGET-enable-debugging-with-LLDB.patch
            echo "git apply ==> patches/0003-$PARAM_TARGET-enable-debugging-with-LLDB.patch"
            git apply -- android/patches/0004-$PARAM_TARGET-link-prebuilt-staic-libraries-of-ffmepg.patch
            echo "git apply ==> patches/0004-$PARAM_TARGET-link-prebuilt-staic-libraries-of-ffmepg.patch"
        ;;
    esac
}

patch_disable () {
    PARAM_TARGET=$1
    case "$PARAM_TARGET" in
        armv5|armv7a|arm64|x86|x86_64)
            git apply -R android/patches/0004-$PARAM_TARGET-link-prebuilt-staic-libraries-of-ffmepg.patch
            echo "git apply reverse ==> patches/0004-$PARAM_TARGET-link-prebuilt-staic-libraries-of-ffmepg.patch"

            git checkout android/ijkplayer/ijkplayer-$PARAM_TARGET/src/main/jni/Android.mk
            git checkout android/ijkplayer/ijkplayer-example/build.gradle
            git checkout android/ijkplayer/ijkplayer-$PARAM_TARGET/build.gradle
            git checkout android/ijkplayer/settings.gradle
#            git apply -R android/patches/0003-$PARAM_TARGET-enable-debugging-with-LLDB.patch
            echo "git apply reverse ==> patches/0003-$PARAM_TARGET-enable-debugging-with-LLDB.patch"
            git apply -R android/patches/0002-gradle-upgrade-build-tool-to-2.2.0-beta2.patch
            echo "git apply reverse ==> patches/0002-gradle-upgrade-build-tool-to-2.2.0-beta2.patch"
            git apply -R android/patches/0001-gitignore-ignore-.externalNativeBuild.patch
            echo "git apply reverse ==> patches/0001-gitignore-ignore-.externalNativeBuild.patch"
        ;;
    esac
}

case "$1" in
    armv5|armv7a|arm64|x86|x86_64)
        # patch_enable $1
        echo "patch apply ==> $1"
        patch_enable $1
    ;;
    reverse)
        case "$2" in
            armv5|armv7a|arm64|x86|x86_64)
                echo "patch reverse ==> $2"
                patch_disable $2
            ;;
            *)
            echo "Usage:"
            echo "  patch-debugging-with-lldb.sh armv5|armv7a|arm64|x86|x86_64"
            echo "  patch-debugging-with-lldb.sh reverse armv5|armv7a|arm64|x86|x86_64"        
            ;;
        esac
    ;;
    *)
        echo "Usage:"
        echo "  patch-debugging-with-lldb.sh armv5|armv7a|arm64|x86|x86_64"
        echo "  patch-debugging-with-lldb.sh reverse armv5|armv7a|arm64|x86|x86_64"
    ;;
esac