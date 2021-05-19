#!/bin/bash

# prepare ffmpeg and openssl codes and build all andriod native libs and JNI libs automatically
# validate ok on Ubuntu 20.04.2 x86_64 LTS

# zhou.weiguo, 2021-05-12,16:19

export TEXT_BLACK=" \033[30m "
export TEXT_RED="   \033[31m "
export TEXT_GREEN=" \033[32m "
export TEXT_BLUE="  \033[34m "
export TEXT_BLACK=" \033[35m "
export TEXT_WHITE=" \033[37m "
export TEXT_RESET=" \033[0m  "

export build_user=$(whoami)
export build_time=`date +"%Y-%m-%d,%H:%M:%S"`
export prj_root_path=$(pwd)
export home_path=`env | grep ^HOME= | cut -c 6-`

#modify following variable accordingly
#ndk-r14b must be used in this project but I guess clang mightbe ok
export ANDROID_NDK=/opt/android-ndk-r14b

#default release build
export build_type=release
#some files must be modified according to android/patch-debugging-with-lldb.sh for debug build
#becareful there some conflicts between release build and debug build for final Android apk
#export build_type=debug

#ARCHs=(armv5 armeabi-v7a arm64-v8a x86 x86_64)
#TODO: target "armv5 armeabi-v7a x86" not working with OpenSSL_1_1_1-stable
ARCHs=(arm64-v8a x86_64)


function show_pwd() 
{
    echo -e "current working path:$(pwd)\n"
}


function check_ndk()
{
    if [ ! -d ${ANDROID_NDK} ]; then
        echo -e "${TEXT_RED}${ANDROID_NDK} not exist, pls check...\n${TEXT_RESET}"
        exit 1
    fi
}


function dump_envs()
{
    echo -e "\n"

    echo "build_user:    $build_user"
    echo "build_time:    $build_time"
    echo "build_type:    $build_type"
    echo "home_path:     $home_path"
    echo "prj_root_path: $prj_root_path"
    echo "ANDROID_NDK:   $ANDROID_NDK"

    echo -e "\n"
}


function dump_usage()
{
    echo "Usage:"
    echo "  $0 clean"
    echo "  $0 build"
    exit 1
}


