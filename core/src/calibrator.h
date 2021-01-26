#pragma once

#include "matching/cashash_matcher.h"
#include "view.h"

class Calibrator {
  public:
    Calibrator();

    void addView(ViewPtr view);
    void calc();

    ViewPtrs &views() { return views_; }
    cv::Mat &views_keypoints() { return views_keypoints_; }

  private:
    void track();

    ViewPtrs views_;
    cv::Mat views_keypoints_;
};
