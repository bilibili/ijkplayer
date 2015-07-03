#! /usr/bin/env bash
set -e

cd ../../android/ijkmediaplayer/
ant release
cp ./sdk/jars/* ../../SDK/android
cp -r ./sdk/libs/armeabi-v7a ../../SDK/android

cd ../../android/ijkmediawidget/
ant release
cp ./sdk/jars/* ../../SDK/android

#cd ../../android/ijkmediaplayer-x86/
#cp -r ./libs/x86 ../../SDK/android

#cd ../../android/ijkmediaplayer-armv5/
#cp -r ./libs/armeabi ../../SDK/android

cd ../../android/ijkmediaplayer-arm64-v8a/
cp -r ./libs/arm64-v8a ../../SDK/android

cd ../../SDK/android

