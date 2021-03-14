#!/bin/bash

set -e

# JAVA_HOME=/usr/lib/jvm/java-14-oracle
JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
ANDROID_SDK_PATH=~/Android/Sdk
ANDROID_NDK_PATH=$ANDROID_SDK_PATH/ndk/21.3.6528147
# ANDROID_NDK_PATH=$ANDROID_SDK_PATH/ndk/23.0.7123448/
MIN_SDK_VERSION=24
export ANDROID_HOME=$ANDROID_SDK_PATH
export ANDROID_NDK_HOME=$ANDROID_NDK_PATH

LIB_PATH=~/prog/lib
SRC_PATH=$LIB_PATH/ceres-solver
BUILD_ROOT_PATH=$LIB_PATH/ceres-build/android
INSTALL_ROOT_PATH=$LIB_PATH/ceres-install/android
CERES_VERSION=2.0.0
EIGEN_INSTALL_PATH=$LIB_PATH/eigen-install/linux/3.3.9/Release

# BUILD_TYPE=Debug
BUILD_TYPE=Release


function build() {
    ABI=$1
    BUILD_PATH=$BUILD_ROOT_PATH/$CERES_VERSION/$BUILD_TYPE/$ABI
    INSTALL_PATH=$INSTALL_ROOT_PATH/$CERES_VERSION/$BUILD_TYPE/$ABI
    mkdir -p $BUILD_PATH && cd $BUILD_PATH
    cmake -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
        -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_PATH/build/cmake/android.toolchain.cmake \
        -DANDROID_SDK="${ANDROID_SDK_PATH}" \
        -DANDROID_NDK="${ANDROID_NDK_PATH}" \
        -DANDROID_ABI=$ABI \
        -DANDROID_NATIVE_API_LEVEL=$MIN_SDK_VERSION \
        -DANDROID_TOOLCHAIN=clang \
        -DANDROID_STL=c++_shared \
        -DEigen3_DIR=$EIGEN_INSTALL_PATH/share/eigen3/cmake \
        -DMINIGLOG=ON \
        -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH \
        $SRC_PATH
    
    make -j6
    make install/strip
}

cd $SRC_PATH && git checkout $CERES_VERSION

build "armeabi-v7a"
build "arm64-v8a"
build "x86"
build "x86_64"

