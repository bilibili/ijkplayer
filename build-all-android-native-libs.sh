#!/bin/bash

# prepare ffmpeg and openssl codes and build all andriod native libs and JNI libs automatically
# validate ok on Ubuntu 20.04.2 x86_64 LTS

# zhou.weiguo, 2021-05-12,16:19


#modify following variable accordingly
#ndk-r14b must be used in this project but I guess clang mightbe ok
export ANDROID_NDK=/opt/android-ndk-r14b

#default release build
export build_type=release

#some files must be modified according to android/patch-debugging-with-lldb.sh for debug build
#becareful there some conflicts between release build and debug build for final Android apk
#export build_type=debug

export prj_root_path=$(pwd)

ARCHs=(armv5 armeabi-v7a arm64-v8a x86 x86_64)


function show_pwd() 
{
    echo -e "current working path:$(pwd)\n"
}


function check_ndk()
{
    if [ ! -d ${ANDROID_NDK} ]; then
        echo -e "${ANDROID_NDK} not exist, pls check...\n"
        exit 1
    fi
}


function check_sources()
{
    module_name=$1
    #TODO: should I check input args here?

    cd ${prj_root_path}
    echo "==enter function check sources of $module_name =="

    #it should be local variable in a standalone function
    #but be a global variable might be a better idea because the same variable 
    #would be defined in other place
    #ARCHs=(armv5 armeabi-v7a arm64-v8a x86 x86_64)

    notexist=0
    for index in ${ARCHs[@]};do
        if [ ${index} == "arm64-v8a" ]; then
            realname="arm64"
        elif [ ${index} == "armeabi-v7a" ]; then
            realname="armv7a"
        else
            realname=${index}
        fi
        test_path=./android/contrib/${module_name}-${realname}
        if [ ! -d $test_path ]; then
            echo "test_path $test_path not exist"
            let notexist++
        else
            echo "test_path $test_path  exist"
        fi
        let index++
    done

    if [ $notexist -eq 0 ];then
        echo "${module_name} source directory exist"
    else
        echo "${module_name} source directory not exist"

        if [ $module_name == "ffmpeg" ]; then
            echo "prepare $module_name codes..."
            ./init-android.sh
        elif [ $module_name == "openssl" ]; then
            echo "prepare $module_name codes..."
            ./init-android-openssl.sh
        else
            echo "module_name $module_name unknown, pls check why?"
        fi
    fi

    cd ${prj_root_path}
    echo "==leave function check sources of $module_name =="
    echo -e "\n"
}


function build_JNI()
{
    cd ${prj_root_path}
    show_pwd
    echo "build JNI..."
    #ARCHs=(armv5 armeabi-v7a arm64-v8a x86 x86_64)

    for index in ${ARCHs[@]};do
        echo "arch = ${index}"
        if [ ${index} == "arm64-v8a" ]; then
            realname="arm64"
        elif [ ${index} == "armeabi-v7a" ]; then
            realname="armv7a"
        else
            realname=${index}
        fi

        if [ ! -d ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jni ]; then
            echo "${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jni not exist"
            mkdir -p ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jni
        else
            echo "${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jni already exist"
        fi
        cd ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jni

        show_pwd
        ${ANDROID_NDK}/ndk-build APP_BUILD_SCRIPT=${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/src/main/jni/Android.mk NDK_APPLICATION_MK=${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/src/main/jni/Application.mk APP_ABI=${index} NDK_ALL_ABIS=${index} NDK_DEBUG=1 APP_PLATFORM=android-21 NDK_OUT=${prj_root_path}/android/ijkplayer/ijkplayer-${realnaem}/build/intermediates/ndkBuild/debug/obj NDK_LIBS_OUT=${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/build/intermediates/ndkBuild/debug/lib NDK_LOG=1 V=1 clean
        ${ANDROID_NDK}/ndk-build APP_BUILD_SCRIPT=${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/src/main/jni/Android.mk NDK_APPLICATION_MK=${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/src/main/jni/Application.mk APP_ABI=${index} NDK_ALL_ABIS=${index} NDK_DEBUG=1 APP_PLATFORM=android-21 NDK_OUT=${prj_root_path}/android/ijkplayer/ijkplayer-${realnaem}/build/intermediates/ndkBuild/debug/obj NDK_LIBS_OUT=${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/build/intermediates/ndkBuild/debug/lib NDK_LOG=1 V=1

        if [ ! -d ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index} ]; then
            echo -e "${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index} not exist"
            mkdir -p ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index}
        else
            echo -e "${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index} already exist"
        fi
        echo -e "/bin/cp -fv ${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/build/intermediates/ndkBuild/debug/lib/${index}/libijksdl.so  ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index}"
        /bin/cp -fv ${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/build/intermediates/ndkBuild/debug/lib/${index}/libijksdl.so  ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index}


        echo -e "/bin/cp -fv ${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/build/intermediates/ndkBuild/debug/lib/${index}/libijkplayer.so  ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index}"
        /bin/cp -fv ${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/build/intermediates/ndkBuild/debug/lib/${index}/libijkplayer.so  ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index}

        let index++
    done
}


check_ndk


show_pwd
check_sources  "openssl"
show_pwd


show_pwd
check_sources  "ffmpeg"
show_pwd


cd ${prj_root_path}/android/contrib
show_pwd
echo "build openssl..."
./compile-openssl.sh all


cd ${prj_root_path}/android/contrib
show_pwd
echo "build ffmpeg..."
./compile-ffmpeg.sh all


if [ ${build_type} == "debug" ]; then
    build_JNI
else
    echo "build ijksdl.so ijkplayer.so..."
    cd ${prj_root_path}/android
    show_pwd
    ./compile-ijk.sh all
fi


cd ${prj_root_path}
show_pwd
echo "all dependent android native libs and JNI libs build finished, pls build ijkplayer APK via latest Andriod Studio IDE"
