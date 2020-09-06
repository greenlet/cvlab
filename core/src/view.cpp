#include "view.h"
#include "utils.h"

View::View(int id, cv::Mat img) : id_(id), img_(img) {}

const CVKeyPoints& View::calcKeypoints() {
  if (keypoints_.size() == 0) {
    cv::Ptr<cv::ORB> orb = cv::ORB::create();
    orb->detectAndCompute(img_, cv::noArray(), keypoints_, descriptors_);
  }
  return keypoints_;
}

cv::Mat View::visualizeKeypoints(const CVKeyPoints& keypoints) {
  cv::Mat res;
  if (!img_.empty()) {
    cv::drawKeypoints(img_, keypoints, res);
  }
  return res;
}

cv::Mat View::visualizeKeypoints() { return visualizeKeypoints(keypoints_); }

const CVMats& View::calcPyramid() {
  if (pyramid_.size() == 0) {
    calcGrayscale();
    cv::buildOpticalFlowPyramid(img_grayscale_, pyramid_, cv::Size(21, 21), 3);
  }
  return pyramid_;
}

const cv::Mat& View::calcGrayscale() {
  if (!img_grayscale_.empty()) {
    return img_grayscale_;
  }
  cv::cvtColor(img_, img_grayscale_, cv::COLOR_BGR2GRAY);
  return img_grayscale_;
}

ViewKeyPointId::ViewKeyPointId(ViewId view_id, KeyPointId keypoint_id)
    : view_id_(view_id), keypoint_id_(keypoint_id) {
  id_ = (view_id_ << 14) + keypoint_id_;
}

