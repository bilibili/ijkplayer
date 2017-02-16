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

import os
import shutil

from copyrighter.CRCopyright import CRCopyright

class CRFile:
    def __init__(self, context, file):
        self.dirty           = False
        self.context         = context
        self.abs_path        = context.get_path_of_file(file)
        self.file_ext        = os.path.splitext(self.abs_path)[1][1:]
        self.copyright_names = {}
        self.copyright_urls  = {}
        self.need_insert_bilibili_copyright = True

    def update(self):
        if not self.dirty:
            self.context.log_file('~ remain', self.abs_path)
            return
        tmp_path = self.abs_path + '.tmp'
        if self.context.dryrun:
            src = open(self.abs_path, 'r')
        else:
            shutil.copy2(self.abs_path, tmp_path)
            src = open(tmp_path, 'r')
            tmp = open(self.abs_path, 'w')
        did_insert_bilibili_copyright = False
        for line in src:
            if self.need_insert_bilibili_copyright and not did_insert_bilibili_copyright:
                copyright = CRCopyright.scan_line(self.context, line)
                if copyright:
                    copyright.name = 'Bilibili'
                    copyright.url  = None
                    if not self.context.dryrun:
                        tmp.write(copyright.get_line())
                        tmp.write("\n")
                    # print '    insert %s' % copyright.get_line()
                    did_insert_bilibili_copyright = True
            if not self.context.dryrun:
                tmp.write(line)    
        src.close()
        if not self.context.dryrun:
            tmp.close()
            os.remove(tmp_path)

        if self.need_insert_bilibili_copyright and did_insert_bilibili_copyright:
            self.context.log_file('+ update', self.abs_path)
        else:
            self.context.log_file('~ missing', self.abs_path)

    def copyright_names(self):
        return self.copyright_names.keys()

    def copyright_urls(self):
        return self.copyright_urls.keys()

    def __parse_line(self, line):
        copyright = CRCopyright.scan_line(self.context, line)
        if copyright:
            # print "match %s" % copyright.name
            self.copyright_names[copyright.name.lower()] = copyright
            self.copyright_urls[copyright.url.lower()]   = copyright
        return True

    @staticmethod
    def load_from_file(context, file):
        parsed_lines = 0;
        crf          = CRFile(context = context, file = file)
        f            = open(crf.abs_path, 'r')
        for line in f:
            if parsed_lines > 20:
                break
            parsed_lines += 1
            crf.__parse_line(line)
        f.close()
        # TODO: use a visitor
        if 'bilibili' not in crf.copyright_names:
            if 'zhang rui' in crf.copyright_names or 'bbcallen@gmail.com' in crf.copyright_urls:
                crf.need_insert_bilibili_copyright = True
                crf.dirty                          = True
        return crf

    @staticmethod
    def update_path(context, file):
        base_name = os.path.basename(file)
        abs_path  = context.get_path_of_file(file)
        if   base_name.startswith('.'):
            context.log_file('- hidden', abs_path)
            return
        elif context.is_black_path(abs_path):
            context.log_file('- black', abs_path)
            return
        elif os.path.islink(abs_path):
            context.log_file('- link', abs_path)
            return
        elif os.path.isdir(abs_path):
            for sub_file in os.listdir(abs_path):
                sub_path = os.path.realpath(os.path.join(abs_path, sub_file))
                CRFile.update_path(context, sub_path)
        elif not context.need_copyright(abs_path):
            context.log_file('- nohead', abs_path)
            return
        elif os.path.isfile(abs_path):
            src_file = CRFile.load_from_file(context, abs_path)
            src_file.update()
