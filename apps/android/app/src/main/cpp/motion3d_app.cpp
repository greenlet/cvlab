#include "motion3d_app.h"

#include <utility>

Motion3dApp::Motion3dApp() : Logger("Motion3dApp") {
  camera_.setCallback([this](cv::Mat image_rgb) { onNewImage(std::move(image_rgb)); });
}

void Motion3dApp::onNewImage(cv::Mat image_rgb) {
  renderer_.newImage_ext(std::move(image_rgb));
}
