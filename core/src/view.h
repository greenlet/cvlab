#pragma once

#include "common.h"
#include "features_collection.h"

using ViewId = int;
using KeyPointId = int;

class View {
  public:
    View(ViewId id, cv::Mat img);

    cv::Mat visualizeKeypoints(const CVKeyPoints& keypoints);
    cv::Mat visualizeOrbKeypoints();
    const CVMats& calcPyramid();
    const cv::Mat& calcGrayscale();

    const ViewId id() const { return id_; }
    const cv::Mat& img() const { return img_; }

    // TODO: bring consistensy to pyramid, grayscale and orb_features_collection calculations
    const CVMats& pyramid() {
        calcPyramid();
        return pyramid_;
    }
    const cv::Mat& img_grayscale() {
        calcGrayscale();
        return img_grayscale_;
    }

    const ORBFeaturesCollectionPtr calcORBFeaturesCollection(int nfeatures = 2000);

    const ORBFeaturesCollectionPtr orb_features_collection() {
        if (!orb_features_collection_) {
            calcORBFeaturesCollection();
        }
        return orb_features_collection_;
    }

  private:
    ViewId id_;
    cv::Mat img_;
    cv::Mat img_grayscale_;
    CVMats pyramid_;
    bool pyramid_calculated_ = false;

    ORBFeaturesCollectionPtr orb_features_collection_;
};

using ViewPtr = std::shared_ptr<View>;
using ViewPtrs = std::vector<ViewPtr>;

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
    std::size_t operator()(const ViewKeyPointId& vkp_id) const noexcept { return vkp_id.id(); }
};
}  // namespace std
