#! /bin/sh

cd ffmpeg-armv7a

git checkout ffmpeg
git pull
git checkout -

git checkout libav
git pull
git checkout -

git checkout master
git pull
git merge ffmpeg
git checkout -

git push origin --all
git push gitcafe --all
git push oschina --all
git push csdn --all

cd -
