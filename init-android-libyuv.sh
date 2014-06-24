#! /usr/bin/env bash

IJK_LIBYUV_UPSTREAM=https://github.com/bbcallen/libyuv.git
IJK_LIBYUV_FORK=https://github.com/bbcallen/libyuv.git
IJK_LIBYUV_COMMIT=master
IJK_LIBYUV_LOCAL_REPO=extra/libyuv

set -e
TOOLS=tools

echo "== pull libyuv base =="
sh $TOOLS/pull-repo-base.sh $IJK_LIBYUV_UPSTREAM $IJK_LIBYUV_LOCAL_REPO

echo "== pull libyuv fork =="
sh $TOOLS/pull-repo-ref.sh $IJK_LIBYUV_FORK ijkmedia/ijkyuv ${IJK_LIBYUV_LOCAL_REPO}
cd ijkmedia/ijkyuv
git checkout ${IJK_LIBYUV_COMMIT}
cd -
