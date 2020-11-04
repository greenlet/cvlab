#include "cvlab_app.h"

#include <utility>

CvlabApp::CvlabApp() : Logger("CvlabApp") {
    scene_ = std::make_shared<Scene>();
    camera_.setCallback([this](cv::Mat image_rgb) { onNewImage(std::move(image_rgb)); });
}

void CvlabApp::onNewImage(cv::Mat image_rgb) {
    ViewPtr view = scene_->processImage(image_rgb);
    cv::Mat img_vis = scene_->visualizeMatches();

    if (!img_vis.empty()) {
        renderer_.newImage_ext(img_vis);
    } else {
        renderer_.newImage_ext(image_rgb.clone());
    }
}
