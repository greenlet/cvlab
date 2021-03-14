cmake_minimum_required(VERSION 3.16.0)
project(CVLab)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(ANDROID)
    set(OpenCV_DIR "${OPENCV_ANDROID_SDK_PATH}/native/jni/abi-${ANDROID_ABI}")
    message(STATUS "OpenCV_DIR: ${OpenCV_DIR}")
endif()

# include_directories(external/eigen)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIRS})
message(STATUS "Eigen3 include dirs: ${EIGEN3_INCLUDE_DIRS}")

find_package(Ceres REQUIRED)
if (NOT DEFINED ${Ceres_INCLUDE_DIRS})
    set(Ceres_INCLUDE_DIRS ${Ceres_DIR}/../../../include)
endif()
include_directories(${Ceres_INCLUDE_DIRS})
message(STATUS "Ceres include dirs: ${Ceres_INCLUDE_DIRS}")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")


include_directories(${PROJECT_SOURCE_DIR}/core/src)
add_subdirectory(core)
add_subdirectory(apps)


target_link_libraries(cvlab_core PUBLIC ${OpenCV_LIBRARIES} ${CERES_LIBRARIES})


# get_cmake_property(_variableNames VARIABLES)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

