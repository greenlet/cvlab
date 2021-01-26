#include "view.h"

#include <opencv2/imgproc/imgproc.hpp>

#include "utils.h"

View::View(ViewId id, cv::Mat img) : id_(id), img_(img) {}

cv::Mat View::visualizeKeypoints(const CVKeyPoints& keypoints) {
    cv::Mat res;
    if (!img_.empty()) {
        // cv::drawKeypoints(img_, keypoints, res);
        img_.copyTo(res);
        for (const cv::KeyPoint& keypoint : keypoints) {
            cv::circle(res, keypoint.pt, 6, cv::Scalar(200, 0, 0), cv::FILLED, 8, 0);
        }
    }
    return res;
}

cv::Mat View::visualizeOrbKeypoints() {
    if (!orb_features_collection_) {
        calcORBFeaturesCollection();
    }
    if (orb_features_collection_->empty()) {
        return img_;
    }
    return visualizeKeypoints(orb_features_collection()->keypoints());
}

const CVMats& View::calcPyramid() {
    if (!pyramid_calculated_) {
        pyramid_calculated_ = true;
        calcGrayscale();
        cv::buildOpticalFlowPyramid(img_grayscale_, pyramid_, cv::Size(21, 21), 3);
    }
    return pyramid_;
}

const cv::Mat& View::calcGrayscale() {
    if (img_grayscale_.empty()) {
        cv::cvtColor(img_, img_grayscale_, cv::COLOR_BGR2GRAY);
    }
    return img_grayscale_;
}

const ORBFeaturesCollectionPtr View::calcORBFeaturesCollection(int nfeatures) {
    std::cout << "calcORBFeaturesCollection: " << nfeatures << std::endl;
    orb_features_collection_ =
        std::make_shared<ORBFeaturesCollection>(img_, FeatureType::ORB, nfeatures);
    return orb_features_collection_;
}

ViewKeyPointId::ViewKeyPointId(ViewId view_id, KeyPointId keypoint_id)
    : view_id_(view_id), keypoint_id_(keypoint_id) {
    id_ = (view_id_ << 14) + keypoint_id_;
}
