#include "motion3d_app.h"

#include <utility>

Motion3dApp::Motion3dApp() : Logger("Motion3dApp") {
  scene_ = std::make_shared<Scene>();
  camera_.setCallback([this](cv::Mat image_rgb) { onNewImage(std::move(image_rgb)); });
}

void Motion3dApp::onNewImage(cv::Mat image_rgb) {
  D("onNewImage!");
  D("scene_->processImage");
  ViewPtr view = scene_->processImage(image_rgb);
  D("scene_->visualizeMatches");
  cv::Mat img_vis = scene_->visualizeMatches();

  if (!img_vis.empty()) {
    D("renderer_.newImage_ext(img_vis)");
    renderer_.newImage_ext(img_vis);
  } else {
    D("renderer_.newImage_ext(std::move(image_rgb))");
    renderer_.newImage_ext(image_rgb.clone());
  }
}
