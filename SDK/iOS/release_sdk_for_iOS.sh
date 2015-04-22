#! /usr/bin/env bash
set -e

xcrun lipo -create ./lib/arm/libIJKMediaPlayer.a ./lib/x86/libIJKMediaPlayer.a ./lib/x86_64/libIJKMediaPlayer.a -output ./lib/libIJKMediaPlayer.a 



