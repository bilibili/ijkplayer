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

cd ffmpeg-armv7a

git remote set-url --push origin git@github.com:bbcallen/FFmpeg.git

git remote add ffmpeg git://source.ffmpeg.org/ffmpeg.git
git remote add libav git://git.libav.org/libav.git
git remote add chromium https://chromium.googlesource.com/chromium/third_party/ffmpeg
git remote add gitcafe git@gitcafe.com:bbcallen/FFmpeg.git
git remote add oschina git@git.oschina.net:bbcallen/ffmpeg.git
git remote add csdn git@code.csdn.net:bbcallen/FFmpeg.git
git fetch --all

git branch --track ffmpeg ffmpeg/master
git branch --track libav libav/master
git branch --track chromium chromium/master
