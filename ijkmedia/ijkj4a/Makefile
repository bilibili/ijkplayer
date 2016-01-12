J4A = ../../extra/bin/j4a
ROOT_CLASS_INCLUDES = j4a/j4a_allclasses.include.h
ROOT_CLASS_LOADERS = j4a/j4a_allclasses.loader.h

all: ijkj4a

# test java -> c
JAVA_SRCS = \
	java/java/nio/Buffer.java \
	java/java/nio/ByteBuffer.java \
	java/java/util/ArrayList.java \
	java/android/media/AudioTrack.java \
	java/android/media/MediaCodec.java \
	java/android/media/MediaFormat.java \
	java/android/media/PlaybackParams.java \
	java/android/os/Build.java \
	java/android/os/Bundle.java \
	java/tv/danmaku/ijk/media/player/misc/IMediaDataSource.java \
	java/tv/danmaku/ijk/media/player/IjkMediaPlayer.java \

C_SRCS := $(JAVA_SRCS:java/%.java=j4a/class/%.c)
H_SRCS := $(C_SRCS:%.c=%.h)

$(C_SRCS): j4a/class/%.c: $(J4A)

$(C_SRCS): j4a/class/%.c: java/%.java
ifneq ("$<", "j4a/class/.c")
	@mkdir -p $(shell dirname $@)
	$(J4A) -c $< -o $@
	@cat j4a/class/$*.include.j4a >> $(ROOT_CLASS_INCLUDES)
	@echo >> $(ROOT_CLASS_INCLUDES)
	@cat j4a/class/$*.loader.j4a >> $(ROOT_CLASS_LOADERS)
	@echo >> $(ROOT_CLASS_LOADERS)
endif

ijkj4a: prebuild $(C_SRCS)

prebuild:
	@rm -f $(ROOT_CLASS_INCLUDES)
	@rm -f $(ROOT_CLASS_LOADERS)

# -----
.PHONY: all clean ijkj4a prebuild

clean:
	@rm -f $(C_SRCS)
	@rm -f $(H_SRCS)
	@rm -f $(ROOT_CLASS_INCLUDES)
	@rm -f $(ROOT_CLASS_LOADERS)
