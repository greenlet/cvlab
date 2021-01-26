
set(SRC_FILES main.cpp)

add_executable(cvlab_calibration ${SRC_FILES})
target_link_libraries(cvlab_calibration cvlab_core)

