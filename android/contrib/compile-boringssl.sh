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

#----------
UNI_BUILD_ROOT=`pwd`
FF_TARGET=$1
set -e
set +x

FF_ACT_ARCHS_32="armv7a x86"
FF_ACT_ARCHS_64="armv7a arm64 x86 x86_64"
FF_ACT_ARCHS_ALL=$FF_ACT_ARCHS_64

. ./tools/do-detect-env.sh

echo_archs() {
    echo "===================="
    echo "[*] check archs"
    echo "===================="
    echo "FF_ALL_ARCHS = $FF_ACT_ARCHS_ALL"
    echo "FF_ACT_ARCHS = $*"
    echo ""
}

echo_usage() {
    echo "Usage:"
    echo "  compile-boringssl.sh armv7a|arm64|x86|x86_64"
    echo "  compile-boringssl.sh all|all32"
    echo "  compile-boringssl.sh all64"
    echo "  compile-boringssl.sh clean"
    echo "  compile-boringssl.sh check"
    exit 1
}

echo_nextstep_help() {
    #----------
    echo ""
    echo "--------------------"
    echo "[*] Finished"
    echo "--------------------"
    echo "# to continue to build ffmpeg, run script below,"
    echo "sh compile-ffmpeg.sh "
    echo "# to continue to build ijkplayer, run script below,"
    echo "sh compile-ijk.sh "
}

do_build_boringssl() {
    
    echo "ARCH:$1"
    ARCH=$1
    API_LEVEL=
    FF_ARCH=
    if [ "$ARCH" = "armv7a" ]; then
        API_LEVEL=16
        FF_ARCH="armeabi-v7a"
    elif [ "$ARCH" = "x86" ]; then
        API_LEVEL=16
        FF_ARCH="x86"
    elif [ "$ARCH" = "x86_64" ]; then
        API_LEVEL=21
        FF_ARCH="x86_64"
    elif [ "$ARCH" = "arm64" ]; then
        API_LEVEL=21
        FF_ARCH="arm64-v8a"
    fi

    BORINGSSL_SRC_PATH=`pwd`/boringssl
    echo "BORINGSSL_SRC_PATH: $BORINGSSL_SRC_PATH"
    echo "FF_ARCH:${FF_ARCH}"
    echo "API_LEVEL:${API_LEVEL}"
    
    mkdir -p "build/boringssl-$ARCH"
    cmake \
        -Bbuild/boringssl-$ARCH \
        -DANDROID_ABI=${FF_ARCH} \
        -DANDROID_NDK=${ANDROID_NDK} \
        -DANDROID_NATIVE_API_LEVEL=${API_LEVEL} \
        -DCMAKE_MAKE_PROGRAM=ninja \
        -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DANDROID_STL=c++_static \
        -DANDROID_TOOLCHAIN=clang \
        -GNinja ${BORINGSSL_SRC_PATH}

    cmake --build build/boringssl-${ARCH}

    mkdir -p build/output-${ARCH}/lib
    cp build/boringssl-${ARCH}/ssl/libssl.a build/output-${ARCH}/lib
    cp build/boringssl-${ARCH}/crypto/libcrypto.a build/output-${ARCH}/lib

    mkdir -p build/output-${ARCH}/include
    cp -r boringssl/include/* build/output-${ARCH}/include
}

#----------
case "$FF_TARGET" in
    "")
        echo_archs armeabi-v7a
        do_build_boringssl armeabi-v7a
    ;;
    armv7a|arm64|x86|x86_64)
        echo_archs $FF_TARGET
        do_build_boringssl $FF_TARGET
        echo_nextstep_help
    ;;
    all32)
        echo_archs $FF_ACT_ARCHS_32
        for ARCH in $FF_ACT_ARCHS_32
        do
            do_build_boringssl $ARCH
        done
        echo_nextstep_help
    ;;
    all|all64)
        echo_archs $FF_ACT_ARCHS_64
        for ARCH in $FF_ACT_ARCHS_64
        do
            do_build_boringssl $ARCH
        done
        echo_nextstep_help
    ;;
    clean)
        echo_archs FF_ACT_ARCHS_64
        for ARCH in $FF_ACT_ARCHS_ALL
        do
            if [ -d boringssl ]; then
                cd boringssl && git clean -xdf && cd -
            fi
            if [ -d build/boringssl-$ARCH ]; then
                rm -rf build/boringssl-$ARCH
            fi
        done
        rm -rf ./build/boringssl-*
    ;;
    check)
        echo_archs FF_ACT_ARCHS_ALL
    ;;
    *)
        echo_usage
        exit 1
    ;;
esac
