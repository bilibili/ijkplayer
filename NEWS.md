tag <next>:
--------------------------------
player: fix crash on invalid audio
android: support build with ndk-r10

tag n0.1.3
--------------------------------
ffmpeg: switch to ffmpeg n2.2
player: fix complete/error state handle
ffmpeg: build with x86_64, armv5
android: replace vlc-chroma-asm with libyuv

tag n0.1.2:
--------------------------------
ffmpeg: build with openssl
player: fix aout leak
player: reduce memory footprint for I420/YV12 overlay
ios: snapshot last displayed image

tag n0.1.1:
--------------------------------
player: remove ugly frame drop trick
ios: simplify application state handle
ios: fix 5.1 channel support
player: handle ffmpeg error
player: fix leak
player: improve buffer indicator
player: drop frame for high fps video

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
