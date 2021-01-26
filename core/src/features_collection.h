#pragma once

#include "common.h"

enum class FeatureType {
    ORB,
};

template <unsigned FeatureSize>
class FeaturesCollection {
  public:
    using FeaturesMat = Eigen::Matrix<float, Eigen::Dynamic, FeatureSize>;
    using FeaturesMatPtr = std::shared_ptr<FeaturesMat>;

    FeaturesCollection(cv::Mat img, FeatureType feature_type, int nfeatures)
        : feature_type_(feature_type), nfeatures_(nfeatures) {
        switch (feature_type_) {
            case FeatureType::ORB:
                assert(FeatureSize == 32);
                cv::Ptr<cv::ORB> orb = cv::ORB::create(nfeatures);
                orb->detectAndCompute(img, cv::noArray(), keypoints_, descriptors_);
                break;
        }
    }

    FeatureType feature_type() { return feature_type_; }
    int nfeatures() { return nfeatures_; }

    const CVKeyPoints& keypoints() const { return keypoints_; }
    const CVDescriptors& descriptors() const { return descriptors_; }

    const FeaturesMatPtr features() {
        if (features_ || empty()) {
            return features_;
        }

        features_ = std::make_shared<FeaturesMat>(descriptors_.rows, descriptors_.cols);
        cv2eigen(descriptors_, *features_);

        return features_;
    }

    bool empty() { return descriptors_.rows == 0; }

  private:
    FeatureType feature_type_;
    int nfeatures_;

    CVKeyPoints keypoints_;
    CVDescriptors descriptors_;

    FeaturesMatPtr features_;
};

using ORBFeaturesCollection = FeaturesCollection<32>;
using ORBFeaturesCollectionPtr = std::shared_ptr<ORBFeaturesCollection>;
