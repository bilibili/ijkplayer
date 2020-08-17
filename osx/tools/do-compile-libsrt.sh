#! /usr/bin/env bash
#
# Copyright (C) 2020-present befovy <befovy@gmail.com>
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

#--------------------
set -e

#--------------------
# common defines
FF_ARCH=$1
if [ -z "$FF_ARCH" ]; then
    echo "You must specific an architecture 'x86-64, ...'.\n"
    exit 1
fi


FF_BUILD_ROOT=`pwd`


FF_BUILD_NAME=
FF_SOURCE=
FF_CROSS_PREFIX=

FF_CFG_FLAGS=
FF_PLATFORM_CFG_FLAGS=

FF_EXTRA_CFLAGS=
FF_EXTRA_LDFLAGS=


export PKG_CONFIG_PATH="${FF_BUILD_ROOT}/contrib/build/lib/pkgconfig"
echo "PKG_CONFIG_PATH ${PKG_CONFIG_PATH}"

if [ "$FF_ARCH" = "x86_64" ]; then

    FF_BUILD_NAME=libsrt-x86_64
    FF_SOURCE=$FF_BUILD_ROOT/contrib/$FF_BUILD_NAME

else
    echo "unknown architecture $FF_ARCH";
    exit 1
fi


FF_PREFIX=$FF_BUILD_ROOT/contrib/build/

mkdir -p $FF_PREFIX
# mkdir -p $FF_SYSROOT



#--------------------
echo ""
echo "--------------------"
echo "[*] check libsrt env"
echo "--------------------"
export PATH=$FF_TOOLCHAIN_PATH/bin:$PATH

export COMMON_FF_CFG_FLAGS=


FF_CFG_FLAGS="$FF_CFG_FLAGS $COMMON_FF_CFG_FLAGS"


FF_CFG_FLAGS="$FF_CFG_FLAGS --cmake-prefix-path=${FF_PREFIX}"
FF_CFG_FLAGS="$FF_CFG_FLAGS --cmake-install-prefix=${FF_PREFIX}"
FF_CFG_FLAGS="$FF_CFG_FLAGS --use-openssl-pc=on"

FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-shared=off --enable-c++11=off"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-static=on"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-apps=on"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-c++-deps=on"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-cxx11=on"

#--------------------
echo ""
echo "--------------------"
echo "[*] configurate libsrt"
echo "--------------------"
cd $FF_SOURCE

git clean -fx

#if [ -f "./Makefile" ]; then
#    echo 'reuse configure'
#else
    echo "./configure $FF_CFG_FLAGS"
    ./configure $FF_CFG_FLAGS
#        --extra-cflags="$FF_CFLAGS $FF_EXTRA_CFLAGS" \
#        --extra-ldflags="$FF_EXTRA_LDFLAGS"
#fi

#--------------------
echo ""
echo "--------------------"
echo "[*] compile libsrt"
echo "--------------------"
make depend
make $FF_MAKE_FLAGS
make install
