#! /bin/sh

cd ffmpeg-armv7a

git remote set-url --push origin git@github.com:bbcallen/FFmpeg.git

git remote add ffmpeg git://source.ffmpeg.org/ffmpeg.git
git remote add libav git://git.libav.org/libav.git
git remote add gitcafe git@gitcafe.com:bbcallen/FFmpeg.git
git remote add oschina git@git.oschina.net:bbcallen/ffmpeg.git
git remote add csdn git@code.csdn.net:bbcallen/FFmpeg.git
git fetch --all

git branch --track ffmpeg ffmpeg/master
git branch --track libav libav/master
