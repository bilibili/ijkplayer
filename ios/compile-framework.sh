#!/bin/sh

set -xe

CURRENTPATH=`pwd`
cd "$CURRENTPATH/IJKMediaPlayer"

BUILD_CONFIGURATION="Release"

BUILD_SDK_VERSION=$(xcodebuild -showsdks | grep iphoneos | sort -r | head -n 1 | grep -o '.\{4\}$')
if [[ $BUILD_SDK_VERSION = "" ]]; then
echo "Error: No iphone sdk ..."
exit 1
fi

BUILD_SDK_IPHONEOS=iphoneos$BUILD_SDK_VERSION
BUILD_SDK_IPHONESIMULATOR=iphonesimulator$BUILD_SDK_VERSION

xcodebuild -project IJKMediaPlayer.xcodeproj -target IJKMediaFramework -configuration $BUILD_CONFIGURATION -sdk $BUILD_SDK_IPHONEOS
xcodebuild -project IJKMediaPlayer.xcodeproj -target IJKMediaFramework -configuration $BUILD_CONFIGURATION -sdk $BUILD_SDK_IPHONESIMULATOR

# 
cd build
cp -r $BUILD_CONFIGURATION-iphoneos/IJKMediaFramework.framework .
lipo -create $BUILD_CONFIGURATION-iphoneos/IJKMediaFramework.framework/IJKMediaFramework $BUILD_CONFIGURATION-iphonesimulator/IJKMediaFramework.framework/IJKMediaFramework -output IJKMediaFramework
mv IJKMediaFramework IJKMediaFramework.framework/IJKMediaFramework