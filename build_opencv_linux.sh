#!/bin/bash

LIB_PATH=~/prog/lib
OPENCV_PATH=$LIB_PATH/opencv
OPENCV_CONTRIB_PATH=$LIB_PATH/opencv_contrib

BUILD_TYPE=Release


function install_prerequisites() {
    cur_dir=$(pwd)
    # Generic tools
    sudo apt install -y build-essential cmake pkg-config unzip yasm git checkinstall
    
    # Image I/O libs
    sudo apt install -y libjpeg-dev libpng-dev libtiff-dev
    
    # Video/Audio Libs - FFMPEG, GSTREAMER, x264 and so on
    sudo apt install -y libavcodec-dev libavformat-dev libswscale-dev libavresample-dev
    sudo apt install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
    sudo apt install -y libxvidcore-dev x264 libx264-dev libfaac-dev libmp3lame-dev libtheora-dev 
    sudo apt install -y libfaac-dev libmp3lame-dev libvorbis-dev

    # OpenCore - Adaptive Multi Rate Narrow Band (AMRNB) and Wide Band (AMRWB) speech codec
    sudo apt install -y libopencore-amrnb-dev libopencore-amrwb-dev

    # Cameras programming interface libs
    sudo apt-get install -y libdc1394-22 libdc1394-22-dev libxine2-dev libv4l-dev v4l-utils
    cd /usr/include/linux
    sudo ln -s -f ../libv4l1-videodev.h videodev.h
    cd $cur_dir

    # GTK lib for the graphical user functionalites coming from OpenCV highghui module
    sudo apt-get install -y libgtk-3-dev

    # Parallelism library C++ for CPU
    sudo apt-get install -y libtbb-dev

    # Optimization libraries for OpenCV
    sudo apt-get install -y libatlas-base-dev gfortran

    # Optional libraries
    sudo apt-get install -y libprotobuf-dev protobuf-compiler
    sudo apt-get install -y libgoogle-glog-dev libgflags-dev
    sudo apt-get install -y libgphoto2-dev libeigen3-dev libhdf5-dev doxygen    

}


function build() {
    BUILD_PATH=$LIB_PATH/opencv-build/linux/$BUILD_TYPE
    INSTALL_PATH=$LIB_PATH/opencv-install/linux/$BUILD_TYPE
    rm -rf $BUILD_PATH $INSTALL_PATH
    mkdir -p $BUILD_PATH && cd $BUILD_PATH
    cmake -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
        -DWITH_CUDA=ON \
        -DCUDNN_INCLUDE_DIR=/usr/include \
        -DCUDNN_LIBRARY=/usr/lib/x86_64-linux-gnu/libcudnn.so.8 \
        -DBUILD_PERF_TESTS=OFF \
        -DBUILD_TESTS=OFF \
        -DBUILD_DOCS=OFF \
        -DWITH_MATLAB=OFF \
        -DOPENCV_DNN_CUDA=ON \
        -DENABLE_FAST_MATH=ON \
        -DOPENCV_EXTRA_MODULES_PATH=$OPENCV_CONTRIB_PATH/modules \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH \
        $OPENCV_PATH
    
    make -j6
    make install
}

# install_prerequisites
build

