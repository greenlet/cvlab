#include "cvlab_app.h"

#include <utility>

CvlabApp::CvlabApp(JNIEnv *env, jobject cvlab_jobject)
    : Logger("CvlabApp"), jni_env_(env), cvlab_jobject_(cvlab_jobject) {
    renderer_ = std::make_shared<Renderer>();
    scene_ = std::make_shared<Scene>();
    camera_.setCallback([this](ViewPtr view) { onNewImage(view); });
}

void CvlabApp::onNewImage(ViewPtr view) {
    G_LOCK(mu_);
    if (calib_manager_) {
        calib_manager_->onNewImage(view);
    } else {
        renderer_->showView(view);
    }
}

CvlabApp::~CvlabApp() {
    G_LOCK(mu_);
    camera_.stop();
    stopCalibration();
    renderer_->deinit();
}

void CvlabApp::startCalibration(JNIEnv *env, jobject thiz) {
    G_LOCK(mu_);
    if (!calib_manager_) {
        calib_manager_ = CalibManager::start(renderer_, env, thiz);
    }
}

void CvlabApp::stopCalibration() {
    G_LOCK(mu_);
    if (calib_manager_) {
        calib_manager_->stop();
        calib_manager_ = nullptr;
    }
}

void CvlabApp::startCalibCapture(JNIEnv *env, jobject thiz) {
    G_LOCK(mu_);
    if (calib_calc_happened_) {
        if (calib_manager_) {
            calib_manager_->stop();
        }
        calib_manager_ = CalibManager::start(renderer_, env, thiz);
    }
    calib_manager_->startCapture();
}

void CvlabApp::stopCalibCapture() {
    G_LOCK(mu_);
    if (!calib_manager_) {
        W("Warning. Calling %s(): empty calib_manager", __FUNCTION__ );
        return;
    }
    calib_manager_->stopCapture();
}

void CvlabApp::startCalibCalc() {
    G_LOCK(mu_);
    if (!calib_manager_) {
        W("Warning. Calling %s(): empty calib_manager", __FUNCTION__ );
        return;
    }
    calib_calc_happened_ = true;
    calib_manager_->startCalc();
}

void CvlabApp::stopCalibCalc() {
    G_LOCK(mu_);
    if (!calib_manager_) {
        W("Warning. Calling %s(): empty calib_manager", __FUNCTION__ );
        return;
    }
    calib_manager_->stopCalc();
    calib_manager_ = nullptr;
}

