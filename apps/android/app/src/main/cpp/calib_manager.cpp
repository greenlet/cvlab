#include "calib_manager.h"

#include "common.h"

void CalibManager::runCb_main(CalibManagerPtr calib_manager) { calib_manager->run_main(); }

CalibManagerPtr CalibManager::start(RendererPtr renderer, JNIEnv *jni_env, jobject calib_subscriber_jobject) {
    CalibManagerPtr calib_manager = std::make_shared<CalibManager>(renderer, jni_env, calib_subscriber_jobject);
    std::thread t(&CalibManager::runCb_main, calib_manager);
    t.detach();
    return calib_manager;
}

CalibManager::CalibManager(RendererPtr renderer, JNIEnv *jni_env, jobject calib_subscriber_jobject)
    : Logger("CalibManager"), renderer_(std::move(renderer)) {
    calib_subscriber_jobject_ =
        reinterpret_cast<jobject>(jni_env->NewGlobalRef(calib_subscriber_jobject));
    jclass cvlab_jclass = jni_env->GetObjectClass(calib_subscriber_jobject_);
    onCalibStateUpdate_jmethod_ = jni_env->GetMethodID(cvlab_jclass, "onCalibStateUpdate_jni", "(IILjava/lang/String;)V");
    jni_env->GetJavaVM(&java_vm_);
    started_main_ = true;
    stop_main_ = false;
}

void CalibManager::run_main() {
    java_vm_->AttachCurrentThread(&jni_env_main_, nullptr);

    u_lock lock(mu_main_);
    updateState_main(State::Viewing);
    while (started_main_) {
        cv_main_.wait(lock);
        if (stop_main_) {
            if (!started_calc_) {
                started_main_ = false;
            }
            stopCalcThread_main();
            break;
        } else {
            while (!func_queue_main_.empty()) {
                auto f = func_queue_main_.front();
                func_queue_main_.pop();
                f();
            }
            ViewPtr view_to_undistort = view_to_undistort_;
            view_to_undistort_ = nullptr;

            lock.unlock();
            undistort_(view_to_undistort);
            lock.lock();
        }
    }

    java_vm_->DetachCurrentThread();
}

void CalibManager::undistort_(ViewPtr view) {
    if (!view) {
        return;
    }
    bool expected = false;
    if (!is_undistorting_.compare_exchange_strong(expected, true)) {
        return;
    }

    cv::Mat img_undist(view->img().size(), view->img().type());
    calibrator_.undistort(view->img(), img_undist);
    view = std::make_shared<View>(view->id(), img_undist);

    is_undistorting_.store(false);

    renderer_->showView(view);
}

void CalibManager::onNewImage_main(ViewPtr view) {
//    D("onNewImage_main: %s", stateToStr(state_));
    if (state_ == State::Capturing) {
        if (frames_.size() < max_frames_) {
            D("Frame added: %ld", frames_.size());
            frames_.push_back(view);
            renderer_->showView(view);
        } else {
            D("Playing loop: %ld", frames_.size());
            cur_render_loop_id_ = renderer_->startLoop(frames_, frames_.size());
            updateState_main(State::CapturePreview);
        }
    } else if (state_ == State::Viewing) {
        if (calib_calculated_) {
//            D("onNewImage_main. Undistoring");
            view_to_undistort_ = view;
        } else {
            renderer_->showView(view);
        }
    }
}

void CalibManager::onNewImage(ViewPtr view) {
    {
        G_LOCK(mu_main_)
        if (!started_main_ || stop_main_ || state_ == State::None) {
            return;
        }
        func_queue_main_.push([this, view] { onNewImage_main(view); });
    }
    cv_main_.notify_one();
}

void CalibManager::stop() {
    {
        G_LOCK(mu_main_)
        if (!started_main_ || stop_main_ || state_ == State::None) {
            return;
        }
        stop_main_ = true;
        func_queue_main_.push([this] { stop_main(); });
    }
    cv_main_.notify_one();
}

