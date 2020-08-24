
set(SRC_FILES
  scene.cpp
  utils.cpp
  view.cpp
  tracking/klt_tracker.cpp
  tracking/cashash_tracker.cpp
)
message(STATUS "SRC_FILES: ${SRC_FILES}")

add_library(m3d_core ${SRC_FILES})
target_link_libraries(m3d_core ${OpenCV_LIBRARIES})
target_link_libraries(m3d_core Eigen3::Eigen)

