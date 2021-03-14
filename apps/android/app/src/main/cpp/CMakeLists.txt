# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(android-lib android)
find_library(log-lib log)
find_library(camera-lib camera2ndk)
find_library(media-lib mediandk)
#find_library(gl-lib GLESv2)
find_library(gl-lib GLESv3)
#find_library(egl-lib EGL)

set(Eigen3_DIR "${EIGEN3_PATH}/share/eigen3/cmake")
message(STATUS "Eigen3_DIR: ${Eigen3_DIR}")
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIRS})

set(Ceres_DIR "${CERES_PATH}/${ANDROID_ABI}/lib/cmake/Ceres")
message(STATUS "Ceres_DIR: ${Ceres_DIR}")
find_package(Ceres REQUIRED)
include_directories(${Ceres_INCLUDE_DIRS})

set(OpenCV_DIR "${OPENCV_SDK_PATH}/native/jni/abi-${ANDROID_ABI}")
message(STATUS "OpenCV_DIR: ${OpenCV_DIR}")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


set(CPP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../../../../../)
set(EIGEN_SRC_DIR ${CPP_SOURCE_DIR}/external/eigen)
include_directories(${EIGEN_SRC_DIR})

include_directories(${CPP_SOURCE_DIR}/core/src)


add_library(cvlab-core STATIC IMPORTED)
set_target_properties(cvlab-core PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../../../libs/${CMAKE_BUILD_TYPE}/${ANDROID_ABI}/libcvlab_core.a)


add_library(cvlab-android SHARED
        cvlab_interface.cpp
        cvlab_app.cpp
        camera_android.cpp
        logger.cpp
        gl_program.cpp
        renderer.cpp
)

target_link_libraries(cvlab-android
        ${android-lib}
        ${log-lib}
        ${camera-lib}
        ${media-lib}
        ${gl-lib}
#        ${egl-lib}
        ${OpenCV_LIBRARIES}
        cvlab-core
        )


get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
