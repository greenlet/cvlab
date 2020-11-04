#!/bin/bash

# JAVA_HOME=/usr/lib/jvm/java-14-oracle
JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
ANDROID_SDK_PATH=/home/misha/Android/Sdk
ANDROID_NDK_PATH=$ANDROID_SDK_PATH/ndk/21.3.6528147
MIN_SDK_VERSION=24
export ANDROID_HOME=$ANDROID_SDK_PATH
export ANDROID_NDK_HOME=$ANDROID_NDK_PATH

SRC_PATH=/home/misha/prog/lib/opencv
CONTRIB_SRC_PATH=/home/misha/prog/lib/opencv_contrib
BUILD_ROOT_PATH=/home/misha/prog/lib/opencv-build/android
INSTALL_ROOT_PATH=/home/misha/prog/lib/opencv-install/android

# BUILD_TYPE=Debug
BUILD_TYPE=Release


function build() {
    ABI=$1
    BUILD_PATH=$BUILD_ROOT_PATH/$BUILD_TYPE/$ABI
    INSTALL_PATH=$INSTALL_ROOT_PATH/$BUILD_TYPE
    mkdir -p $BUILD_PATH && cd $BUILD_PATH
    cmake -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
        -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_PATH/build/cmake/android.toolchain.cmake \
        -DANDROID_SDK="${ANDROID_SDK_PATH}" \
        -DANDROID_NDK="${ANDROID_NDK_PATH}" \
        -DANDROID_ABI=$ABI \
        -DANDROID_NATIVE_API_LEVEL=$MIN_SDK_VERSION \
        -DANDROID_TOOLCHAIN=clang \
        -DANDROID_STL=c++_static \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DWITH_CUDA=OFF \
        -DWITH_MATLAB=OFF \
        -DBUILD_ANDROID_EXAMPLES=OFF \
        -DBUILD_DOCS=OFF \
        -DBUILD_PERF_TESTS=OFF \
        -DBUILD_TESTS=OFF \
        -DWITH_IPP=ON \
        -DOPENCV_EXTRA_MODULES_PATH=$CONTRIB_SRC_PATH/modules  \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH \
        $SRC_PATH
    
    make -j6
    make install/strip
}


build "armeabi-v7a"
build "arm64-v8a"
build "x86"
build "x86_64"



