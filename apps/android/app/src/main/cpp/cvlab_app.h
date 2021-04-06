#pragma once

#include <opencv2/video/tracking.hpp>
#include <jni.h>

#include "camera_android.h"
#include "logger.h"
#include "renderer.h"
#include "scene.h"
#include "calibrator.h"


class CvlabApp : public Logger {
   public:
    CvlabApp(JNIEnv *env, jobject cvlab_jboject);
    ~CvlabApp();

    void startCalibration();
    void stopCalibration();
    void startCalibCapture();
    void stopCalibCapture();
    void startCalibCalc();
    void stopCalibCalc();

    CameraAndroid &camera() { return camera_; }
    Renderer &renderer() { return renderer_; }
    Calibrator &calibrator() { return calibrator_; }

   private:
    enum class CalibState {
        Viewing, Capturing, CapturePreview, Calculating
    };

    void onNewImage(cv::Mat image_rgb);
    void updateCalibState(CalibState state, int value = 0);
    void calibCapture(cv::Mat image_rgb);
    void calibCapturePreview();
    void calibProcess();

    std::mutex mu_;

    std::shared_ptr<Scene> scene_;
    CameraAndroid camera_;
    Renderer renderer_;
    Calibrator calibrator_;
    bool calibration_started_ = false;
    int calib_frame_no_ = 0;
    int calib_max_frames_ = 30;

    CalibState calib_state_;

    JavaVM *java_vm_;
    JNIEnv *jni_env_;
    jobject cvlab_jobject_;
    jmethodID onCalibStateUpdate_jmethod_;

    std::thread calib_thread_;
    bool calib_calc_started_ = false;
};
