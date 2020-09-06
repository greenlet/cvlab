#pragma once

#include <opencv2/video/tracking.hpp>

#include "scene.h"
#include "camera_android.h"

class Motion3dApp {
public:
    Motion3dApp() = default;

    void startCamera();
    void stopCamera();

    const CameraAndroid &camera() { return camera_; }

private:
    std::shared_ptr<Scene> scene_;
    CameraAndroid camera_;
};

