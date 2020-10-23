#include "motion3d_app.h"

#include <utility>

Motion3dApp::Motion3dApp() : Logger("Motion3dApp") {
  scene_ = std::make_shared<Scene>();
  camera_.setCallback([this](cv::Mat image_rgb) { onNewImage(std::move(image_rgb)); });
}

void Motion3dApp::onNewImage(cv::Mat image_rgb) {
  D("onNewImage!");
  ViewPtr view = scene_->processImage(image_rgb);
  cv::Mat img_vis = scene_->visualizeMatches();

  if (!img_vis.empty()) {
    renderer_.newImage_ext(img_vis);
  } else {
    renderer_.newImage_ext(std::move(image_rgb));
  }
}
