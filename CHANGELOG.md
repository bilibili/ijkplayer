
# Changelog

All notable changes to this project will be documented in this file. See [standard-version](https://github.com/conventional-changelog/standard-version) for commit guidelines.

---

## 0.7.8 (2020-07-11)

* fix, flush audio before toggle eof

---
## 0.7.6 (2020-07-03)

* fix: update position with duration value when playback complete ([f3d27d8](https://github.com/befovy/ijkplayer/commit/f3d27d832ba0a3472bb59c129c2c65591f8855d4))
* fix: use cmake configure generate ijkversion.h ([d252733](https://github.com/befovy/ijkplayer/commit/d252733c36cc05a2fc22ea23c99c04b4872b9f72))
* fix: android vout memory leak ([39631b0](https://github.com/befovy/ijkplayer/commit/39631b0344e5d9576d01d3498a0786362222cec2))
* upgrade android build ndk to ndk16 (#39 #42 #43)

## 0.7.4 (2020-05-10)

* fix **android:** video render failed after call reset ([ef323bb](https://github.com/befovy/ijkplayer/commit/ef323bb5f3e661a11125a0a4cc9c4c2ced6d04c0))
* add loglevel for java code ([fb1e278](https://github.com/befovy/ijkplayer/commit/fb1e2781a273327c2b32b741d100279335f7bf1a))

---
## 0.7.3 (2020-05-09)

* support custom "pos-update-interval", min 10 ms, max 1000 ms ([ed97462](https://github.com/befovy/ijkplayer/commit/ed97462deece795f956f1c5feae0db815df2938d))

---
## 0.7.2 (2020-05-05)

* fix rv32 colorspace convert error ([3d50112](https://github.com/befovy/ijkplayer/commit/3d50112568df078e3015569810f8b11cf85a31f1))

---
## 0.7.1 (2020-05-04)

* fix ios crash, use cache pixelbuffer ([48e46ce](https://github.com/befovy/ijkplayer/commit/48e46ced57560f5b749e348f716b3ad42dfc8e05))

---
## 0.7.0 (2020-05-03)

* add audioSessionInterrupt handler in IJKFFMediaPlayer ([273f36a](https://github.com/befovy/ijkplayer/commit/273f36a435f187944be7fd96ca181c9b0a420868))
* add gles ose render for android mediacodec buffers ([b1f924f](https://github.com/befovy/ijkplayer/commit/b1f924fd0aa4310e9383f6cafbd21375e0c9f37c))
* add ijkplayer ios error code ([a79c754](https://github.com/befovy/ijkplayer/commit/a79c7546e590d3b5746396d7729509240723beba))
* add log level filter for ijklog ([16bb3ed](https://github.com/befovy/ijkplayer/commit/16bb3edb99c1d3391f4d4d168a4c7158b411e9f9))
* add snapshot support ([7c64747](https://github.com/befovy/ijkplayer/commit/7c647475719617941f031e3bf8678bdd3f2889a2))
* error color pixel format bgra and rgba, rv32 ([501aa9a](https://github.com/befovy/ijkplayer/commit/501aa9a29a881af92b7fac34686b77afada4a5e8))

---
## 0.6.0 (2020-04-11)

* add input process for tui demo ([e479d4e](https://github.com/befovy/ijkplayer/commit/e479d4e75fd05ed3f5e0e64a6722d8a900bd9774))
* add SDL2 audio and video render, add glfw video render. ([9e3da67](https://github.com/befovy/ijkplayer/commit/9e3da67ef7a1c0f71b93157a82f8bbb41331e5bc))
* C style API for Mac OS ([cef0b89](https://github.com/befovy/ijkplayer/commit/cef0b897827fdb81e3cd40e3d14d74e0e47ee5e1))
* ijkplayer desktop works on Mac OS ([8da29be](https://github.com/befovy/ijkplayer/commit/8da29bec7b53579ba7cb4d8e8176b1be219227d0))
* notify current position option ([1629f87](https://github.com/befovy/ijkplayer/commit/1629f874bee484b2999afdf9976a8cd26eedd0df))


---

## 0.5.1 (2020-02-21)


* add libsrt compile support for Android and mac ([#14](https://github.com/befovy/ijkplayer/issues/14)) ([f7764b6](https://github.com/befovy/ijkplayer/commit/f7764b6cd8b4da84910e57b94c0cd5614feb08e7))
* add consumerProguardFiles fix minify crash, add test sign ([48859d7](https://github.com/befovy/ijkplayer/commit/48859d71f567067218d287ed439d448b2946bbdb))
* add missing constant ([30b8baa](https://github.com/befovy/ijkplayer/commit/30b8baa52f5de50cb997189be472ee7fe513c1c1))

---

tag f0.5.0 (2020-01-06)
--------------------------------
- feat: add option cover_after_prepared, show the first video as cover ([c64d2ab](https://github.com/befovy/ijkplayer/commit/c64d2ab5918468753630cb4e1e5364c0df9aecb2))
- feat: make start and pause available during async-preparing ([9e2ef17](https://github.com/befovy/ijkplayer/commit/9e2ef174096dfe0750a0030f4c2ec6a3f4fe847c))

tag f0.4.4
--------------------------------
- desktop: add libyuv for YUV2RGB
- ijkplayer: iOS videotoolbox notify rotate msg
- fix: call start after preparAsync may cause pause state

tag f0.4.3
--------------------------------
- desktop: fix invalid audio channel number for portaudio
- ijkplayer: add current position msg option
- ijkplayer: notify buffering update 100 percent

tag f0.4.0
--------------------------------
- ijkplayer: add osx desktop support, use portaudio for audio output

tag f0.3.8
--------------------------------
- ijkplayer: ios vout add render to pixelbuffer, off screen buffer
- android: add FileMediaDataSource StreamDataSource

tag f0.3.7
--------------------------------
- ffmpeg: enable concat and crypto protocol

tag f0.3.6
--------------------------------
- ijkplayer: add ffp_reset, call ffp_reset in ijkmp_reset

tag f0.3.5
--------------------------------
- fijkplayer: change to state error when notify error
- fijkplayer: add error msg when notify error
- android: add set/get loopCount api

tag f0.3.4
--------------------------------
- ffmpeg: add rtsp support
- ios: remove armv7s

tag f0.3.3
--------------------------------
- ijkplayer: msg_queue_abort move to shutdown
- ijkplayer: update ijkmp_reset, just wait_stop_l, no release
- ios: IJKFFMediaPlayer copy overlay memory pixel to CVPixelBuffer

tag f0.3.2
--------------------------------
- android: add javadoc and source to bintray
- expose all player event to fijkplayer

tag f0.3.1
--------------------------------
- ios: IJKFFMediaPlayer add volume and position api
- android: update Mesage id
- ijkplayer: make sure change to state started

tag f0.3.0
--------------------------------
- change prefix for fijkplayer release tag names `f`
- ios: build static lib FIJKPlayer.framework
- ios: fix FIJKPlayer.podspec error

tag b0.2.0
--------------------------------
- ios: add support for flutter texture
- ios: fix some vtb errors #873
- ijkplayer: new release bintary and CocoaPods

tag k0.8.8
--------------------------------
- ffmpeg: upgrade to 3.4
- ffmpeg: fix hls some issue
- android: fix seek bug when no audio
- openssl: upgrade to 1.0.2n
- ios: vtb support h265

tag k0.8.7
--------------------------------

tag k0.8.6
--------------------------------
- ijkplayer: fix opengl config error
- ffmpeg: fix a concat issue 

tag k0.8.5
--------------------------------
- ijkplayer: fix opengl config error
- ijkplayer: fix some bug about audio

tag k0.8.4
--------------------------------
- ffmpeg: enable hevc by default
- ijkio: support cache share
- ijkplayer: fix some bug

tag k0.8.3
--------------------------------
- ffmpeg: dns cache refactor
- ijkio: cache support synchronize read avoid frequent lseek
- ijkplayer: fix some bug

tag k0.8.2
--------------------------------
- ffmpeg: fix some bug
- ijkio: update and modify features
- ijkplayer: support don't calculate real frame rate, the first frame will speed up

tag k0.8.1
--------------------------------
- ffmpeg: support dns cache
- ijkio: support inject extra node

tag k0.8.0
--------------------------------
- ffmpeg: upgrade to 3.3
- ffmpeg: enable flac
- android: support sync mediacodec
- android: support framedrop when use mediacodec
- openssl: upgrade to 1.0.2k
- jni4android: upgrade to v0.0.2

tag k0.7.9
--------------------------------
- ffmpeg: add tcp timeout control
- android: support soundtouch

tag k0.7.8
--------------------------------
- ffplay: support accurate seek
- ijkio: fix some issue
- ios: add ijkplayer dynamic target with ssl

tag k0.7.7
--------------------------------
- ffmpeg: enable ijkio protocol
- ffmpeg: avoid some unreasonable pts
- ios: fix a crash caused by videotoolbox sync initialization fail

tag k0.7.6
--------------------------------
- ffmpeg: ass subtitle support
- msg_queue: add resource for msg_queue
- ios: separate vtb sync mode from mixed vtb
- android: fix some thread competition
- android: support setSpeed for pre-M(api<23) versions

tag k0.7.5
--------------------------------
- ffmpeg: disable-asm on architecture x86
- ffmpeg: revert some cutted demuxer and decoder
- ios: add playback volume interface

tag k0.7.4
--------------------------------
- ffplay: fix sample buffer leak introduced in k0.7.1
- doc: add takeoff checklist

tag k0.7.3
--------------------------------
- ios: turn videotoolbox into singleton
- ffmpeg: merge ipv6 issue in tcp.c

tag k0.7.2
-------------------------------
- ios: fix a compile error

tag k0.7.1
-------------------------------
- ffmpeg: upgrade to n3.2

tag k0.6.3
--------------------------------
- ffmpeg: disable clock_gettime added in xcode8
- android: make NDKr13 happy

tag k0.6.2
--------------------------------
- ffmpeg: fix wild pointer when decoder was not found
- player: fix bug introduced in k0.6.0

tag k0.6.1
--------------------------------
- concat: fix crash introduced in k0.6.0
- flvdec: fix seek problem introduced in k0.6.0
- hls: fix regression with ranged media segments

tag k0.6.0
--------------------------------
- openssl: upgrade to 1.0.2h
- ffmpeg: upgrade to n3.1
- MediaCodec: add options to enable resolution change.
- VideoToolbox: add options to enable resolution change.

tag k0.5.1
--------------------------------
- ffmpeg: fix crash introduced in k0.5.0

tag k0.5.0
--------------------------------
- ffmpeg: upgrade to n3.0
- android: support NDKr11

tag k0.4.5
--------------------------------
- ios: support playbackRate change. (iOS 7.0 or later)
- android: support speed change. (Android 6.0 or later)
- player: do not link avfilter by default.
- android: add x86_64 support
- android: move jjk out to jni4android project
- android: support OpenGL ES2 render

tag k0.4.4
--------------------------------
- ios: replace MPMoviePlayerXXX with IJKMPMoviePlayerXXX
- ios: remove target 'IjkMediaPlayer'. 'IjkMediaFramework' should be used instead.
- android: switch ExoPlayer to r1.5.2

tag k0.4.3
--------------------------------
- android: fix several crash when reconfiguring MediaCodec
- android: add jjk to generate API native wrapper
- android: support IMediaDataSource for user to supply media data

tag k0.4.2
--------------------------------
- ios: support Xcode 7
- ios: drop support of iOS 5.x
- ffmpeg: enable libavfilter
- player: limited support of libavfilter
- android: add ExoPlayer as an alternative backend player

tag k0.4.1
--------------------------------
- android: support downloading from jcenter

tag k0.4.0
--------------------------------
- ffmpeg: switch to ffmpeg n2.8

tag k0.3.3
--------------------------------
- player: custom protocol as io hook
- android/sample: support rotation meta (TextureView only)

tag k0.3.2
--------------------------------
- android: drop support of Eclipse
- android: update to SDK 23
- android/sample: better UI
- ios: support SAR
- android/sample: support background play

tag k0.3.1
--------------------------------
- player: key-value options API
- player: remove ijkutil
- build: support cygwin
- ios: optimize performance of VideoToolbox.

tag k0.3.0
--------------------------------
- android: support build with Android Studio / Gradle
- build: improve library fetch
- openssl: switch to openssl 1.0.1o

tag k0.2.4
--------------------------------
- ios: remove armv7s build from default
- player: introduce key-value options
- ios: demo improvement
- ios: support init/play in background.
- ffmpeg: switch to ffmpeg n2.7

tag k0.2.3
--------------------------------
- android: support OpenSL ES
- ios: support NV12 Render
- ios: support VideoToolBox
- ffmpeg: switch to ffmpeg n2.6

tag n0.2.2:
--------------------------------
- ffmpeg: switch to ffmpeg n2.5
- android: fix leak in jni
- player: retrieve media informations

tag n0.2.1:
--------------------------------
- android: support MediaCodec (API 16+)

tag n0.2.0
--------------------------------
- player: fix crash on invalid audio
- android: support build with ndk-r10
- ios: add IJKAVMoviePlayerController based on AVPlayer API
- ios: remove some unused interface
- ios8: fix latency of aout_pause_audio()
- ios8: upgrade project
- ffmpeg: switch to ffmpeg n2.4

tag n0.1.3
--------------------------------
- ffmpeg: switch to ffmpeg n2.2
- player: fix complete/error state handle
- ffmpeg: build with x86_64, armv5
- android: replace vlc-chroma-asm with libyuv

tag n0.1.2:
--------------------------------
- ffmpeg: build with openssl
- player: fix aout leak
- player: reduce memory footprint for I420/YV12 overlay
- ios: snapshot last displayed image

tag n0.1.1:
--------------------------------
- player: remove ugly frame drop trick
- ios: simplify application state handle
- ios: fix 5.1 channel support
- player: handle ffmpeg error
- player: fix leak
- player: improve buffer indicator
- player: drop frame for high fps video

tag n0.1.0:
--------------------------------
- android: replace AbstractMediaPlayer with IMediaPlayer and other misc interfaces
- android: remove list player classes due to lack of regression test
- ios: support build with SDK7
- ffmpeg: switch to n2.1 base
- ios: fix possible block on ijkmp_pause
- ios: set CAEAGLLayer.contentsScale to avoid bad image on retina devices
- ios: fix handle of AudioSession interruption
- ios: add AudioQueue api as replacement of AudioUnit api
- ijksdl: fix non-I420 pixel-format support
- player: improve late packet/frame dropping
- player: prefer h264 stream if multiple video stream exists

tag n0.0.6:
--------------------------------
- android: fix NativeWindow leak
- ios: fix a deadlock related to AudioUnit
- ios: support ffmpeg concat playback
- ios: add ffmpeg options methods
- android: limait audio sample-rate to 4kHz~48kHz
- ios: fix gles texture alignment

tag n0.0.5:
--------------------------------
- build: disable -fmodulo-sched -fmodulo-sched-allow-regmoves, may crash on gcc4.7~4.8
- player: support ios
- ijksdl: support ios gles2 video output
- ijksdl: support ios AudioUnit audio output
- build: add android/ios sub directory
- player: fix some dead lock
- build: use shell scripts instead of git-submodule
- android: use RV32 as default chroma

tag n0.0.4:
--------------------------------
- ffmpeg: enable ac3
- android: target API-18
- build: switch to NDKr9 gcc4.8 toolchain

tag n0.0.3:
--------------------------------
- ffmpeg: switch to tag n2.0
- ffmpeg: remove rarely used decoders, parsers, demuxers
- avformat/hls: fix many bugs
- avformat/http: support reading compressed data
- avformat/mov: optimize short seek
- player: fix AudioTrack latency
- player: refactor play/pause/step/buffering logic
- player: fix A/V sync
- yuv2rgb: treat YUVJ420P as YUV420P
- yuv2rgb: support zero copy of YUV420P frame output to YV12 surface
- ijksdl: fix SDL_GetTickHR() returns wrong time 
