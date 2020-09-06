
set(SRC_FILES main.cpp)

add_executable(motion3d ${SRC_FILES})
target_link_libraries(motion3d motion3d_core)

