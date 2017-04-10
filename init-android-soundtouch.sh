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


IJK_SOUNDTOUCH_UPSTREAM=https://github.com/Bilibili/soundtouch.git
IJK_SOUNDTOUCH_FORK=https://github.com/Bilibili/soundtouch.git
IJK_SOUNDTOUCH_COMMIT=ijk-r0.1.2-dev
IJK_SOUNDTOUCH_LOCAL_REPO=extra/soundtouch

set -e
TOOLS=tools

echo "== pull soundtouch base =="
sh $TOOLS/pull-repo-base.sh $IJK_SOUNDTOUCH_UPSTREAM $IJK_SOUNDTOUCH_LOCAL_REPO

echo "== pull soundtouch fork =="
sh $TOOLS/pull-repo-ref.sh $IJK_SOUNDTOUCH_FORK ijkmedia/ijksoundtouch ${IJK_SOUNDTOUCH_LOCAL_REPO}
cd ijkmedia/ijksoundtouch
git checkout ${IJK_SOUNDTOUCH_COMMIT}
cd -
