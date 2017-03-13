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

import re

class CRCopyright:
    regexp = None

    def __init__(self, name, url = None, prefix = None, symbol = None, years = None):
        self.name   = name
        self.url    = url
        self.prefix = prefix
        self.symbol = symbol
        self.years  = years
        assert prefix
        assert symbol
        assert years
        assert name

    def duplicate(self):
        return CRCopyright(
            name   = self.name,
            url    = self.url,
            prefix = self.prefix,
            symbol = self.symbol,
            years  = self.years,
            )

    def get_line(self):
        line = '%sCopyright %s %s %s' % (self.prefix, self.symbol, self.years, self.name)
        if self.url:
            line += ' <%s>' % self.url
        return line

    def match_name(self, name):
        if not self.name or not name:
            return False
        return self.name.lower() == name.lower()

    def match_name(self, url):
        if not self.url or not url:
            return False
        return self.url.lower() == url.lower()

    @staticmethod
    def scan_line(context, line):
        if line[-1] == '\n':
            line = line[0:-1]
        if not CRCopyright.regexp:
            # CRCopyright.regexp = re.compile("^([#\*\/\s]+)Copyright(?:\s+)([\(\)\w]+)(?:\s+)([\d]+|[\d]+-[\d]+)(?:\s+)([\w\s]+)[$|(<[\w\.@]+>)$]")
            # 1 prefix
            # 3 symbol
            # 5 year
            # 6 name
            # 7 url
            CRCopyright.regexp = re.compile(
                pattern = "^([#\*\/\s]+)Copyright(\s+)([\(\)\w]+)(\s+)([0-9\-]+)([^<]+)(.*)$",
                flags = re.IGNORECASE,
                )
        m = CRCopyright.regexp.match(line)
        if m:
            prefix = m.group(1)
            symbol = m.group(3).strip()
            years  = m.group(5).strip()
            name   = m.group(6).strip()
            url    = m.group(7).strip().strip('<>')
            if prefix and symbol and years and name:
                cr = CRCopyright(
                    prefix = prefix,
                    symbol = symbol,
                    years  = years,
                    name   = name,
                    url    = url,
                    )
                return cr
            # print '>====='
            # for i in range(1, 8):
            #     print '> %d "%s"' % (i, match.group(i))
            # print '>====='
        return None