void CalibManager::startCapture() {
    {
        G_LOCK(mu_main_)
        func_queue_main_.push([this] { startCapture_main(); });
    }
    cv_main_.notify_one();
}

void CalibManager::stopCapture() {
    {
        G_LOCK(mu_main_)
        func_queue_main_.push([this] { stopCapture_main(); });
    }
    cv_main_.notify_one();
}

void CalibManager::startCalc() {
    {
        G_LOCK(mu_main_)
        func_queue_main_.push([this] { startCalc_main(); });
    }
    cv_main_.notify_one();
}

void CalibManager::stopCalc() {
    {
        G_LOCK(mu_main_)
        func_queue_main_.push([this] { stopCalc_main(); });
    }
    cv_main_.notify_one();
}

void CalibManager::updateState_main(CalibManager::State state, int value, std::string error) {
    D("updateState_main: %s. value: %d. err: %s", stateToStr(state), value, error.c_str());
    if (state_ != state || state_value_ != value) {
        state_ = state;
        state_value_ = value;
        jstring jstr_error = jni_env_main_->NewStringUTF(error.c_str());
        jni_env_main_->CallVoidMethod(calib_subscriber_jobject_, onCalibStateUpdate_jmethod_,
                                      int(state), value, jstr_error);
    }
}

void CalibManager::startCalcThread_main() {
    if (started_calc_) {
        return;
    }
    started_calc_ = true;
    stop_calc_ = false;
    std::thread t{&CalibManager::run_calc, this};
    t.detach();
}

void CalibManager::stopCalcThread_main() {
    if (!started_calc_) {
        return;
    }
    stop_calc_ = true;
    stopRenderLoop_main();
}

bool CalibManager::calcCallback_calc(const Calibrator::CalcState &state, double agg_start_perc, double agg_part_perc) {
    if (stop_calc_) {
        return false;
    }

    int value = std::ceil(agg_start_perc + double(state.step_no * agg_part_perc) / std::max(state.max_steps, 1));
    if (state.is_error) {
        updateState_(State::CapturePreview, value, state.error_message);
        cur_render_loop_id_ = renderer_->startLoop(frames_, frames_.size());
        stop_calc_ = true;
        return false;
    }

    updateState_(State::Calculating, value);
    return true;
}

bool CalibManager::calcTrackCallback_calc(const Calibrator::CalcState &state, double agg_start_perc,
                                          double agg_part_perc, CVPoints &points_pre) {
    if (!calcCallback_calc(state, agg_start_perc, agg_part_perc)) {
        return true;
    }

    const ViewPtrs &views = calibrator_.views();
    D("step_no: %d. inlier_mask: %ld. points: %ld. points_pre: %ld",
      state.step_no, state.inlier_mask.size(), state.points.size(), points_pre.size());
    if (state.step_no < views.size()) {
//        D("views: %ld", views.size());
        ViewPtr view = views[state.step_no];
//        D("img size: (%d, %d)", view->img().size().width, view->img().size().height);
        cv::Mat img_vis = view->img().clone();
        if (state.step_no > 0) {
            for (int i = 0; i < state.inlier_mask.size(); i++) {
                if (!state.inlier_mask[i]) {
                    continue;
                }
                auto &pt_pre = points_pre[i];
                auto &pt_cur = state.points[i];
//                    D("pt_pre: (%.2f x %.2f)", pt_pre.x, pt_pre.y);
//                    D("pt_cur: (%.2f x %.2f)", pt_cur.x, pt_cur.y);
                cv::circle(img_vis, pt_pre, 3, cv::Scalar(0, 0, 200), cv::FILLED);
                cv::line(img_vis, pt_pre, pt_cur, cv::Scalar(0, 200, 0));
            }
        }
        for (int i = 0; i < state.inlier_mask.size(); i++) {
            if(!state.inlier_mask[i]) {
                continue;
            }
            auto &pt_cur = state.points[i];
            cv::circle(img_vis, pt_cur, 3, cv::Scalar(200, 0, 0), cv::FILLED);
        }
        ViewPtr view_viz = std::make_shared<View>(view->id(), img_vis);
        views_track_vis_.push_back(view_viz);
        points_pre = state.points;
        renderer_->showView(view_viz);
    }
    return true;
}


