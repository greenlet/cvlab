#pragma once

#include <opencv2/video/tracking.hpp>
#include <jni.h>

#include "camera_android.h"
#include "logger.h"
#include "renderer.h"
#include "scene.h"
#include "calib_manager.h"



class CvlabApp : public Logger {
   public:
    CvlabApp(JNIEnv *env, jobject cvlab_jboject);
    ~CvlabApp();

    void startCalibration(JNIEnv *env, jobject thiz);
    void stopCalibration();
    void startCalibCapture(JNIEnv *env, jobject thiz);
    void stopCalibCapture();
    void startCalibCalc();
    void stopCalibCalc();

    CameraAndroid &camera() { return camera_; }
    RendererPtr &renderer() { return renderer_; }
    CalibManagerPtr calib_manager() { return calib_manager_; }

   private:
    void onNewImage(ViewPtr view);

    std::mutex mu_;

    CameraAndroid camera_;
    RendererPtr renderer_;
    ScenePtr scene_;

    JNIEnv *jni_env_;
    jobject  cvlab_jobject_;
    CalibManagerPtr calib_manager_;
    bool calib_calc_happened_ = false;
};
