#! /usr/bin/env bash

#--------------------
# Standard options:
export COMMON_FF_CFG_FLAGS=
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --prefix=PREFIX"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-logging"

# Licensing options:
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-gpl"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-version3"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-nonfree"

# Configuration options:
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-static"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-shared"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-small"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-runtime-cpudetect"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-gray"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-swscale-alpha"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-all"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-incompatible-libav-abi"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-raise-majors"

# Program options:
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-programs"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-ffmpeg"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-ffplay"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-ffprobe"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-ffserver"

# Documentation options:
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-doc"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-htmlpages"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-manpages"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-podpages"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-txtpages"

# Component options:
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-avdevice"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-avcodec"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-avformat"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-avutil"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-swresample"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-swscale"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-postproc"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-avfilter"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-avresample"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-pthreads"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-w32threads"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-os2threads"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-network"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-dct"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-dwt"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-error-resilience"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-lsp"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-lzo"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-mdct"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-rdft"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-fft"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-faan"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-pixelutils"

# Hardware accelerators:
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-dxva2"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-vaapi"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-vda"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-vdpau"

# Individual component options:
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-everything"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-encoders"

# ./configure --list-decoders
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-decoders"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-decoder=aac"
# export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-decoder=aac_latm"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-decoder=h264"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-decoder=hevc"


export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-hwaccels"

# ./configure --list-muxers
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-muxers"

# ./configure --list-demuxers
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-demuxers"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-demuxer=flv"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-demuxer=live_flv"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-demuxer=mpegvideo"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-demuxer=mpegps"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-demuxer=mpegts"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-demuxer=hls"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-demuxer=mov"

# ./configure --list-parsers
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-parsers"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-parser=aac"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-parser=h264"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-parser=mpegvideo"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-parser=mpeg4video"

# ./configure --list-bsfs
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-bsfs"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-bsf=aac_adtstoasc"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-bsf=h264_mp4toannexb"

# ./configure --list-protocols
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-protocols"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-protocol=http"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-protocol=https"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-protocol=hls"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-protocol=rtmp"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --enable-protocol=file"

#
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-devices"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-filters"

# External library support:
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-iconv"
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-sdl"

# ...

# Advanced options (experts only):
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-symver"

# Optimization options (experts only):

# Developer options (useful when working on FFmpeg itself):
export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-debug"