void CalibManager::run_calc() {
    {
        G_LOCK(mu_main_);
        calib_calculated_ = false;
        for (const ViewPtr& frame : frames_) {
            calibrator_.addView(frame);
        }
        updateState_(State::Calculating);
    }

    double agg_start_perc = 0.0, agg_part_perc = 30.0;
    CVPoints points_pre;
    calibrator_.track([this, agg_start_perc, agg_part_perc, &points_pre](Calibrator::CalcState state) -> bool {
        bool retval;
        {
            G_LOCK(mu_main_);
            retval = calcTrackCallback_calc(state, agg_start_perc, agg_part_perc, points_pre);
        }
        cv_main_.notify_one();
        return retval;
    });

    D("Going to BA");
    agg_start_perc += agg_part_perc;
    agg_part_perc = 50.0;
    u_lock lock_ba(mu_main_);
    if (started_calc_ && !stop_calc_) {
        cur_render_loop_id_ = renderer_->startLoop(views_track_vis_, views_track_vis_.size());
        lock_ba.unlock();

        calibrator_.bundleAdjustment([this, agg_start_perc, agg_part_perc](Calibrator::CalcState state) -> bool {
            bool retval;
            {
                G_LOCK(mu_main_);
                retval = calcCallback_calc(state, agg_start_perc, agg_part_perc);
            }
            cv_main_.notify_one();
            return retval;
        });
    } else {
        lock_ba.unlock();
    }


    D("Going to Undist");
    agg_start_perc += agg_part_perc;
    agg_part_perc = 100.0 - agg_start_perc;
    u_lock lock_undist(mu_main_);
    if (started_calc_ && !stop_calc_) {
        lock_undist.unlock();

        calibrator_.calcUndistParams([this, agg_start_perc, agg_part_perc](Calibrator::CalcState state) -> bool {
            bool retval;
            {
                G_LOCK(mu_main_);
                retval = calcCallback_calc(state, agg_start_perc, agg_part_perc);
            }
            cv_main_.notify_one();
            return retval;
        });
    } else {
        lock_undist.unlock();
    }

    {
        G_LOCK(mu_main_);
        if (!calibrator_.stopped()) {
            calib_calculated_ = true;
            updateState_(State::Viewing);
        }
        started_calc_ = false;
    }

    cv_main_.notify_one();
}

void CalibManager::startCapture_main() {
    if (state_ == State::Viewing || state_ == State::CapturePreview) {
        size_t sz = frames_.size();
        frames_.clear();
        D("Starting capturing frames: %ld -> %ld", sz, frames_.size());
        updateState_main(State::Capturing);
    }
}

void CalibManager::stopCapture_main() {
    if (state_ == State::Capturing) {
        frames_.clear();
        stopRenderLoop_main();
        updateState_main(State::Viewing);
    }
}

void CalibManager::startCalc_main() {
    if (state_ == State::CapturePreview) {
        startCalcThread_main();
        updateState_main(State::Calculating);
    }
}

void CalibManager::stopCalc_main() {
    if (state_ == State::Calculating) {
        stopCalcThread_main();
        updateState_main(State::CapturePreview);
    }
}

void CalibManager::stopRenderLoop_main() {
    if (cur_render_loop_id_ > 0) {
        renderer_->stopLoop(cur_render_loop_id_);
        cur_render_loop_id_ = -1;
    }
}

void CalibManager::updateState_(CalibManager::State state, int value, std::string error) {
    func_queue_main_.push([this, state, value, error] {
        updateState_main(state, value, error);
    });
    cv_main_.notify_one();
}

void CalibManager::stop_main() {
    stopRenderLoop_main();
    stopCalcThread_main();
    stopCalc_main();
}

