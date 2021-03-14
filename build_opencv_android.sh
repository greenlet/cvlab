#!/bin/bash

set -e

# JAVA_HOME=/usr/lib/jvm/java-14-oracle
JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
ANDROID_SDK_PATH=/home/misha/Android/Sdk
ANDROID_NDK_PATH=$ANDROID_SDK_PATH/ndk/21.3.6528147
MIN_SDK_VERSION=24
export ANDROID_HOME=$ANDROID_SDK_PATH
export ANDROID_NDK_HOME=$ANDROID_NDK_PATH

LIB_PATH=/home/misha/prog/lib
SRC_PATH=$LIB_PATH/opencv
CONTRIB_SRC_PATH=$LIB_PATH/opencv_contrib
BUILD_ROOT_PATH=$LIB_PATH/opencv-build/android
INSTALL_ROOT_PATH=$LIB_PATH/opencv-install/android
OPENCV_VERSION=4.5.1
Eigen3_DIR=$LIB_PATH/eigen-install/linux/3.3.9/Release/share/eigen3/cmake

# BUILD_TYPE=Debug
BUILD_TYPE=Release


function build() {
    ABI=$1
    BUILD_PATH=$BUILD_ROOT_PATH/$OPENCV_VERSION/$BUILD_TYPE/$ABI
    INSTALL_PATH=$INSTALL_ROOT_PATH/$OPENCV_VERSION/$BUILD_TYPE
    Ceres_DIR=$LIB_PATH/ceres-install/android/2.0.0/Release/$ABI/lib/cmake/Ceres
    mkdir -p $BUILD_PATH && cd $BUILD_PATH
    cmake -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
        -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_PATH/build/cmake/android.toolchain.cmake \
        -DANDROID_SDK="${ANDROID_SDK_PATH}" \
        -DANDROID_NDK="${ANDROID_NDK_PATH}" \
        -DANDROID_ABI=$ABI \
        -DANDROID_NATIVE_API_LEVEL=$MIN_SDK_VERSION \
        -DANDROID_TOOLCHAIN=clang \
        -DANDROID_STL=c++_shared \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DWITH_CUDA=OFF \
        -DWITH_MATLAB=OFF \
        -DBUILD_ANDROID_EXAMPLES=OFF \
        -DBUILD_DOCS=OFF \
        -DBUILD_PERF_TESTS=OFF \
        -DBUILD_TESTS=OFF \
        -DWITH_IPP=ON \
        -DOPENCV_EXTRA_MODULES_PATH=$CONTRIB_SRC_PATH/modules  \
        -DEigen3_DIR=$Eigen3_DIR \
        -DCeres_DIR=$Ceres_DIR \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH \
        $SRC_PATH
    
    make -j6
    make install/strip
}

cd $SRC_PATH && git checkout $OPENCV_VERSION
cd $CONTRIB_SRC_PATH && git checkout $OPENCV_VERSION

build "armeabi-v7a"
build "arm64-v8a"
build "x86"
build "x86_64"

