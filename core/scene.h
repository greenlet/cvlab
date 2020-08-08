#pragma once

#include "common.h"
#include "tracker.h"
#include "view.h"

class Scene {
 public:
  Scene();

  ViewPtr processImage(cv::Mat img);

  const std::vector<ViewPtr>& views() { return views_; }
  const Tracker& tracker() const { return tracker_; }

 private:
  int next_view_id_;
  std::vector<ViewPtr> views_;
  Tracker tracker_;
};
