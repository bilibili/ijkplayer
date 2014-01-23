ijkplayer
=========
- Video player based on [ffplay](http://ffmpeg.org)
 - Android: [MediaPlayer-like](android/ijkmediaplayer/src/tv/danmaku/ijk/media/player/AbstractMediaPlayer.java)
 - iOS: [MediaPlayer.framework-like](ios/IJKMediaPlayer/IJKMediaPlayer/IJKMediaPlayback.h)

### My Build Enviroment
- Common
 - Mac OS X 10.8.5
- Android
 - [ADT v22.3.0-887826](http://developer.android.com/sdk/index.html)
 - [NDK r9](http://developer.android.com/tools/sdk/ndk/index.html)
- iOS
 - Xcode 5.0.2 (with iOS SDK 7)

### Dependent Tools (Mac OS X)
- [MacPorts](http://www.macports.org/install.php)
 - sudo port -vd install git-core
 - sudo port -vd install ccache

### Latest Changes
- [NEWS.md](NEWS.md)

### Features
- Common
 - remove rarely used ffmpeg components to reduce binary size
 - workaround for some buggy online video.
- Android
 - platform: API 9~19
 - cpu: ARMv7a-NEON
 - api: [MediaPlayer-like](android/ijkmediaplayer/src/tv/danmaku/ijk/media/player/AbstractMediaPlayer.java)
 - video output: NativeWindow
 - audio output: AudioTrack
- iOS
 - platform: iOS 5.0~7.0.x
 - cpu: ARMv7, ARMv7s, i386
 - api: [MediaPlayer.framework-like](ios/IJKMediaPlayer/IJKMediaPlayer/IJKMediaPlayback.h)
 - video-output: OpenGL ES 2.0 (YUV2RGB shaders)
 - audio-output: CoreAudio

### TODO
- Android
 - video-output: OpenGL ES 2.0 (YUV2RGB shaders)
 - hw-accelerator: HW decode
 - cpu: x86 (maybe)
- iOS
 - api: AVFoundation-like
 - hw-accelerator: HW decode

### NOT-ON-PLAN
- obsolete playforms (Android: API 8-; iOS: 4.x-)
- obsolete cpu: ARMv5, ARMv6, MIPS (I don't even have these types of devices…)
- native subtitle render

### Build Android
- Stable

```
git clone https://github.com/bbcallen/ijkplayer.git ijkplayer-android-stable
cd ijkplayer-android-stable
git checkout -B stable n0.0.6

sh init-android.sh

cd android
sh compile-ffmpeg.sh
sh compile-ijk.sh

# import android/ijkmediaplayer for MediaPlayer-like interface (recommended)
# import android/ijkmediawidget for VideoView-like interface (based on Vitamio UI)
# import android/ijkmediademo for VideoActivity demo (Simple VideoActivity)
```

- Unstable

```
git clone https://github.com/bbcallen/ijkplayer.git ijkplayer-android-master
cd ijkplayer-android-master
git checkout master

sh init-android.sh

cd android
sh compile-ffmpeg.sh
sh compile-ijk.sh

cd ijkmediaplayer/jni
ndk-build

# or Add Native Support in eclipse
# cd ijkmediaplayer
# cp .cproject.bak .cproject

# import android/ijkmediaplayer for MediaPlayer-like interface (recommended)
# import android/ijkmediawidget for VideoView-like interface (based on Vitamio UI)
# import android/ijkmediademo for VideoActivity demo (Simple VideoActivity)
```

### iOS (works, but not really stable for now)
- Stable

```
git clone https://github.com/bbcallen/ijkplayer.git ijkplayer-ios-stable
cd ijkplayer-ios-stable
git checkout -B stable n0.0.6

sh init-ios.sh

cd ios
sh compile-ffmpeg.sh all

# import ios/IJKMediaPlayer for MediaPlayer.framework-like interface (recommended)
# open ios/IJKMedia.xcworkspace with Xcode
```

- Unstable

```
git clone https://github.com/bbcallen/ijkplayer.git ijkplayer-ios-master
cd ijkplayer-ios-master
git checkout master

sh init-ios.sh

cd ios
sh compile-ffmpeg.sh all

# import ios/IJKMediaPlayer for MediaPlayer.framework-like interface (recommended)
# open ios/IJKMedia.xcworkspace with Xcode
```

### License

```
Copyright (C) 2013 Zhang Rui <bbcallen@gmail.com> 
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

### Commercial Use
ijkplayer is licensed under LGPLv2.1 or later, so itself is free for commercial use under LGPLv2.1 or later

But ijkplayer is also based on other different projects under various licenses, which I have no idea whether they are compatible to each other or to your product.

[IANAL](http://en.wikipedia.org/wiki/IANAL), you should always ask your lawyer for these stuffs before use it in your product.

[![xrefs](https://sourcegraph.com/api/repos/github.com/bbcallen/ijkplayer/badges/xrefs.png)](https://sourcegraph.com/github.com/bbcallen/ijkplayer)
