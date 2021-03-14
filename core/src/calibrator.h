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
    int image_width() { return image_width_; }
    int image_height() { return image_height_; }

    void undistort(cv::Mat img_src, cv::Mat img_dst);

  private:
    void track();
    void bundleAdjustment();
    void initPosesDepths();
    void deinitPosesDepths();
    void undistortImages();

    ViewPtrs views_;
    cv::Mat views_kpts_;

    int num_views_;
    int num_kpts_;
    int image_height_;
    int image_width_;

    double cx_, cy_;
    double f_, k1_, k2_;
    double f_new_;
    // std::shared_ptr<double[]> poses_;
    // std::shared_ptr<double[]> inv_depths_;
    double *poses_ = nullptr;
    double *inv_depths_ = nullptr;

    cv::Mat ud_mapx_, ud_mapy_;  // undistorted->distorted mapping for dense matching
    cv::Mat du_mapx_, du_mapy_;  // distorted->undistorted mapping for final visualization
};
