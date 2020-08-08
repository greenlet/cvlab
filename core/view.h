#pragma once

#include "common.h"

using ViewId = int;
using KeyPointId = int;

class View {
 public:
  View(int id, cv::Mat img);

  const CVKeyPoints& calcKeypoints();
  cv::Mat visualizeKeypoints(const CVKeyPoints& keypoints);
  cv::Mat visualizeKeypoints();
  const CVMats& calcPyramid();
  const cv::Mat& calcGrayscale();

  const ViewId id() const { return id_; }
  const cv::Mat& img() const { return img_; }
  const CVKeyPoints& keypoints() const { return keypoints_; }
  const CVMats& pyramid() const { return pyramid_; }
  const cv::Mat& img_grayscale() const { return img_grayscale_; }

 private:
  ViewId id_;
  cv::Mat img_;
  cv::Mat img_grayscale_;
  CVKeyPoints keypoints_;
  CVMats pyramid_;
};

using ViewPtr = std::shared_ptr<View>;

class ViewKeyPointId {
 public:
  ViewKeyPointId(ViewId view_id, KeyPointId keypoint_id);

  ViewId view_id() const { return view_id_; }
  KeyPointId keypoint_id() const { return keypoint_id_; }
  int id() const { return id_; }

 private:
  ViewId view_id_;
  KeyPointId keypoint_id_;
  int id_;
};

namespace std {
template <>
struct hash<ViewKeyPointId> {
  std::size_t operator()(const ViewKeyPointId& vkp_id) const noexcept {
    return vkp_id.id();
  }
};
}  // namespace std
