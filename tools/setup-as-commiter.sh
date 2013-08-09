#! /bin/sh

cd ffmpeg-armv7a

git remote add upstream -f -t master git://source.ffmpeg.org/ffmpeg.git
git fetch upstream
git branch -f master upstream/master

git remote add mirrors --mirror=push git@github.com:bbcallen/FFmpeg.git
git remote set-url mirrors --delete git@github.com:bbcallen/FFmpeg.git
git remote set-url mirrors --add    git@github.com:bbcallen/FFmpeg.git
git remote set-url mirrors --delete git@gitcafe.com:bbcallen/FFmpeg.git
git remote set-url mirrors --add    git@gitcafe.com:bbcallen/FFmpeg.git
git remote set-url mirrors --delete git@git.oschina.net:bbcallen/ffmpeg.git
git remote set-url mirrors --add    git@git.oschina.net:bbcallen/ffmpeg.git
git remote set-url mirrors --delete git@code.csdn.net:bbcallen/FFmpeg.git
git remote set-url mirrors --add    git@code.csdn.net:bbcallen/FFmpeg.git

