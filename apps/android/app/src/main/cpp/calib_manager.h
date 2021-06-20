#pragma once

#include <jni.h>

#include "common.h"
#include "calibrator.h"
#include "logger.h"
#include "renderer.h"


using ImageCallback = std::function<void(ViewPtr)>;

class CalibManager;
using CalibManagerPtr = std::shared_ptr<CalibManager>;

class CalibManager : Logger {
public:
    static CalibManagerPtr start(RendererPtr renderer, JNIEnv *jni_env, jobject calib_subscriber_jobject);
    CalibManager(RendererPtr renderer, JNIEnv *jni_env, jobject calib_subscriber_jobject);
    void onNewImage(ViewPtr view);

    void stop();
    void startCapture();
    void stopCapture();
    void startCalc();
    void stopCalc();

private:
    static void runCb_main(CalibManagerPtr calib_manager);
    void run_main();
    void onNewImage_main(ViewPtr view);
    void undistort_(ViewPtr view);
    void stop_main();
    void startCapture_main();
    void stopCapture_main();
    void startCalc_main();
    void stopCalc_main();

    enum class State {
        None, Viewing, Capturing, CapturePreview, Calculating,
    };

    const char *stateToStr(State state) {
        switch (state) {
            case State::None:
                return "None";
            case State::Viewing:
                return "Viewing";
            case State::Capturing:
                return "Capturing";
            case State::CapturePreview:
                return "CapturePreview";
            case State::Calculating:
                return "Calculating";
        }
    }

    inline void updateState_(State state, int value = 0, std::string error = "");
    void updateState_main(State state, int value = 0, std::string error = "");
    void startCalcThread_main();
    void stopCalcThread_main();
    void run_calc();
    void stopRenderLoop_main();
    inline bool calcCallback_calc(const Calibrator::CalcState &state, double agg_start_perc, double agg_part_perc);
    inline bool calcTrackCallback_calc(const Calibrator::CalcState &state, double agg_start_perc, double agg_part_perc, CVPoints &points_pre);

    std::mutex mu_main_;
    std::condition_variable cv_main_;
    std::queue<std::function<void()>> func_queue_main_;

    RendererPtr renderer_;
    JNIEnv *jni_env_main_ = nullptr;
    JavaVM *java_vm_;
    jobject calib_subscriber_jobject_ = nullptr;
    jmethodID onCalibStateUpdate_jmethod_;

    Calibrator calibrator_;
    bool calib_calculated_ = false;
    bool started_main_ = false;
    bool stop_main_ = false;
    State state_ = State::None;
    int state_value_ = 0;


    ViewPtrs frames_;
    int max_frames_ = 30;
    int cur_render_loop_id_ = -1;

    bool started_calc_ = false;
    bool stop_calc_ = false;

    bool calib_calc_started_ = false;
    ViewPtrs views_track_vis_;
    ViewPtr view_to_undistort_;
    std::atomic<bool> is_undistorting_ = false;
};

