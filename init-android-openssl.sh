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

#IJK_OPENSSL_UPSTREAM=https://github.com/openssl/openssl
IJK_OPENSSL_UPSTREAM=https://github.com/Bilibili/openssl.git
IJK_OPENSSL_FORK=https://github.com/Bilibili/openssl.git
IJK_OPENSSL_COMMIT=OpenSSL_1_0_2d
IJK_OPENSSL_LOCAL_REPO=extra/openssl

set -e
TOOLS=tools

echo "== pull openssl base =="
sh $TOOLS/pull-repo-base.sh $IJK_OPENSSL_UPSTREAM $IJK_OPENSSL_LOCAL_REPO

function pull_fork()
{
    echo "== pull openssl fork $1 =="
    sh $TOOLS/pull-repo-ref.sh $IJK_OPENSSL_FORK android/contrib/openssl-$1 ${IJK_OPENSSL_LOCAL_REPO}
    cd android/contrib/openssl-$1
    git checkout ${IJK_OPENSSL_COMMIT}
    cd -
}

pull_fork "armv7a"
pull_fork "armv5"
pull_fork "x86"
