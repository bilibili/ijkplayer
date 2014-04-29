#! /usr/bin/env bash

adb logcat | ndk-stack -sym ijkmediaplayer/obj/local/armeabi-v7a
