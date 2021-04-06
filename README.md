ijkplayer
=========
- Video player based on [ffplay](http://ffmpeg.org)
 - Android: [MediaPlayer-like](android/ijkmediaplayer/src/tv/danmaku/ijk/media/player/AbstractMediaPlayer.java)
 - iOS: [MediaPlayer.framework-like](ios/IJKMediaPlayer/IJKMediaPlayer/IJKMediaPlayback.h)

### My Build Enviroment
- Common
 - Mac OS X 10.10.3
- Android
 - [NDK r10e](http://developer.android.com/tools/sdk/ndk/index.html)
 - Android Studio 1.2.2
- iOS
 - Xcode 6.3.2 (6D2105)
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
 - platform: API 9~22
 - cpu: ARMv7a, x86, ARMv5 (ARMv5 is not tested on real devices)
 - api: [MediaPlayer-like](android/ijkmediaplayer/src/tv/danmaku/ijk/media/player/IMediaPlayer.java)
 - video output: NativeWindow
 - audio output: OpenSL ES, AudioTrack
 - hw decoder: MediaCodec (API 16+, Android 4.1+)
- iOS
 - platform: iOS 5.1.1~8.3.x
 - cpu: ARMv7, ARM64, i386, x86_64, (armv7s is obselete)
 - api: [MediaPlayer.framework-like](ios/IJKMediaPlayer/IJKMediaPlayer/IJKMediaPlayback.h)
 - video-output: OpenGL ES 2.0 (I420/YV12/NV12 shaders)
 - audio-output: AudioQueue, AudioUnit
 - hw decoder: VideoToolbox (iOS 8+)

### TODO
- iOS
 - api: AVFoundation-like
- Build 
 - cygwin compatibility

### NOT-ON-PLAN
- obsolete platforms (Android: API-8 and below; iOS: below 5.1.1)
- obsolete cpu: ARMv5, ARMv6, MIPS (I don't even have these types of devices…)
- native subtitle render

### Before Build
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
git checkout -B latest k0.3.0

./init-android.sh

cd android/contrib
./compile-ffmpeg.sh clean
./compile-ffmpeg.sh

cd ..
./compile-ijk.sh

# Eclipse:
#     File -> New -> Project -> Android Project from Existing Code
#     Select android/ and import all project
#
# Android Studio:
#     Open an existing Android Studio project
#     Select android/ijkplayer/ and import
#
# Gradle
#     cd ijkplayer
#     gradle

```


### Build iOS
```
git clone https://github.com/Bilibili/ijkplayer.git ijkplayer-ios
cd ijkplayer-ios
git checkout -B latest k0.3.0

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


### Links
- [FFmpeg_b4a](http://www.basic4ppc.com/android/forum/threads/ffmpeg_b4a-a-ffmpeg-library-for-b4a-decoding-streaming.44476/)
- 中文
 - [ijkplayer学习系列之环境搭建 2013-11-23](http://blog.csdn.net/nfer_zhuang/article/details/16905755)
 - [Ubuntu 14.04 下编译 ijkplayer Android 2014-08-01](http://xqq.0ginr.com/ijkplayer-build/#more-134)

### License

```
Copyright (C) 2013-2015 Zhang Rui <bbcallen@gmail.com> 
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



## Android编译IjkPlayer流程 ##

	Copy From: https://blog.csdn.net/casemate/article/details/114766740

### 配置环境 ###

	JDK   1.8.0+
	SDK
	NDK
	Git
	Make
	yasm

### 1. 编译工具安装 ###

	yum install git cmake gcc g++

	wget http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz
	tar zxvf yasm-1.3.0.tar.gz
	cd yasm-1.3.0
	./configure
	make && make install
 

### 2. SDK安装 ###

	下载SDK
	wget  https://dl.google.com/android/android-sdk_r24.2-linux.tgz


	解压SDK
	tar xvzf android-sdk_r24.2-linux.tgz

	移动到/opt目录下
	 mv android-sdk-linux /opt

	打开环境变量配置文件
	 gedit ~/.bashrc
	
	添加环境变量
	添加如下环境变量到文件最后，依据具体的位置添加

		export ANDROID_SDK_HOME=/opt/android-sdk-linux
		export PATH=$PATH:${ANDROID_SDK_HOME}/tools
		export PATH=$PATH:${ANDROID_SDK_HOME}/platform-tools

	保存文件并使之生效

		source  ~/.bashrc

	验证:
		重启一个终端，输入android打开打开Android SDK Manager
		输入 android
	
### 3. NDK安装 ###
	
	ijkplayer官方推荐的编译版本为NDK r10e 所以本文采用的环境也相同。

	NDK没有方便的安装方案，具体如下：

	下载方式一：
		wget -c http://dl.google.com/android/ndk/android-ndk64-r10b-linux-x86_64.tar.bz2
		sudo tar -C /opt -xvf android-ndk64-r10b-linux-x86_64.tar.bz2

	下载方式二：
		wget -c http://dl.google.com/android/ndk/android-ndk-r10e-linux-x86_64.bin
		./android-ndk-r10c-linux-x86_64.bin
		mv android-ndk-r10e /opt

	要想使用Android-NDK，还需要进行环境变量的配置：
		 gedit ~/.bashrc

		 在文件末尾添加以下内容（具体的路径为准）：

		export ANDROID_NDK=/opt/android-ndk-r10e
		export PATH=${PATH}:$ANDROID_NDK

	保存文件并使之生效

		source  ~/.bashrc

	安装并配置完成Android-NDK之后，需要进行安装验证，以确认正确安装并配置：
	
		ndk-build

### 4. 源码编译 ###

	把源代码克隆到本地
	
		git clone https://github.com/Bilibili/ijkplayer.git ijkplayer-android

	下载好后进入源代码的主目录

		cd ijkplayer-android

	切换到最新版本（到github上查看最新版本，好像不更新了）

		git checkout -B latest k0.8.8

	配置编译支持所有格式

	ijkplayer默认编译配置在config/目录下，默认使用的是module-default.sh，可以使用vim命令打开里面内容查看具体支持的播放音视频类型等。

	官方提供的四个编译脚本分别为：
	module.sh 当前编译的配置，软连接->指向下面三个中的一个(默认是module-lite.sh )
	module-default.sh：最多的编码解码支持。
	module-lite-hevc.sh：主流的编码解码支持（支持hevc），更小的体积。
	module-lite.sh：主流的编码解码支持，更小的体积。（默认编译使用的是这个脚本）
 
	以编译最全的so为例，需要执行的操作如下：

	进入到config目录

		cd config

	先打开 module-default.sh

		gedit module-default.sh

	在尾部添加下面代码，要不然会编译失败，这里是一个大坑

		export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-linux-perf"
		export COMMON_FF_CFG_FLAGS="$COMMON_FF_CFG_FLAGS --disable-bzlib"
		
	选择编译格式

		rm module.sh
		ln -s module-default.sh module.sh

	开始编译

	回到项目根目录ijkplayer-android

		cd ../../

	配置ssl，https协议, 这里下载ffmpeg和预编译内容，比较耗时，可以喝一杯咖啡先。

		./init-android-openssl.sh
		./init-android.sh

	进入到 ijkplayer-android/android/contrib

		cd android/contrib
	
	编译openssl

		./compile-openssl.sh clean
		./compile-openssl.sh all

	编译ffmeg

		./compile-ffmpeg.sh clean
		./compile-ffmpeg.sh all	

	这里的编译也比较耗时，不过等待的就是成功的喜悦啦

	结束后，编译就完成了，接下来就是生成so文件	

	生成so文件

		cd ..
		./compile-ijk.sh all

	生成过程不会太久，会在/ijkplayer-android/android/ijkplayer目录下生成我们所想要的包，生成的包还挺大的。

	每一个步骤都是比较重要的，落掉某个步骤可能就会导入生成失败，每次的编译生成过程也是比较耗时的，所以少踩点坑就可以节约很多时间。


### 4. 添加到Android工程编译 ###

	拷贝文件

	把ijkplayer-android\android\ijkplayer从CentOS中拷贝到windows中

	更改ijkplayer项目中的build.gradle
	
	Copy From
	https://blog.csdn.net/casemate/article/details/114767343


