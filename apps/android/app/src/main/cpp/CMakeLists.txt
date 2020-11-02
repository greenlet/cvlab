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

set(OpenCV_DIR "${OPENCV_SDK_PATH}/native/jni/abi-${ANDROID_ABI}")
message(STATUS "OpenCV_DIR: ${OpenCV_DIR}")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


set(CPP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../../../../../)
set(EIGEN_SRC_DIR ${CPP_SOURCE_DIR}/external/eigen)
include_directories(${EIGEN_SRC_DIR})

include_directories(${CPP_SOURCE_DIR}/core/src)


add_library(motion3d-core STATIC IMPORTED)
set_target_properties(motion3d-core PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../../../libs/${CMAKE_BUILD_TYPE}/${ANDROID_ABI}/libmotion3d_core.a)


add_library(motion3d-android SHARED
        motion3d_interface.cpp
        motion3d_app.cpp
        camera_android.cpp
        logger.cpp
        gl_program.cpp
        renderer.cpp
)

target_link_libraries(motion3d-android
        ${android-lib}
        ${log-lib}
        ${camera-lib}
        ${media-lib}
        ${gl-lib}
#        ${egl-lib}
        ${OpenCV_LIBRARIES}
        motion3d-core
        )


get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
