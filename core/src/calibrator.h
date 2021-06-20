#pragma once

#include "matching/cashash_matcher.h"
#include "view.h"
#include <ceres/ceres.h>


class Calibrator : ceres::IterationCallback {
  public:
    struct CalcState {
      CalcState(int step_no, int max_steps, bool is_error = false, const char *error_message = "",
        std::vector<bool> inlier_mask = {}, CVPoints points = {}) {
          this->step_no = step_no;
          this->max_steps = max_steps;
          this->is_error = is_error;
          this->error_message = std::string(error_message);
          this->inlier_mask = std::move(inlier_mask);
          this->points = std::move(points);
        }

      int step_no = 0;
      int max_steps = 0;
      bool is_error = false;
      std::string error_message;
      std::vector<bool> inlier_mask;
      CVPoints points;
    };
    
    using CalcCallback = std::function<bool(CalcState)>;

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
    bool stopped() { return stopped_; }

    void track(CalcCallback callback = nullptr);
    void bundleAdjustment(CalcCallback callback = nullptr);
    void calcUndistParams(CalcCallback callback = nullptr);

    void undistort(cv::Mat img_src, cv::Mat img_dst);

    ceres::CallbackReturnType operator()(const ceres::IterationSummary &summary) override;
  private:
    void initPosesDepths();
    void deinitPosesDepths();

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

    bool stopped_ = false;
    int min_tracked_views_ = 10;
    int min_tracked_points_ = 200;
    int max_ba_iterations_ = 100;
    CalcCallback ba_callback_ = nullptr;
    int max_undist_iterations_ = 10;
};
