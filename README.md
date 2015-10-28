ijkplayer
=========
- Video player based on [ffplay](http://ffmpeg.org)

### Download

- Android:
 - Gradle
```
# required
allprojects {
    repositories {
        jcenter()
    }
}

dependencies {
    # required, enough for most devices.
    compile 'tv.danmaku.ijk.media:ijkplayer-java:0.4.2.4'
    compile 'tv.danmaku.ijk.media:ijkplayer-armv7a:0.4.2.4'

    # Other ABIs: optional
    compile 'tv.danmaku.ijk.media:ijkplayer-armv5:0.4.2.4'
    compile 'tv.danmaku.ijk.media:ijkplayer-arm64:0.4.2.4'
    compile 'tv.danmaku.ijk.media:ijkplayer-x86:0.4.2.4'

    # ExoPlayer as IMediaPlayer: optional, experimental
    compile 'tv.danmaku.ijk.media:ijkplayer-exo:0.4.2.4'
}
```
- iOS
 - in coming...

### My Build Environment
- Common
 - Mac OS X 10.10.5
- Android
 - [NDK r10e](http://developer.android.com/tools/sdk/ndk/index.html)
 - Android Studio 1.4
 - Gradle 2.6
- iOS
 - Xcode 7.0 (7A220)
- [HomeBrew](http://brew.sh)
 - ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
 - brew install git

### Latest Changes
- [NEWS.md](NEWS.md)

### Features
- Common
 - remove rarely used ffmpeg components to reduce binary size [config/module-lite.sh](config/module-lite.sh)
 - workaround for some buggy online video.
- Android
 - platform: API 9~23
 - cpu: ARMv7a, ARM64v8a, x86 (ARMv5 is not tested on real devices)
 - api: [MediaPlayer-like](android/ijkplayer/ijkplayer-java/src/main/java/tv/danmaku/ijk/media/player/IMediaPlayer.java)
 - video-output: NativeWindow
 - audio-output: OpenSL ES, AudioTrack
 - hw-decoder: MediaCodec (API 16+, Android 4.1+)
 - alternative-backend: android.media.MediaPlayer, ExoPlayer
- iOS
 - platform: iOS 6.0~8.4.x
 - cpu: armv7, arm64, i386, x86_64, (armv7s is obselete)
 - api: [MediaPlayer.framework-like](ios/IJKMediaPlayer/IJKMediaPlayer/IJKMediaPlayback.h)
 - video-output: OpenGL ES 2.0 (I420/YV12/NV12 shaders)
 - audio-output: AudioQueue, AudioUnit
 - hw-decoder: VideoToolbox (iOS 8+)
 - alternative-backend: AVFoundation.Framework.AVPlayer, MediaPlayer.Framework.MPMoviePlayerControlelr (obselete since iOS 8) 

### TODO
- iOS
 - api: AVPlayer-like

### NOT-ON-PLAN
- obsolete platforms (Android: API-8 and below; iOS: pre-6.0)
- obsolete cpu: ARMv5, ARMv6, MIPS (I don't even have these types of devices…)
- native subtitle render
- avfilter support

### Before Build
```
# install homebrew, git, yasm
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
brew install git
brew install yasm

# add these lines to your ~/.bash_profile or ~/.profile
# export ANDROID_SDK=<your sdk path>
# export ANDROID_NDK=<your ndk path>

# on Cygwin (unmaintained)
# install git, make, yasm
```

- If you prefer more codec/format
```
cd config
rm module.sh
ln -s module-default.sh module.sh
cd android/contrib
# cd ios
sh compile-ffmpeg clean
```

- If you prefer less codec/format for smaller binary size (by default)
```
cd config
rm module.sh
ln -s module-lite.sh module.sh
cd android/contrib
# cd ios
sh compile-ffmpeg clean
```

- For Ubuntu/Debian users.
```
# choose [No] to use bash
sudo dpkg-reconfigure dash
```

- If you'd like to share your config, pull request is welcome.

### Build Android
```
git clone https://github.com/Bilibili/ijkplayer.git ijkplayer-android
cd ijkplayer-android
git checkout -B latest k0.4.2.4

./init-android.sh

cd android/contrib
./compile-ffmpeg.sh clean
./compile-ffmpeg.sh all

cd ..
./compile-ijk.sh all

# Android Studio:
#     Open an existing Android Studio project
#     Select android/ijkplayer/ and import
#
#     define ext block in your root build.gradle
#     ext {
#       compileSdkVersion = 23       // depending on your sdk version
#       buildToolsVersion = "23.0.0" // depending on your build tools version
#
#       targetSdkVersion = 23        // depending on your sdk version
#     }
#
# Eclipse: (obselete)
#     File -> New -> Project -> Android Project from Existing Code
#     Select android/ and import all project
#     Import appcompat-v7
#     Import preference-v7
#
# Gradle
#     cd ijkplayer
#     gradle

```


### Build iOS
```
git clone https://github.com/Bilibili/ijkplayer.git ijkplayer-ios
cd ijkplayer-ios
git checkout -B latest k0.4.2.4

./init-ios.sh

cd ios
./compile-ffmpeg.sh clean
./compile-ffmpeg.sh all

# import ios/IJKMediaPlayer for MediaPlayer.framework-like interface (recommended)
# open ios/IJKMediaDemo/IJKMediaDemo.xcodeproj with Xcode
```


### Support (支持) ###
- Although not all issues can be well resolved by me in time, but they are welcome, and could be resolved by other developers.
- 能力所限，我个人无法及时有效解决所有问题，不过仍然欢迎[提交问题](https://github.com/bilibili/ijkplayer/issues)。考虑到某些问题有可能被老外大牛看到并解决，建议尽量用英文提问，以获得更多支持。
- Please do not send e-mail to me. Public technical discussion on github is preferred.
- 请尽量在 github 上公开讨论[技术问题](https://github.com/bilibili/ijkplayer/issues)，不要以邮件方式私下询问，恕不一一回复。


### License

```
Copyright (C) 2013-2015 Zhang Rui <bbcallen@gmail.com> 
Licensed under LGPLv2.1 or later
```

ijkplayer required features are based on or derives from projects below:
- LGPL
  - [FFmpeg](http://git.videolan.org/?p=ffmpeg.git)
  - [libVLC](http://git.videolan.org/?p=vlc.git)
  - [kxmovie](https://github.com/kolyvan/kxmovie)
- zlib license
  - [SDL](http://www.libsdl.org)
- BSD-style license
  - [libyuv](https://code.google.com/p/libyuv/)
- ISC license
  - [libyuv/source/x86inc.asm](https://code.google.com/p/libyuv/source/browse/trunk/source/x86inc.asm)

android/ijkplayer-exo is based on or derives from projects below:
- Apache License 2.0
  - [ExoPlayer](https://github.com/google/ExoPlayer)

android/sample is based on or derives from projects below:
- GPL
  - [android-ndk-profiler](https://github.com/richq/android-ndk-profiler) (not included by default)

ios/IJKMediaDemo is based on or derives from projects below:
- Unknown license
  - [iOS7-BarcodeScanner](https://github.com/jpwidmer/iOS7-BarcodeScanner)

ijkplayer's build scripts are based on or derives from projects below:
- [gas-preprocessor](http://git.libav.org/?p=gas-preprocessor.git)
- [VideoLAN](http://git.videolan.org)
- [yixia/FFmpeg-Android](https://github.com/yixia/FFmpeg-Android)
- [kewlbear/FFmpeg-iOS-build-script](http://github.com/kewlbear/FFmpeg-iOS-build-script) 

### Commercial Use
ijkplayer is licensed under LGPLv2.1 or later, so itself is free for commercial use under LGPLv2.1 or later

But ijkplayer is also based on other different projects under various licenses, which I have no idea whether they are compatible to each other or to your product.

[IANAL](http://en.wikipedia.org/wiki/IANAL), you should always ask your lawyer for these stuffs before use it in your product.
