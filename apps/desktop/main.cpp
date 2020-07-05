#include <iostream>

#include "utils.h"

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

  CVImageLoop(file_name, [](const cv::Mat &frame) {
    cv::imshow("frame", frame);
  }, delay_ms);

  return 0;
}
