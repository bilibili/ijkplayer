ijkplayer
=========
- Video player based on [ffplay](http://ffmpeg.org)
 - Android: MediaPlayer-like 
 - iOS: MediaPlayer.framework-like

### My Build Enviroment
- Common
 - Mac OS X 10.8.4
- Android
 - ADT v22.0.4-741630
 - NDK r9
- iOS
 - Xcode 4.6.3 (with iOS SDK 6.1)

### Dependent Tools (Mac OS X)
- MacPorts http://www.macports.org/install.php
 - sudo port -vd install git-core
 - sudo port -vd install ccache

### Features
- Common
 - remove rarely used ffmpeg components to reduce binary size
 - workaround for some buggy online video.
- Android
 - platform: API 9~18
 - cpu: ARMv7a-NEON
 - api: MediaPlayer-like
 - video output: NativeWindow (YUV2RGB shaders)
 - audio output: AudioTrack
- iOS
 - platform: iOS 5.0~7.0.x
 - cpu: ARMv7, ARMv7s, i386
 - api: MediaPlayer.framework-like
 - video-output: OpenGL ES 2.0 (YUV2RGB shaders)
 - audio-output: CoreAudio

### TODO
- Android
 - video-output: OpenGL ES 2.0 (YUV2RGB shaders)
 - hw-accelerator: HW decode
- iOS
 - api: AVFoundation-like
 - hw-accelerator: HW decode

### Build Android
- Stable

```
git clone https://github.com/bbcallen/ijkplayer.git ijkplayer-android-stable
cd ijkplayer-android-stable
git checkout -B stable n0.0.5

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

# import android/ijkmediaplayer for MediaPlayer-like interface (recommended)
# import android/ijkmediawidget for VideoView-like interface (based on Vitamio UI)
# import android/ijkmediademo for VideoActivity demo (Simple VideoActivity)
```

### iOS (works, but not really stable for now)
- Stable

```
git clone https://github.com/bbcallen/ijkplayer.git ijkplayer-ios-stable
cd ijkplayer-ios-stable
git checkout -B stable n0.0.5

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

Other libraries projects are distributed on theirs own license:
- LGPL
  - FFmpeg http://git.videolan.org/?p=ffmpeg.git
  - libVLC http://git.videolan.org/?p=vlc.git
  - kxmovie https://github.com/kolyvan/kxmovie
- zlib license
  - SDL http://www.libsdl.org
- Apache License v2
  - VitamioBundle https://github.com/yixia/VitamioBundle

