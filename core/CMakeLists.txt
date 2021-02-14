
set(SRC_FILES
  src/scene.cpp
  src/image_loop.cpp
  src/utils.cpp
  src/view.cpp
  src/tracking/klt_tracker.cpp
  src/matching/cashash_matcher.cpp
  src/calibrator.cpp
)
message(STATUS "SRC_FILES: ${SRC_FILES}")

add_library(cvlab_core ${SRC_FILES})

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CERES_INCLUDE_DIRS})

target_link_libraries(cvlab_core PUBLIC ${OpenCV_LIBRARIES} ${CERES_LIBRARIES})

