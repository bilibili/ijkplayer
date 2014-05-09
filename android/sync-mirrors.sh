#! /usr/bin/env bash

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

git push origin --all --follow-tags
git push gitcafe --all --follow-tags
git push oschina --all --follow-tags
git push csdn --all --follow-tags

cd -
