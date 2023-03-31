#! /usr/bin/env bash
#
# Copyright © 2022-present, Pinterest, Inc. All rights reserved.
# Copyright © 2022-present, Liang Ma <liangma@pinterest.com>
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

IOS_ROOT=`pwd`
IJKMediaPlayerHome="$IOS_ROOT/IJKMediaPlayer"
FRAMEWORK_TYPE=$1
set -e

#--------------------
echo "===================="
echo "make xcframework for ijkplayer"
BUILD_PROJECT="$IJKMediaPlayerHome/IJKMediaPlayer.xcodeproj" # build static framework by default
if [ "$FRAMEWORK_TYPE" = "dynamic" ] ; then
    BUILD_PROJECT="$IJKMediaPlayerHome/IJKMediaPlayer_build_dynamic_framework.xcodeproj"
    echo "and it is a dynamic framework...."
fi
echo "===================="

echo "clean up build artifacts before making device build"
rm -rf ./IJKMediaFramework.xcframework
rm -rf $IJKMediaPlayerHome/*.xcarchive
rm -rf ./build
rm -rf $IJKMediaPlayerHome/build

echo "===== making arm64 device framework"
sh compile-ffmpeg.sh arm64 release
xcodebuild archive -project $BUILD_PROJECT -scheme "IJKMediaFramework" -destination "generic/platform=iOS" -archivePath "./device.xcarchive" SKIP_INSTALL=NO BUILD_LIBRARY_FOR_DISTRIBUTION=YES
if [ ! -d "device.xcarchive" ]; then
    echo "device.xcarchive fails to build, please check errors"
    exit 1
fi
lipo -info device.xcarchive/Products/Library/Frameworks/IJKMediaFramework.framework/IJKMediaFramework
echo "===== making arm64 device framework done!"

echo "clean up ffmpeg build artifacts before making device build"
rm -rf ./build
rm -rf $IJKMediaPlayerHome/build

# make clean after arm64 device build
./compile-ffmpeg.sh clean

echo "===== making simulator (x86_64 and arm64)frameworks"
sh compile-ffmpeg.sh arm64 release simulator
sh compile-ffmpeg.sh x86_64 release
xcodebuild archive -project $BUILD_PROJECT -scheme "IJKMediaFramework" -destination "generic/platform=iOS Simulator" -archivePath "./simulator.xcarchive" SKIP_INSTALL=NO BUILD_LIBRARY_FOR_DISTRIBUTION=YES
if [ ! -d "simulator.xcarchive" ]; then
    echo "simulator.xcarchive fails to build, please check errors"
    exit 1
fi
lipo -info simulator.xcarchive/Products/Library/Frameworks/IJKMediaFramework.framework/IJKMediaFramework
echo "===== making simulator (x86_64 and arm64)frameworks done!"

echo "===== composing xcframework"
xcodebuild -create-xcframework -framework device.xcarchive/Products/Library/Frameworks/IJKMediaFramework.framework -framework simulator.xcarchive/Products/Library/Frameworks/IJKMediaFramework.framework -output ./IJKMediaFramework.xcframework
echo "===== composing xcframework done, at: ./IJKMediaFramework.xcframework"

echo "clean up IJKMediaPlayerHome build artifacts"
rm -rf $IJKMediaPlayerHome/build
rm -rf ./*.xcarchive

echo "git reset changes to $IJKMediaPlayerHome/build"
git checkout ./build
