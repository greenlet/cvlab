#!/bin/bash

set -e

ANDROID_SDK_PATH=/home/misha/Android/Sdk
ANDROID_NDK_PATH=$ANDROID_SDK_PATH/ndk/21.3.6528147
MIN_SDK_VERSION=24


# BUILD_TYPE=Debug
BUILD_TYPE=Release
LIB_PATH=/home/misha/prog/lib
SRC_PATH=$(pwd)
OPENCV_VERSION=4.5.1
OPENCV_BUILD_TYPE=Release
OPENCV_ANDROID_SDK_PATH=$LIB_PATH/opencv-install/android/$OPENCV_VERSION/$OPENCV_BUILD_TYPE/sdk
Eigen3_DIR=$LIB_PATH/eigen-install/linux/3.3.9/Release/share/eigen3/cmake


function build() {
    ABI=$1
    BUILD_PATH=$SRC_PATH/build-android/$BUILD_TYPE/$ABI
    OUT_LIB_PATH=$SRC_PATH/apps/android/app/libs/$BUILD_TYPE/$ABI
    Ceres_DIR=$LIB_PATH/ceres-install/android/2.0.0/Release/$ABI/lib/cmake/Ceres
    mkdir -p $BUILD_PATH && cd $BUILD_PATH
    cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_PATH/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=$ABI \
        -DANDROID_NATIVE_API_LEVEL=$MIN_SDK_VERSION \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -D Eigen3_DIR=$Eigen3_DIR \
        -D Ceres_DIR=$Ceres_DIR \
        -DOPENCV_ANDROID_SDK_PATH=$OPENCV_ANDROID_SDK_PATH \
        -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=$OUT_LIB_PATH \
        -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=$OUT_LIB_PATH \
        $SRC_PATH
    make cvlab_core -j6
    cd $SRC_PATH
}


build "armeabi-v7a"
build "arm64-v8a"
build "x86"
build "x86_64"

