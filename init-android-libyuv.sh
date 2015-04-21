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


IJK_LIBYUV_UPSTREAM=https://github.com/Bilibili/libyuv.git
IJK_LIBYUV_FORK=https://github.com/Bilibili/libyuv.git
IJK_LIBYUV_COMMIT=ijk-r0.2.1-dev
IJK_LIBYUV_LOCAL_REPO=extra/libyuv

set -e
TOOLS=tools

echo "== pull libyuv base =="
sh $TOOLS/pull-repo-base.sh $IJK_LIBYUV_UPSTREAM $IJK_LIBYUV_LOCAL_REPO

echo "== pull libyuv fork =="
sh $TOOLS/pull-repo-ref.sh $IJK_LIBYUV_FORK ijkmedia/ijkyuv ${IJK_LIBYUV_LOCAL_REPO}
cd ijkmedia/ijkyuv
git checkout ${IJK_LIBYUV_COMMIT}
cd -