function echo_left_align()
{
    if [ ! $# -eq 2 ]; then
        echo "params counts should be 2"
        exit 1
    fi

    index=0
    max_length=40
    l_string=$1
    r_string=$2

    length=${#l_string}
    echo -e "${TEXT_BLUE}$l_string${TEXT_RESET}\c"
    if [ $length -lt $max_length ]; then
        padding_length=`expr $max_length - $length`
        for (( tmp=0; tmp < $padding_length; tmp++ ))
            do
               echo -e " \c"
            done
    fi

    echo $r_string
}


function check_sources()
{
    module_name=$1
    #TODO: should I check input args here?

    cd ${prj_root_path}
    echo "======enter function check sources of $module_name ======"

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
            echo_left_align "$test_path" "not exist"
            let notexist++
        else
            echo_left_align "$test_path" "exist"
        fi
        let index++
    done

    if [ $notexist -eq 0 ];then
        echo_left_align "${module_name} source directory" "exist"
    else
        echo -e "${TEXT_RED}${module_name} source directory not exist${TEXT_RESET}"

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
    echo "======leave function check sources of $module_name ======"
    echo -e "\n"
}


function build_native_debug()
{
    set -e
    cd ${prj_root_path}
    show_pwd
    echo "build JNI..."

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
            echo -e "${TEXT_RED}${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index} not exist${TEXT_RESET}"
            mkdir -p ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index}
        else
            echo -e "${TEXT_BLUE}${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index} already exist${TEXT_RESET}"
        fi
        echo -e "/bin/cp -fv ${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/build/intermediates/ndkBuild/debug/lib/${index}/libijksdl.so  ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index}"
        /bin/cp -fv ${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/build/intermediates/ndkBuild/debug/lib/${index}/libijksdl.so  ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index}


        echo -e "/bin/cp -fv ${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/build/intermediates/ndkBuild/debug/lib/${index}/libijkplayer.so  ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index}"
        /bin/cp -fv ${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/build/intermediates/ndkBuild/debug/lib/${index}/libijkplayer.so  ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index}

        let index++
    done
    set +e
}


function build_native_release()
{
    cd ${prj_root_path}
    show_pwd
    echo "build JNI..."

    for index in ${ARCHs[@]};do
        echo "arch = ${index}"
        if [ ${index} == "arm64-v8a" ]; then
            realname="arm64"
        elif [ ${index} == "armeabi-v7a" ]; then
            realname="armv7a"
        else
            realname=${index}
        fi

        cd ${prj_root_path}/android/contrib
        show_pwd
        echo "build openssl..."
        ./compile-openssl.sh $realname
        if [ $? != 0 ]; then
            echo -e "${TEXT_RED} build openssl failed ${TEXT_RESET}"
            exit 1
        else
            echo -e "${TEXT_BLUE} build openssl successed ${TEXT_RESET}"
        fi

        echo "build ffmpeg..."
        ./compile-ffmpeg.sh  $realname
        if [ $? != 0 ]; then
            echo -e "${TEXT_RED}build ffmpeg failed ${TEXT_RESET}"
            exit 1
        else
            echo -e "${TEXT_BLUE} build ffmpeg successed ${TEXT_RESET}"
        fi

        echo "build ijksdl.so ijkplayer.so..."
        cd ${prj_root_path}/android
        show_pwd
        ./compile-ijk.sh $realname
        if [ $? != 0 ]; then
            echo -e "${TEXT_RED}build ijkplayer.so ijksdl.so failed ${TEXT_RESET}"
            exit 1
        else
            echo -e "${TEXT_BlUE}build ijkplayer.so ijksdl.so successed ${TEXT_RESET}"
        fi


        echo -e "\n"
        if [ ! -d ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs ]; then
            echo -e "${TEXT_RED}${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs not exist${TEXT_RESET}"
            mkdir -p ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs
        else
            echo -e "${TEXT_BLUE}${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jni already exist${TEXT_RESET}"
        fi
        cd ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs
        show_pwd
        ls -l ${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/src/main/libs/${index}/libijk*.so
        echo "/bin/cp -fv ${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/src/main/libs/${index}/libijk*.so  ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index}"
        /bin/cp -fv ${prj_root_path}/android/ijkplayer/ijkplayer-${realname}/src/main/libs/${index}/libijk*.so  ${prj_root_path}/android/ijkplayer/ijkplayer-example/src/main/jniLibs/${index}

        let index++
    done

    echo -e "\n"
}


function exec_clean()
{
    cd ${prj_root_path}
    show_pwd

    rm -rf android/contrib/build/*

    for index in ${ARCHs[@]};do
        echo "arch = ${index}"
        if [ ${index} == "arm64-v8a" ]; then
            realname="arm64"
        elif [ ${index} == "armeabi-v7a" ]; then
            realname="armv7a"
        else
            realname=${index}
        fi

        cd ${prj_root_path}/android/contrib
        show_pwd
        if [ -f ffmpeg-${realname}/config.h ];then
            echo -e "${TEXT_BLUE} ffmpeg-${realname}/config.h exist${TEXT_RESET}\n"
            rm -f ffmpeg-${realname}/config.h
        else
            echo -e "${TEXT_RED} ffmpeg-${realname}/config.h not exist${TEXT_RESET}\n"
        fi
    done
}


function exec_build()
{
    if [ ${build_type} = "release" ];then
        build_native_release
    elif [ ${build_type} = "debug" ];then
        build_native_debug
    else
        echo -e "${TEXT_RED} $build_type unknown,pls check${TEXT_RESET}"
        exit 1
    fi

    cd ${prj_root_path}
    show_pwd
    echo -e "${TEXT_BLUE} all dependent android native libs and JNI libs build finished, pls build ijkplayer APK via latest Andriod Studio IDE ${TEXT_RESET}"
}



function main()
{
case "$user_command" in
    clean)
        exec_clean
    ;;
    build)
        exec_build
    ;;
    *)
        dump_usage
        exit 1
    ;;
esac
}


user_command=$1

dump_envs
check_ndk
check_sources  "openssl"
check_sources  "ffmpeg"

main
