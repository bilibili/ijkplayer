set -e

cd parser
make
cd -

make rebuild
#parser/jjk java/android/media/MediaCodec.java

