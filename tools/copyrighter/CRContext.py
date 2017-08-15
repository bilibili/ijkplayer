#! /usr/bin/env bash
#
# Copyright (C) 2013-2017 Bilibili
# Copyright (C) 2013-2017 Zhang Rui <bbcallen@gmail.com>
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

import datetime
import os
import fnmatch
from sets import Set

from copyrighter.CRCopyright import CRCopyright

class CRContext:
    file_ext_white_list = Set([
        # '*',
        'c',
        'h',
        'java',
        'm',
        'mk',
        'py',
        'sh',
        ])

    rel_path_black_list = Set([
        '*.a',
        '*.d',
        '*.jar',
        '*.lib',
        '*.lproj',
        '*.o',
        '*.patch',
        '*.pbxproj',
        '*.pch',
        '*.png',
        '*.pyc',
        '*.so',
        '*.strings',
        '*.xcassets',
        '*.xcodeproj',
        '*.xcuserdatad',
        '*.xcworkspace',
        '*.xib',

        'android/contrib',
        'android/ijkplayer/build',
        'android/ijkplayer/*/build',
        'android/ijkplayer/*/libs',
        'android/ijkplayer/*/obj',
        'android/ijkplayer/*/res',
        'android/patches',
        'extra',
        'ijkmedia/ijkyuv',
        'ijkmedia/ijkj4a',
        'ijkprof/android-ndk-profiler',
        'ios/build',
        'ios/contrib',
        'ios/ffmpeg-*',
        'ios/openssl-*',
        ])

    def __init__(self, verbose = True, dryrun = True):
        self.verbose    = verbose
        self.dryrun     = dryrun
        self.root_path  = os.path.realpath(os.path.join(__file__, '../../..'))
        self.year       = datetime.date.today().year

    def get_path_of_file(self, file):
        if not os.path.isabs(file):
            file = os.path.realpath(os.path.join(self.root_path, file))
        return file

    def get_relpath(self, file):
        return os.path.relpath(file, self.root_path)

    def is_black_path(self, path):
        rel_path = self.get_relpath(path)
        for black_item in CRContext.rel_path_black_list:
            if fnmatch.fnmatch(rel_path, black_item):
                # print '  + fnmatch %s %s' % (rel_path, black_item)
                return True
            # else:
                # print '  - fnmatch %s %s' % (rel_path, black_item)
        return False

    def need_copyright(self, file):
        if '*' in CRContext.file_ext_white_list:
            return True
        ext = os.path.splitext(file)[1][1:]
        return ext in CRContext.file_ext_white_list

    def log_file(self, tag, file):
        if self.verbose:
            rel_path = self.get_relpath(file)
            print '%-10s %s' % (tag, rel_path)
