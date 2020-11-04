
set(SRC_FILES main.cpp)

add_executable(cvlab_tracking ${SRC_FILES})
target_link_libraries(cvlab_tracking cvlab_core)

