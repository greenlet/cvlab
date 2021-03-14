#!/bin/bash

set -e

LIB_PATH=~/prog/lib
EIGEN_PATH=$LIB_PATH/eigen
EIGEN_VERSION=3.3.9

BUILD_TYPE=Release


function build() {
    BUILD_PATH=$LIB_PATH/eigen-build/linux/$EIGEN_VERSION/$BUILD_TYPE
    INSTALL_PATH=$LIB_PATH/eigen-install/linux/$EIGEN_VERSION/$BUILD_TYPE
    rm -rf $BUILD_PATH $INSTALL_PATH
    mkdir -p $BUILD_PATH && cd $BUILD_PATH
    cmake -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH \
        $EIGEN_PATH
    
    make -j6
    make install
}

# install_prerequisites

cd $EIGEN_PATH && git checkout $EIGEN_VERSION

build

