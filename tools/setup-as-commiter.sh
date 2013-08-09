#! /bin/sh

cd ffmpeg-armv7a

git remote set-url --push origin git@github.com:bbcallen/FFmpeg.git

git remote add ffmpeg git://source.ffmpeg.org/ffmpeg.git
git remote add libav git://git.libav.org/libav.git
git fetch --all

git branch --track ffmpeg ffmpeg/master
git branch --track libav libav/master

