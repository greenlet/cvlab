#include <iostream>

#include "utils.h"
#include "image_loop.h"
#include "view.h"
#include "scene.h"

int main(int argc, char **argv) {
  std::cout << "Starting Motion 3d" << std::endl;

  if (argc != 2 and argc != 3) {
    std::cout << "Usage: " << argv[0] << " <path-to-video-file> <delay-ms>?"
              << std::endl;
    return 1;
  }

  std::string file_name(argv[1]);
  std::cout << "Opening file: " << file_name << std::endl;

  int delay_ms = 0;
  if (argc == 3) {
    delay_ms = atoi(argv[2]);
  }


  Scene scene;
  CVImageLoop(file_name, [&scene](cv::Mat img) {
    ViewPtr view = scene.processImage(img);
    // std::cout << "View: " << view->id() << ". Keypoints: " << view->keypoints().size() << std::endl;
    // cv::Mat img_vis = view->visualizeKeypoints();

    const CVKeyPoints &keypoints = scene.klt_tracker().tracked_keypoints();
    cv::Mat img_vis = view->visualizeKeypoints(keypoints);

    cv::imshow("keypoints", img_vis);
  }, delay_ms);

  return 0;
}
