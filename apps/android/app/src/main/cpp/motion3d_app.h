#pragma once

#include <opencv2/video/tracking.hpp>

#include "logger.h"
#include "scene.h"
#include "camera_android.h"
#include "renderer.h"

class Motion3dApp : public Logger {
 public:
  Motion3dApp();

  CameraAndroid &camera() { return camera_; }
  Renderer &renderer() { return renderer_; }

 private:
  void onNewImage(cv::Mat image_rgb);

  std::shared_ptr<Scene> scene_;
  CameraAndroid camera_;
  Renderer renderer_;
};

