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

# This script is based on projects below
# https://github.com/yixia/FFmpeg-Android
# http://git.videolan.org/?p=vlc-ports/android.git;a=summary

#--------------------
set -e

UNAME_S=$(uname -s)
UNAME_SM=$(uname -sm)
echo "build on $UNAME_SM"

echo "ANDROID_NDK=$ANDROID_NDK"

if [ -z "$ANDROID_NDK" ]; then
    echo "You must define ANDROID_NDK before starting."
    echo "They must point to your NDK directories."
    echo ""
    exit 1
fi



# try to detect NDK version
export IJK_GCC_VER=4.9
export IJK_GCC_64_VER=4.9
export IJK_MAKE_TOOLCHAIN_FLAGS=
export IJK_MAKE_FLAG=
export IJK_NDK_REL=$(grep -o '^r[0-9]*.*' $ANDROID_NDK/RELEASE.TXT 2>/dev/null | sed 's/[[:space:]]*//g' | cut -b2-)
case "$IJK_NDK_REL" in
    10e*)
        # we don't use 4.4.3 because it doesn't handle threads correctly.
        if test -d ${ANDROID_NDK}/toolchains/arm-linux-androideabi-4.8
        # if gcc 4.8 is present, it's there for all the archs (x86, mips, arm)
        then
            echo "NDKr$IJK_NDK_REL detected"

            case "$UNAME_S" in
                Darwin)
                    export IJK_MAKE_TOOLCHAIN_FLAGS="$IJK_MAKE_TOOLCHAIN_FLAGS --system=darwin-x86_64"
                ;;
                CYGWIN_NT-*)
                    export IJK_MAKE_TOOLCHAIN_FLAGS="$IJK_MAKE_TOOLCHAIN_FLAGS --system=windows-x86_64"
                ;;
            esac
        else
            echo "You need the NDKr10e or later"
            exit 1
        fi
    ;;
    *)
        IJK_NDK_REL=$(grep -o '^Pkg\.Revision.*=[0-9]*.*' $ANDROID_NDK/source.properties 2>/dev/null | sed 's/[[:space:]]*//g' | cut -d "=" -f 2)
        echo "IJK_NDK_REL=$IJK_NDK_REL"
        case "$IJK_NDK_REL" in
            11*|12*)
                if test -d ${ANDROID_NDK}/toolchains/arm-linux-androideabi-4.9
                then
                    echo "NDKr$IJK_NDK_REL detected"
                else
                    echo "You need the NDKr10e or later"
                    exit 1
                fi
            ;;
            *)
                echo "You need the NDKr10e or later"
                exit 1
            ;;
        esac
    ;;
esac


case "$UNAME_S" in
    Darwin)
        export IJK_MAKE_FLAG=-j`sysctl -n machdep.cpu.thread_count`
    ;;
    CYGWIN_NT-*)
        IJK_WIN_TEMP="$(cygpath -am /tmp)"
        export TEMPDIR=$IJK_WIN_TEMP/

        echo "Cygwin temp prefix=$IJK_WIN_TEMP/"
    ;;
esac
