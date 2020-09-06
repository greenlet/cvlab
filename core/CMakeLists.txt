
set(SRC_FILES
  src/scene.cpp
  src/image_loop.cpp
  src/utils.cpp
  src/view.cpp
  src/tracking/klt_tracker.cpp
  src/tracking/cashash_tracker.cpp
)
message(STATUS "SRC_FILES: ${SRC_FILES}")

add_library(motion3d_core ${SRC_FILES})

include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(motion3d_core ${OpenCV_LIBRARIES})

