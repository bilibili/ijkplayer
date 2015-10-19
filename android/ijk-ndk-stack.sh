#!/bin/sh

adb logcat | ndk-stack -sym ijkplayer/ijkplayer-armv7a/src/main/obj/local/armeabi-v7a
