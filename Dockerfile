# Android development environment based on Ubuntu 14.04 LTS.
# version 0.0.1

# Start with Ubuntu 14.04 LTS.
FROM phusion/baseimage

MAINTAINER Brian Prodoehl <bprodoehl@connectify.me>

# Never ask for confirmations
ENV DEBIAN_FRONTEND noninteractive
RUN echo "debconf shared/accepted-oracle-license-v1-1 select true" | debconf-set-selections
RUN echo "debconf shared/accepted-oracle-license-v1-1 seen true" | debconf-set-selections

# First, install add-apt-repository and bzip2
RUN apt-get update
RUN apt-get -y install software-properties-common python-software-properties bzip2 unzip openssh-client git lib32stdc++6 lib32z1 expect yasm make

# Add oracle-jdk7 to repositories
RUN add-apt-repository ppa:webupd8team/java

# Update apt
RUN apt-get update

# Install oracle-jdk7
RUN apt-get -y install oracle-java7-installer

# Install android sdk
RUN wget http://dl.google.com/android/android-sdk_r23-linux.tgz
RUN tar -xvzf android-sdk_r23-linux.tgz
RUN mv android-sdk-linux /usr/local/android-sdk
RUN rm android-sdk_r23-linux.tgz

# Install Android tools
ENV PATH ${PATH}:/usr/local/android-sdk/tools
COPY tools/android-accept-licenses.sh /usr/local/android-sdk/tools
RUN ls -laih /usr/local/android-sdk/tools
RUN ["android-accept-licenses.sh", "android update sdk --all --no-ui --filter platform-tools,tools,build-tools-21,build-tools-21.0.1,build-tools-21.0.2,build-tools-21.1,build-tools-21.1.1,build-tools-21.1.2,build-tools-22,build-tools-22.0.1,android-19,android-20,android-21,android-22,addon-google_apis_x86-google-21,extra-android-support,extra-android-m2repository,extra-google-m2repository,extra-google-google_play_services,sys-img-armeabi-v7a-android-21"]

# Install Android NDK
RUN wget http://dl.google.com/android/ndk/android-ndk-r10e-linux-x86_64.bin
RUN chmod a+x android-ndk-r10e-linux-x86_64.bin
RUN ./android-ndk-r10e-linux-x86_64.bin
RUN mv android-ndk-r10e /usr/local/android-ndk
RUN rm android-ndk-r10e-linux-x86_64.bin

# Install Gradle
RUN wget https://downloads.gradle.org/distributions/gradle-2.9-bin.zip
RUN unzip gradle-2.9-bin.zip
RUN mv gradle-2.9 /usr/local/gradle
RUN rm gradle-2.9-bin.zip

# get ijkplayer sources
RUN mkdir /player-sources
RUN cd /player-sources && git clone https://github.com/yeyus/ijkplayer.git

# Environment variables
ENV ANDROID_HOME /usr/local/android-sdk
ENV ANDROID_SDK_HOME $ANDROID_HOME
ENV ANDROID_NDK_HOME /usr/local/android-ndk
ENV ANDROID_SDK $ANDROID_SDK_HOME
ENV ANDROID_NDK $ANDROID_NDK_HOME
ENV GRADLE_HOME /usr/local/gradle
ENV PATH $PATH:$ANDROID_SDK_HOME/tools
ENV PATH $PATH:$ANDROID_SDK_HOME/platform-tools
ENV PATH $PATH:$ANDROID_NDK_HOME
ENV PATH $PATH:$GRADLE_HOME/bin

# Export JAVA_HOME variable
ENV JAVA_HOME /usr/lib/jvm/java-7-oracle

RUN apt-get clean

WORKDIR /player-sources
