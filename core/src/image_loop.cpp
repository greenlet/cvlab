#include "image_loop.h"

CVImageProcessorImpl::CVImageProcessorImpl(CVImageCallback callback) : callback_(callback) {}

void CVImageProcessorImpl::process(cv::Mat frame) { callback_(frame); }

CVImageLoop::CVImageLoop(const std::string &file_name, CVImageCallback callback, int delay_ms)
    : CVImageLoop(file_name, std::make_shared<CVImageProcessorImpl>(callback), delay_ms) {}

CVImageLoop::CVImageLoop(const std::string &file_path, std::shared_ptr<CVImageProcessor> processor,
                         int delay_ms)
    : file_path_(file_path), processor_(processor), delay_ms_(delay_ms) {
    auto cap = cv::VideoCapture(file_path_, cv::CAP_ANY);
    while (cap.isOpened()) {
        cv::Mat frame;
        if (!cap.read(frame)) {
            continue;
        }
        processor_->process(frame);

        int pressed_key = cv::waitKey(delay_ms_);
        if (pressed_key == 'q' || pressed_key == 'Q') {
            break;
        }
    }
}
