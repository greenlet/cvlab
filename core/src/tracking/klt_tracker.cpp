
#include "tracking/klt_tracker.h"

#include <opencv2/video/tracking.hpp>

#include "utils.h"

KLTTracker::KLTTracker() {}

void KLTTracker::processView(ViewPtr view) {
    const CVKeyPoints &cur_keypoints = view->calcKeypoints();
    view->calcPyramid();

    if (!last_view_) {
        last_view_ = view;
        tracked_keypoints_ = cur_keypoints;
        tracked_points_ = KeyPointsToPoints2f(cur_keypoints);
        return;
    }

    std::vector<float> error;
    std::vector<unsigned char> status;
    CVPoints2f found_points(tracked_points_.size());
    cv::calcOpticalFlowPyrLK(last_view_->pyramid(), view->pyramid(), tracked_points_, found_points,
                             status, error);

    size_t i_found = 0;
    for (size_t i = 0; i < tracked_points_.size(); i++) {
        if (status[i]) {
            tracked_keypoints_[i_found] = tracked_keypoints_[i];
            tracked_keypoints_[i_found].pt = found_points[i];
            tracked_points_[i_found] = found_points[i];
            i_found++;
        }
    }

    if (i_found >= 10) {
        tracked_keypoints_.resize(i_found);
        tracked_points_.resize(i_found);
    } else {
        tracked_keypoints_ = cur_keypoints;
        tracked_points_ = KeyPointsToPoints2f(cur_keypoints);
    }

    last_view_ = view;
}
