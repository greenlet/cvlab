#pragma once

#include <opencv2/video/tracking.hpp>

#include "camera_android.h"
#include "logger.h"
#include "renderer.h"
#include "scene.h"

class CvlabApp : public Logger {
   public:
    CvlabApp();

    CameraAndroid &camera() { return camera_; }
    Renderer &renderer() { return renderer_; }

   private:
    void onNewImage(cv::Mat image_rgb);

    std::shared_ptr<Scene> scene_;
    CameraAndroid camera_;
    Renderer renderer_;
};
