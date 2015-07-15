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

IJK_LIB_NAME=android-ndk-profiler
IJK_LIB_UPSTREAM=https://github.com/Bilibili/android-ndk-profiler.git
IJK_LIB_FORK=https://github.com/Bilibili/android-ndk-profiler.git
IJK_LIB_COMMIT=ijk-r0.3.0-dev
IJK_LIB_LOCAL_REPO=extra/android-ndk-profiler

set -e
TOOLS=tools

echo "== pull $IJK_LIB_NAME base =="
sh $TOOLS/pull-repo-base.sh $IJK_LIB_UPSTREAM $IJK_LIB_LOCAL_REPO

echo "== pull $IJK_LIB_NAME fork =="
sh $TOOLS/pull-repo-ref.sh $IJK_LIB_FORK ijkprof/$IJK_LIB_NAME ${IJK_LIB_LOCAL_REPO}
cd ijkprof/$IJK_LIB_NAME
git checkout ${IJK_LIB_COMMIT}
cd -
