#! /usr/bin/env bash
#
# Copyright (C) 2016 Zhang Rui <bbcallen@gmail.com>
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

IJK_J4A_UPSTREAM=https://github.com/Bilibili/jni4android.git
IJK_J4A_FORK=https://github.com/Bilibili/jni4android.git
IJK_J4A_COMMIT=v0.0.1
IJK_J4A_LOCAL_REPO=extra/jni4android

set -e
TOOLS=tools

echo "== pull j4a base =="
sh $TOOLS/pull-repo-base.sh $IJK_J4A_UPSTREAM $IJK_J4A_LOCAL_REPO

echo "== pull j4a fork =="
sh $TOOLS/pull-repo-ref.sh $IJK_J4A_FORK extra/jni4android-fork ${IJK_J4A_LOCAL_REPO}
cd extra/jni4android-fork
git checkout ${IJK_J4A_COMMIT}
cd -
