tag next
--------------------------------
- ffmpeg: upgrade to 4.0

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
