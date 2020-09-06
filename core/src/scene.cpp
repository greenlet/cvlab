#include "scene.h"

Scene::Scene () : next_view_id_(0) {}

ViewPtr Scene::processImage(cv::Mat img) {
  ViewPtr view = std::make_shared<View>(next_view_id_++, img);

  view->calcKeypoints();

  klt_tracker_.processView(view);

  views_.push_back(view);

  return view;
}

