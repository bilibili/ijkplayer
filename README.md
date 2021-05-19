# ijkplayer

 Platform | Build Status
 -------- | ------------
 Android | [![Build Status](https://travis-ci.org/Bilibili/ci-ijk-ffmpeg-android.svg?branch=master)](https://travis-ci.org/Bilibili/ci-ijk-ffmpeg-android)
 iOS | [![Build Status](https://travis-ci.org/Bilibili/ci-ijk-ffmpeg-ios.svg?branch=master)](https://travis-ci.org/Bilibili/ci-ijk-ffmpeg-ios)

Video player based on [ffplay](http://ffmpeg.org)


### Features
- Common
 - remove rarely used ffmpeg components to reduce binary size [config/module-lite.sh](config/module-lite.sh)
 - workaround for some buggy online video.
- Android
 - platform: API 9~23
 - cpu: ARMv7a, ARM64v8a, x86 (ARMv5 is not tested on real devices)
 - api: [MediaPlayer-like](android/ijkplayer/ijkplayer-java/src/main/java/tv/danmaku/ijk/media/player/IMediaPlayer.java)
 - video-output: NativeWindow, OpenGL ES 2.0
 - audio-output: AudioTrack, OpenSL ES
 - hw-decoder: MediaCodec (API 16+, Android 4.1+)
 - alternative-backend: android.media.MediaPlayer, ExoPlayer
- iOS
 - platform: iOS 7.0~10.2.x
 - cpu: armv7, arm64, i386, x86_64, (armv7s is obselete)
 - api: [MediaPlayer.framework-like](ios/IJKMediaPlayer/IJKMediaPlayer/IJKMediaPlayback.h)
 - video-output: OpenGL ES 2.0
 - audio-output: AudioQueue, AudioUnit
 - hw-decoder: VideoToolbox (iOS 8+)
 - alternative-backend: AVFoundation.Framework.AVPlayer, MediaPlayer.Framework.MPMoviePlayerControlelr (obselete since iOS 8)


### Android

#### prerequisites

- Host OS information:

```
uname -a

Linux 5.8.0-43-generic #49~20.04.1-Ubuntu SMP Fri Feb 5 09:57:56 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux

cat /etc/issue

Ubuntu 20.04.2 LTS \n \l

```

- [ndk-r14b](https://developer.android.com/ndk)
- Android Studio 4.2.1
- Gradle 6.6.1

#### Before Build

```
git clone https://github.com/zhouwg/ijkplayer
cd ijkplayer
git checkout dev-baseon-latest-k0.8.8


# add these lines to your ~/.bash_profile or ~/.profile
# export ANDROID_SDK=<your sdk path>
# ndk-r14b was used in this project
# export ANDROID_NDK=<your ndk path>/android-ndk-r14b

# add these line to ./android/ijkplayer/local.properties

sdk.dir=<your sdk path>
ndk.dir=<your ndk path>/android-ndk-r14b

```

#### Build Android

step1:build all native libs

```
./build-all-android-native-libs.sh clean
time ./build-all-android-native-libs.sh build

```

step2: build APK


build apk by latest Android Studio IDE


### iOS

```
 refers to

 [https://github.com/bilibili/ijkplayer/blob/master/README.md](https://github.com/bilibili/ijkplayer/blob/master/README.md)

```


### Support

- Please do not send e-mail to me. Public technical discussion on github is preferred.
- feel free to submit issues or new features(focus on Android at the moment), volunteer support would be provided if time permits.


### Latest Changes
- [NEWS.md](NEWS.md)


### License

```
Copyright (c) 2017 Bilibili
Licensed under LGPLv2.1 or later
```

ijkplayer required features are based on or derives from projects below:
- LGPL
  - [FFmpeg](http://git.videolan.org/?p=ffmpeg.git)
  - [libVLC](http://git.videolan.org/?p=vlc.git)
  - [kxmovie](https://github.com/kolyvan/kxmovie)
  - [soundtouch](http://www.surina.net/soundtouch/sourcecode.html)
- zlib license
  - [SDL](http://www.libsdl.org)
- BSD-style license
  - [libyuv](https://code.google.com/p/libyuv/)
- ISC license
  - [libyuv/source/x86inc.asm](https://code.google.com/p/libyuv/source/browse/trunk/source/x86inc.asm)

android/ijkplayer-exo is based on or derives from projects below:
- Apache License 2.0
  - [ExoPlayer](https://github.com/google/ExoPlayer)

android/example is based on or derives from projects below:
- GPL
  - [android-ndk-profiler](https://github.com/richq/android-ndk-profiler) (not included by default)

ios/IJKMediaDemo is based on or derives from projects below:
- Unknown license
  - [iOS7-BarcodeScanner](https://github.com/jpwiddy/iOS7-BarcodeScanner)

ijkplayer's build scripts are based on or derives from projects below:
- [gas-preprocessor](http://git.libav.org/?p=gas-preprocessor.git)
- [VideoLAN](http://git.videolan.org)
- [yixia/FFmpeg-Android](https://github.com/yixia/FFmpeg-Android)
- [kewlbear/FFmpeg-iOS-build-script](https://github.com/kewlbear/FFmpeg-iOS-build-script) 

### Commercial Use
ijkplayer is licensed under LGPLv2.1 or later, so itself is free for commercial use under LGPLv2.1 or later

But ijkplayer is also based on other different projects under various licenses, which I have no idea whether they are compatible to each other or to your product.

[IANAL](https://en.wikipedia.org/wiki/IANAL), you should always ask your lawyer for these stuffs before use it in your product.
