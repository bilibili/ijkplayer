#! /usr/bin/env bash
#
# Copyright (C) 2020-present befovy <befovy@gmail.com>
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

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BASEDIR=$(dirname "$DIR")

IJK_LIBSRT_UPSTREAM=https://github.com/Haivision/srt.git
IJK_LIBSRT_FORK=https://github.com/Haivision/srt.git
IJK_LIBSRT_COMMIT=7bf96c716d1ab8e75422b9cb7118fc82f497a5b3
IJK_LIBSRT_LOCAL_REPO=$BASEDIR/extra/libsrt

set -e
TOOLS=$BASEDIR/tools

echo "== pull libsrt base =="
sh $TOOLS/pull-repo-base.sh $IJK_LIBSRT_UPSTREAM $IJK_LIBSRT_LOCAL_REPO

function pull_fork()
{
    echo "== pull libsrt fork =="
    sh $TOOLS/pull-repo-ref.sh $IJK_LIBSRT_FORK $BASEDIR/android/contrib/libsrt ${IJK_LIBSRT_LOCAL_REPO}
    cd $BASEDIR/android/contrib/libsrt
    git checkout ${IJK_LIBSRT_COMMIT} -B ijkplayer
    cd -
}

# pull_fork "armv5"
# pull_fork "armv7a"
# pull_fork "arm64"
# pull_fork "x86"
# pull_fork "x86_64"
pull_fork