#pragma once

#include "common.h"
#include "tracking/cashash_tracker.h"
#include "tracking/klt_tracker.h"
#include "view.h"

class Scene {
 public:
  Scene();

  ViewPtr processImage(cv::Mat img);

  const std::vector<ViewPtr>& views() { return views_; }
  const KLTTracker& klt_tracker() const { return klt_tracker_; }
  const CasHashTracker& cashash_tracker() const { return cashash_tracker_; }

 private:
  int next_view_id_;
  std::vector<ViewPtr> views_;
  KLTTracker klt_tracker_;
  CasHashTracker cashash_tracker_;
};
