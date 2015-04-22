#! /usr/bin/env bash
set -e

cd ../../android/ijkmediaplayer/
ant release
cp ./sdk/jars/* ../../SDK/android
cp -r ./sdk/libs/armeabi-v7a ../../SDK/android

cd ../../android/ijkmediawidget/
ant release
cp ./sdk/jars/* ../../SDK/android

cd ../../SDK/android

