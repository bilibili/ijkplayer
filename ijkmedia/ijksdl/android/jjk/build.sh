set -e

cd parser
make
cd -

make rebuild
#parser/jjk java/android/media/MediaCodec.java
#parser/jjk java/tv/danmaku/ijk/media/player/misc/IMediaDataSource.java

