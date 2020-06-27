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
    echo "  compile-libsrt.sh armv7a|arm64|x86|x86_64"
    echo "  compile-libsrt.sh all|all32"
    echo "  compile-libsrt.sh all64"
    echo "  compile-libsrt.sh clean"
    echo "  compile-libsrt.sh check"
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

do_build_libsrt() {
    
    echo "ARCH:$1"
    ARCH=$1
    API_LEVEL=
    FF_ARCH=
    FF_CROSS_PREFIX=

    if [ "$ARCH" = "armv7a" ]; then
        API_LEVEL=16
        FF_ARCH="armeabi-v7a"
        FF_CROSS_PREFIX=arm-linux-androideabi
    elif [ "$ARCH" = "x86" ]; then
        API_LEVEL=16
        FF_ARCH="x86"
        FF_CROSS_PREFIX=i686-linux-android
    elif [ "$ARCH" = "x86_64" ]; then
        API_LEVEL=21
        FF_ARCH="x86_64"
        FF_CROSS_PREFIX=x86_64-linux-android
    elif [ "$ARCH" = "arm64" ]; then
        API_LEVEL=21
        FF_ARCH="arm64-v8a"
        FF_CROSS_PREFIX=aarch64-linux-android
    fi

    FF_PREFIX=`pwd`/build/output-${ARCH}
    LIBSRC_SRC_PATH=`pwd`/libsrt
    echo "LIBSRC_SRC_PATH: $LIBSRC_SRC_PATH"
    echo "FF_ARCH:${FF_ARCH}"
    echo "API_LEVEL:${API_LEVEL}"
    
    mkdir -p "build/libsrt-$ARCH"
    cd build/libsrt-$ARCH
    ${ANDROID_HOME}/cmake/3.10.2.4988404/bin/cmake \
        -DANDROID_ABI=${FF_ARCH} \
        -DANDROID_NDK=${ANDROID_NDK} \
        -DCMAKE_ANDROID_API=${API_LEVEL} \
        -DANDROID_NATIVE_API_LEVEL=${API_LEVEL} \
        -DCMAKE_MAKE_PROGRAM=${ANDROID_HOME}/cmake/3.10.2.4988404/bin/ninja \
        -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${FF_PREFIX} \
        -DCMAKE_PREFIX_PATH=${FF_PREFIX} \
        -DANDROID_STL=c++_static \
        -DANDROID_TOOLCHAIN=gcc \
        -DUSE_OPENSSL_PC=OFF \
        -DOPENSSL_CRYPTO_LIBRARY=${FF_PREFIX}/lib/libcrypto.a \
        -DOPENSSL_SSL_LIBRARY=${FF_PREFIX}/lib/libssl.a \
        -DOPENSSL_INCLUDE_DIR=${FF_PREFIX}/include \
        -DENABLE_SHARED=OFF \
        -DENABLE_CXX11=OFF \
        -DENABLE_CXX_DEPS=OFF \
        -DENABLE_APPS=OFF \
        -DUSE_STATIC_LIBSTDCXX=OFF \
        -GNinja ${LIBSRC_SRC_PATH}

    cd -
    cmake --build build/libsrt-${ARCH}
    cmake --build build/libsrt-${ARCH} --target install
    sed -i 's|-lsrt   |-lsrt -lc -lm -ldl -lcrypto -lssl -lstdc++|g' ${FF_PREFIX}/lib/pkgconfig/srt.pc
    sed -i '12d;' ${FF_PREFIX}/lib/pkgconfig/srt.pc
}

#----------
case "$FF_TARGET" in
    "")
        echo_archs armv7a
        do_build_libsrt armv7a
    ;;
    armv7a|arm64|x86|x86_64)
        echo_archs $FF_TARGET
        do_build_libsrt $FF_TARGET
        echo_nextstep_help
    ;;
    all32)
        echo_archs $FF_ACT_ARCHS_32
        for ARCH in $FF_ACT_ARCHS_32
        do
            do_build_libsrt $ARCH
        done
        echo_nextstep_help
    ;;
    all|all64)
        echo_archs $FF_ACT_ARCHS_64
        for ARCH in $FF_ACT_ARCHS_64
        do
            do_build_libsrt $ARCH
        done
        echo_nextstep_help
    ;;
    clean)
        echo_archs FF_ACT_ARCHS_64
        for ARCH in $FF_ACT_ARCHS_ALL
        do
            if [ -d libsrt ]; then
                cd libsrt && git clean -xdf && cd -
            fi
            if [ -d build/libsrt-$ARCH ]; then
                rm -rf build/libsrt-$ARCH
            fi
        done
        rm -rf ./build/libsrt-*
    ;;
    check)
        echo_archs FF_ACT_ARCHS_ALL
    ;;
    *)
        echo_usage
        exit 1
    ;;
esac
