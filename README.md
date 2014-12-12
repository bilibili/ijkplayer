ijkplayer
=========
- Video player based on [ffplay](http://ffmpeg.org)
 - Android: [MediaPlayer-like](android/ijkmediaplayer/src/tv/danmaku/ijk/media/player/AbstractMediaPlayer.java)
 - iOS: [MediaPlayer.framework-like](ios/IJKMediaPlayer/IJKMediaPlayer/IJKMediaPlayback.h)

### My Build Enviroment
- Common
 - Mac OS X 10.9.5
- Android
 - [ADT v23.0.4-1468518](http://developer.android.com/sdk/index.html)
 - [NDK r10c](http://developer.android.com/tools/sdk/ndk/index.html)
- iOS
 - Xcode 6.1.0
- [HomeBrew](http://brew.sh)
 - ruby -e "$(curl -fsSL https://raw.github.com/Homebrew/homebrew/go/install)"
 - brew install git

### Latest Changes
- [NEWS.md](NEWS.md)

### Features
- Common
 - remove rarely used ffmpeg components to reduce binary size [config/module-lite.sh](config/module-lite.sh)
 - workaround for some buggy online video.
- Android
 - platform: API 9~21
 - cpu: ARMv7a, x86, ARMv5 (not tested on real devices)
 - api: [MediaPlayer-like](android/ijkmediaplayer/src/tv/danmaku/ijk/media/player/IMediaPlayer.java)
 - video output: NativeWindow
 - audio output: AudioTrack
 - hw decoder: MediaCodec
- iOS
 - platform: iOS 5.1.1~8.1.x
 - cpu: ARMv7, ARMv7s, ARM64, i386, x86_64
 - api: [MediaPlayer.framework-like](ios/IJKMediaPlayer/IJKMediaPlayer/IJKMediaPlayback.h)
 - video-output: OpenGL ES 2.0 (I420/YV12 shaders)
 - audio-output: AudioQueue, AudioUnit

### TODO
- iOS
 - api: AVFoundation-like
 - hw-accelerator: HW decode

### NOT-ON-PLAN
- obsolete platforms (Android: API-8 and below; iOS: below 5.1.1)
- obsolete cpu: ARMv5, ARMv6, MIPS (I don't even have these types of devices…)
- native subtitle render
- cygwin compatibility

### Before Build
- If you prefer more codec/format
```
rm config/module.sh
ln -fs config/module-default.sh config/module.sh
```

- If you prefer less codec/format for smaller binary size (by default)
```
rm config/module.sh
ln -fs config/module-lite.sh config/module.sh
```

- For Ubuntu/Debian users.
```
# choose [No] to use bash
sudo dpkg-reconfigure dash
```

- If you'd like to share your config, pull request is welcome.

### Build Android
```
git clone https://github.com/bbcallen/ijkplayer.git ijkplayer-android
cd ijkplayer-android
git checkout -B latest n0.2.1
# or for master
# git checkout master

./init-android.sh

cd android
./compile-ffmpeg.sh
./compile-ijk.sh

# or Add Native Support in eclipse
# cd ijkmediaplayer
# cp .cproject.bak .cproject

# import android/ijkmediaplayer for MediaPlayer-like interface (recommended)
# import android/ijkmediawidget for VideoView-like interface (based on Vitamio UI)
# import android/ijkmediademo for VideoActivity demo (Simple VideoActivity)
```


### Build iOS
```
git clone https://github.com/bbcallen/ijkplayer.git ijkplayer-ios
cd ijkplayer-ios
git checkout -B latest n0.2.1
# or for master
# git checkout master

./init-ios.sh

cd ios
./compile-ffmpeg.sh all

# import ios/IJKMediaPlayer for MediaPlayer.framework-like interface (recommended)
# open ios/IJKMediaDemo/IJKMediaDemo.xcodeproj with Xcode
```


### Links
- [FFmpeg_b4a](http://www.basic4ppc.com/android/forum/threads/ffmpeg_b4a-a-ffmpeg-library-for-b4a-decoding-streaming.44476/)
- 中文
 - [ijkplayer学习系列之环境搭建 2013-11-23](http://blog.csdn.net/nfer_zhuang/article/details/16905755)
 - [Ubuntu 14.04 下编译 ijkplayer Android 2014-08-01](http://xqq.0ginr.com/ijkplayer-build/#more-134)

### License

```
Copyright (C) 2013-2014 Zhang Rui <bbcallen@gmail.com> 
Licensed under LGPLv2.1 or later
```

ijkplayer is based on or derives from projects below:
- LGPL
  - [FFmpeg](http://git.videolan.org/?p=ffmpeg.git)
  - [libVLC](http://git.videolan.org/?p=vlc.git)
  - [kxmovie](https://github.com/kolyvan/kxmovie)
- zlib license
  - [SDL](http://www.libsdl.org)
- Apache License v2
  - [VitamioBundle](https://github.com/yixia/VitamioBundle)
- BSD-style license
  - [libyuv](https://code.google.com/p/libyuv/)
- ISC license
  - [libyuv/source/x86inc.asm](https://code.google.com/p/libyuv/source/browse/trunk/source/x86inc.asm)

ijkplayer's build scripts are based on or derives from projects below:
- [gas-preprocessor](http://git.libav.org/?p=gas-preprocessor.git)
- [VideoLAN](http://git.videolan.org)
- [yixia/FFmpeg-Android](https://github.com/yixia/FFmpeg-Android)
- [kewlbear/FFmpeg-iOS-build-script](http://github.com/kewlbear/FFmpeg-iOS-build-script) 

### Commercial Use
ijkplayer is licensed under LGPLv2.1 or later, so itself is free for commercial use under LGPLv2.1 or later

But ijkplayer is also based on other different projects under various licenses, which I have no idea whether they are compatible to each other or to your product.

[IANAL](http://en.wikipedia.org/wiki/IANAL), you should always ask your lawyer for these stuffs before use it in your product.
