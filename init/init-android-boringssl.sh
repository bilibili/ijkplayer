#! /usr/bin/env bash
#
# Copyright (C) 2013-2015 Bilibili
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

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BASEDIR=$(dirname "$DIR")

#IJK_OPENSSL_UPSTREAM=https://github.com/openssl/openssl
IJK_OPENSSL_UPSTREAM=https://boringssl.googlesource.com/boringssl
IJK_OPENSSL_FORK=https://boringssl.googlesource.com/boringssl
IJK_OPENSSL_COMMIT=7f02881e9 #fips-android-20191020  #tag: OpenSSL_1_0_2r
IJK_OPENSSL_LOCAL_REPO=$BASEDIR/extra/boringssl

set -e
TOOLS=$BASEDIR/tools

echo "== pull boringssl base =="
sh $TOOLS/pull-repo-base.sh $IJK_OPENSSL_UPSTREAM $IJK_OPENSSL_LOCAL_REPO

function pull_fork()
{
    echo "== pull boringssl fork =="
    sh $TOOLS/pull-repo-ref.sh $IJK_OPENSSL_FORK $BASEDIR/android/contrib/boringssl ${IJK_OPENSSL_LOCAL_REPO}
    cd $BASEDIR/android/contrib/boringssl
    git checkout ${IJK_OPENSSL_COMMIT} -B ijkplayer
    cd -
}

pull_fork
