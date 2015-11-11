JJK = ./parser/jjk

ALL_SRC = \
	c/android/media/AudioTrack.c \
	c/android/media/MediaCodec.c \
	c/android/media/MediaFormat.c \
	c/android/os/Build.c \
	c/android/os/Bundle.c \
	c/java/nio/Buffer.c \
	c/java/nio/ByteBuffer.c \
	c/java/util/ArrayList.c \
	c/tv/danmaku/ijk/media/player/IjkMediaPlayer.c \
	c/tv/danmaku/ijk/media/player/misc/IMediaDataSource.c \

ALL_HEAD := $(ALL_SRC:.c=.h)

all: $(ALL_SRC)

c/%.c: java/%.java
	mkdir -p $(shell dirname $@)
	$(JJK) $< $(patsubst %.c,%.h,$@) $@

.PHONY: all clean clean-all rebuild
clean:
	rm -f $(ALL_SRC)
	rm -f $(ALL_HEAD)

clean-all: clean
	cd parser && make -f ./Makefile clean && cd -

rebuild: clean all
