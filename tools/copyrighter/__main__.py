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

import sys
import os
import fnmatch
from sets import Set

if not __package__:
    path = os.path.join(os.path.dirname(__file__), os.pardir)
    sys.path.insert(0, path)

from copyrighter.CRContext import CRContext
from copyrighter.CRFile import CRFile

context = CRContext();

target_list = Set([
    'ijkmedia',
    'extra/init-extra.sh'
    ])

for target in Set(['']):
# for target in target_list:
    CRFile.update_path(context, target)
