#!/bin/bash


ANDROID_SDK_PATH=/home/misha/Android/Sdk
ANDROID_NDK_PATH=$ANDROID_SDK_PATH/ndk/21.3.6528147
MIN_SDK_VERSION=24

#OPENCV_ANDROID_SDK_PATH=/home/misha/prog/lib/OpenCV-android-sdk/sdk
OPENCV_ANDROID_SDK_PATH=/home/misha/prog/lib/opencv-install/android/Release/sdk

BUILD_TYPE=Release
SRC_PATH=$(pwd)


function build() {
    ABI=$1
    BUILD_PATH=$SRC_PATH/build-android/$BUILD_TYPE/$ABI
    OUT_LIB_PATH=$SRC_PATH/apps/android/app/libs/$ABI
    mkdir -p $BUILD_PATH && cd $BUILD_PATH
    cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_PATH/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=$ABI \
        -DANDROID_NATIVE_API_LEVEL=$MIN_SDK_VERSION \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DOPENCV_ANDROID_SDK_PATH=$OPENCV_ANDROID_SDK_PATH \
        -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=$OUT_LIB_PATH \
        -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=$OUT_LIB_PATH \
        $SRC_PATH
    make motion3d_core -j6
    cd $SRC_PATH
}


build "armeabi-v7a"
build "arm64-v8a"
build "x86"
build "x86_64"

