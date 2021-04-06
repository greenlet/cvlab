#include "cvlab_app.h"

#include <utility>

CvlabApp::CvlabApp(JNIEnv *env, jobject cvlab_jobject)
    : Logger("CvlabApp"), jni_env_(env), cvlab_jobject_(cvlab_jobject) {
    scene_ = std::make_shared<Scene>();
    camera_.setCallback([this](cv::Mat image_rgb) { onNewImage(std::move(image_rgb)); });

    cvlab_jobject_ = reinterpret_cast<jobject>(jni_env_->NewGlobalRef(cvlab_jobject_));
    jclass cvlab_jclass = jni_env_->GetObjectClass(cvlab_jobject_);
    onCalibStateUpdate_jmethod_ = jni_env_->GetMethodID(cvlab_jclass, "onCalibStateUpdate_jni", "(II)V");
    jni_env_->GetJavaVM(&java_vm_);
}

void CvlabApp::updateCalibState(CalibState state, int value) {
    calib_state_ = state;
    JNIEnv *jni_env;
    java_vm_->GetEnv((void **)&jni_env, JNI_VERSION_1_6);
    jni_env->CallVoidMethod(cvlab_jobject_, onCalibStateUpdate_jmethod_, int(state), value);
}

void CvlabApp::calibCapture(cv::Mat image_rgb) {
    JNIEnv *jni_env;
    java_vm_->GetEnv((void **)&jni_env, JNI_VERSION_1_6);
    if (calib_frame_no_ < calib_max_frames_) {
        ViewPtr view = std::make_shared<View>(calib_frame_no_, image_rgb);
        calibrator_.addView(view);
        calib_frame_no_ += 1;
        double perc = double(100.0 * calib_frame_no_) / calib_max_frames_;
        jni_env->CallVoidMethod(cvlab_jobject_, onCalibStateUpdate_jmethod_, int(CalibState::Capturing), int(perc));
    } else if (calib_state_ == CalibState::Capturing) {
        updateCalibState(CalibState::CapturePreview);
        calib_frame_no_ = 0;
    }

    renderer_.newImage_ext(image_rgb);
}

void CvlabApp::calibCapturePreview() {
    cv::Mat image = calibrator_.views()[calib_frame_no_]->img();
    calib_frame_no_ = (calib_frame_no_ + 1) % calib_max_frames_;

    renderer_.newImage_ext(image);
}

void CvlabApp::onNewImage(cv::Mat image_rgb) {
    std::lock_guard<std::mutex> lock(mu_);

    if (calibration_started_) {
        switch (calib_state_) {
            case CalibState::Viewing:
                renderer_.newImage_ext(image_rgb);
                break;
            case CalibState::Capturing:
                calibCapture(image_rgb);
                break;
            case CalibState::CapturePreview:
                calibCapturePreview();
                break;
            case CalibState::Calculating:
                break;
        }
    } else {
//        ViewPtr view = scene_->processImage(image_rgb);
//        cv::Mat img_vis = scene_->visualizeMatches();
//
//        if (!img_vis.empty()) {
//            renderer_.newImage_ext(img_vis);
//        } else {
//            renderer_.newImage_ext(image_rgb.clone());
//        }

        renderer_.newImage_ext(image_rgb);
    }
}

CvlabApp::~CvlabApp() {
    std::lock_guard<std::mutex> lock(mu_);
    camera_.stop();
    stopCalibration();
    renderer_.deinit_ext();
}

void CvlabApp::startCalibration() {
    std::lock_guard<std::mutex> lock(mu_);
    if (calibration_started_) {
        return;
    }
    calibration_started_ = true;
    updateCalibState(CalibState::Viewing);
}

void CvlabApp::stopCalibration() {
    std::lock_guard<std::mutex> lock(mu_);
    if (!calibration_started_) {
        return;
    }
    calibration_started_ = false;
    updateCalibState(CalibState::Viewing);
}

void CvlabApp::startCalibCapture() {
    std::lock_guard<std::mutex> lock(mu_);
    calibrator_.views().clear();
    calib_frame_no_ = 0;
    updateCalibState(CalibState::Capturing);
}

void CvlabApp::stopCalibCapture() {
    std::lock_guard<std::mutex> lock(mu_);
    calibrator_.views().clear();
    calib_frame_no_ = 0;
    updateCalibState(CalibState::Viewing);
}

void CvlabApp::startCalibCalc() {
    std::lock_guard<std::mutex> lock(mu_);
    if (calib_calc_started_) {
        return;
    }
    calib_calc_started_ = true;
    calib_thread_ = std::thread(&CvlabApp::calibProcess, this);
    updateCalibState(CalibState::Calculating);
}

void CvlabApp::stopCalibCalc() {
    std::lock_guard<std::mutex> lock(mu_);
    if (!calib_calc_started_) {
        return;
    }
    calib_calc_started_ = false;
    calib_thread_.detach();
    updateCalibState(CalibState::CapturePreview);
}

void CvlabApp::calibProcess() {
    D("calibProcess begin");
    JNIEnv *jni_env;
    java_vm_->AttachCurrentThread(&jni_env, nullptr);

    int i = 0;
    while (true) {
        std::unique_lock<std::mutex> lock(mu_);
        if (!calib_calc_started_) {
            break;
        }
        lock.unlock();

        D("calibProcess: %d", i++);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    java_vm_->DetachCurrentThread();
    D("calibProcess end");
}


