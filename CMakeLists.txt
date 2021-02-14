cmake_minimum_required(VERSION 3.16.0)
project(CVLab)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(ANDROID)
    set(OpenCV_DIR "${OPENCV_ANDROID_SDK_PATH}/native/jni/abi-${ANDROID_ABI}")
    message(STATUS "OpenCV_DIR: ${OpenCV_DIR}")
endif()

find_package(OpenCV REQUIRED)
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")

find_package(Ceres REQUIRED)

include_directories(external/eigen)

include_directories(${PROJECT_SOURCE_DIR}/core/src)
add_subdirectory(core)
add_subdirectory(apps)

# get_cmake_property(_variableNames VARIABLES)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()
