#! /bin/sh

adb logcat | ndk-stack -sym ijkplayer/ijkmediaplayer/obj/local/armeabi-v7a
