#!/bin/bash

set -e

LIB_PATH=~/prog/lib
CERES_PATH=$LIB_PATH/ceres-solver
CERES_VERSION=2.0.0
EIGEN_INSTALL_PATH=$LIB_PATH/eigen-install/linux/3.3.9/Release

BUILD_TYPE=Release

function install_prerequisites() {
    cur_dir=$(pwd)

    sudo apt-get install -y cmake
    # google-glog + gflags
    sudo apt-get install -y libgoogle-glog-dev libgflags-dev
    # BLAS & LAPACK
    sudo apt-get install -y libatlas-base-dev
    
    # Eigen3
    # sudo apt-get install -y libeigen3-dev
    
    # SuiteSparse and CXSparse (optional)
    sudo apt-get install -y libsuitesparse-dev
}

function build() {
    BUILD_PATH=$LIB_PATH/ceres-build/linux/$CERES_VERSION/$BUILD_TYPE
    INSTALL_PATH=$LIB_PATH/ceres-install/linux/$CERES_VERSION/$BUILD_TYPE
    rm -rf $BUILD_PATH $INSTALL_PATH
    mkdir -p $BUILD_PATH && cd $BUILD_PATH
    cmake -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH \
        -DEigen3_DIR=$EIGEN_INSTALL_PATH/share/eigen3/cmake \
        $CERES_PATH
    
    make -j6
    make install
}


# install_prerequisites

cd $CERES_PATH && git checkout $CERES_VERSION

build

