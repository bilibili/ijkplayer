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

IJK_LIBSOXR_UPSTREAM=http://git.code.sf.net/p/soxr/code
IJK_LIBSOXR_FORK=http://git.code.sf.net/p/soxr/code
IJK_LIBSOXR_COMMIT=0.1.2
IJK_LIBSOXR_COMMIT_64=master
IJK_LIBSOXR_LOCAL_REPO=extra/soxr

set -e
TOOLS=tools

echo "== pull soxr base =="
sh $TOOLS/pull-repo-base.sh $IJK_LIBSOXR_UPSTREAM $IJK_LIBSOXR_LOCAL_REPO

function pull_fork()
{
    echo "== pull soxr fork $1 =="
    sh $TOOLS/pull-repo-ref.sh $IJK_LIBSOXR_FORK android/contrib/libsoxr-$1 ${IJK_LIBSOXR_LOCAL_REPO}
    cp extra/android-cmake/android.toolchain.cmake android/contrib/libsoxr-$1
    cd android/contrib/libsoxr-$1
    case "$1" in
        arm64|x86_64)
            git checkout ${IJK_LIBSOXR_COMMIT_64} -B ijkplayer
            ;;
        *)
            git checkout ${IJK_LIBSOXR_COMMIT} -B ijkplayer
            ;;
    esac
    cd -
}

function pull_android_toolchain_cmake()
{
    ANDROID_TOOLCHAIN_CMAKE_UPSTREAM=https://github.com/taka-no-me/android-cmake.git
    echo "== pull android toolchain cmake from $ANDROID_TOOLCHAIN_CMAKE_UPSTREAM =="
    sh $TOOLS/pull-repo-base.sh $ANDROID_TOOLCHAIN_CMAKE_UPSTREAM extra/android-cmake
}

pull_android_toolchain_cmake

pull_fork "armv5"
pull_fork "armv7a"
pull_fork "arm64"
pull_fork "x86"
pull_fork "x86_64"
