#include "utils.h"

CVImageProcessorImpl::CVImageProcessorImpl(CVImageCallback callback)
    : callback_(callback) {}

void CVImageProcessorImpl::process(cv::Mat frame) { callback_(frame); }

CVImageLoop::CVImageLoop(const std::string &file_name, CVImageCallback callback,
                         int delay_ms)
    : CVImageLoop(file_name, std::make_shared<CVImageProcessorImpl>(callback),
                  delay_ms) {}

CVImageLoop::CVImageLoop(const std::string &file_name,
                         std::shared_ptr<CVImageProcessor> processor,
                         int delay_ms)
    : file_name_(file_name), processor_(processor), delay_ms_(delay_ms) {
  auto cap = cv::VideoCapture(file_name_, cv::CAP_ANY);
  cv::Mat frame;
  while (cap.isOpened()) {
    if (!cap.read(frame)) {
      continue;
    }
    processor_->process(frame);

    int pressed_key = cv::waitKey(delay_ms_);
    if (pressed_key == 'q' || pressed_key == 'Q') {
      break;
    }
  }
}

CVPoints2f KeyPointsToPoints2f(const CVKeyPoints &keypoints) {
  CVPoints2f points(keypoints.size());
  for (size_t i = 0; i < keypoints.size(); i++) {
    points[i] = keypoints[i].pt;
  }
  return points;
}
