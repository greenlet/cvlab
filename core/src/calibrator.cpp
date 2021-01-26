#include "calibrator.h"
#include "matching/cascade_hasher.h"

Calibrator::Calibrator() {}

void Calibrator::addView(ViewPtr view) { views_.push_back(view); }

void Calibrator::calc() {
    if (views_.size() <= 1) {
        std::cout << "Cannot start calibration. No or too little views added (" << views_.size()
                  << ")" << std::endl;
        return;
    }
    track();
}

void Calibrator::track() {
    int max_corners = 20000;
    int n_views = views_.size();

    ViewPtr view_pre = views_[0];
    const cv::Mat &img_gray_pre = view_pre->img_grayscale();
    std::vector<cv::Point2f> corners_pre;
    cv::goodFeaturesToTrack(view_pre->img_grayscale(), corners_pre, max_corners, 1e-10, 10);
    int num_corners = corners_pre.size();

    std::cout << "num_corners = " << num_corners << std::endl;

    bool inlier_mask[num_corners];
    cv::Mat features = cv::Mat(2 * n_views, num_corners, CV_32FC1);
    for (int j = 0; j < num_corners; j++) {
        features.at<float>(0, j) = corners_pre[j].x;
        features.at<float>(1, j) = corners_pre[j].y;
        inlier_mask[j] = true;
    }

    std::vector<cv::Point2f> corners_fwd;
    std::vector<cv::Point2f> corners_bwd;
    std::vector<unsigned char> status_fwd;
    std::vector<unsigned char> status_bwd;
    std::vector<float> err_fwd;
    std::vector<float> err_bwd;
    corners_fwd.reserve(num_corners);
    corners_bwd.reserve(num_corners);
    status_fwd.reserve(num_corners);
    status_bwd.reserve(num_corners);
    err_fwd.reserve(num_corners);
    err_bwd.reserve(num_corners);

    int num_inliers = 0;
    for (int i = 1; i < views_.size(); i++) {
        ViewPtr view_cur = views_[i];
        const CVMats &pyr_pre = view_pre->pyramid();
        const CVMats &pyr_cur = view_cur->pyramid();
        
        cv::calcOpticalFlowPyrLK(pyr_pre, pyr_cur, corners_pre, corners_fwd, status_fwd, err_fwd);
        cv::calcOpticalFlowPyrLK(pyr_cur, pyr_pre, corners_fwd, corners_bwd, status_bwd, err_bwd);
        
        for (int j = 0; j < num_corners; j++) {
            features.at<float>(2 * i + 0, j) = corners_fwd[j].x;
            features.at<float>(2 * i + 1, j) = corners_fwd[j].y;

            float bidir_err = cv::norm(corners_bwd[j] - corners_pre[j]);
            if (bidir_err > 0.1 || status_fwd[i] == 0 || status_bwd[i] == 0) {
                inlier_mask[j] = false;
            }
        }

        view_pre = view_cur;
        corners_pre = std::move(corners_fwd);

        num_inliers = std::accumulate(inlier_mask, inlier_mask + num_corners, 0);
        std::cout << i << ". num_inliers = " << num_inliers << std::endl;
    }

    if (num_inliers > 0) {
        views_keypoints_ = cv::Mat(features.rows, num_inliers, CV_32FC1);
        for (int r = 0; r < features.rows; r++) {
            int c_inlier = 0;
            for (int c = 0; c < features.cols; c++) {
                if (inlier_mask[c]) {
                    assert(c_inlier < num_inliers);
                    views_keypoints_.at<float>(r, c_inlier++) = features.at<float>(r, c);
                }
            }
        }
    }

}
