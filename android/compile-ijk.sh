#! /usr/bin/env bash

cd ijkmediaplayer/jni
ndk-build
cd -

cd ijkmediaplayer-armv7a/jni
ndk-build
cd -

cd ijkmediaplayer-armv5/jni
ndk-build
cd -

cd ijkmediaplayer-x86/jni
ndk-build
cd -
