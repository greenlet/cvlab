#pragma once

#include "matching/cashash_matcher.h"
#include "view.h"

class Calibrator {
  public:
    Calibrator();
    ~Calibrator();

    void addView(ViewPtr view);
    void calc();

    ViewPtrs &views() { return views_; }
    cv::Mat &views_kpts() { return views_kpts_; }

    int num_views() { return num_views_; }
    int num_kpts() { return num_kpts_; }
    const cv::Size &img_size() { return img_size_; }

  private:
    void track();
    void bundleAdjustment();
    void initPosesDepths();
    void deinitPosesDepths();

    ViewPtrs views_;
    cv::Mat views_kpts_;

    int num_views_;
    int num_kpts_;
    cv::Size img_size_;

    double cx_, cy_;
    double f_, k1_, k2_;
    double f_new_;
    // std::shared_ptr<double[]> poses_;
    // std::shared_ptr<double[]> inv_depths_;
    double *poses_ = nullptr;
    double *inv_depths_ = nullptr;
};
