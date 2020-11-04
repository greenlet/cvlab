#pragma once

#include "common.h"

using CVImageCallback = std::function<void(const cv::Mat img)>;

class CVImageProcessor {
   protected:
    virtual void process(cv::Mat img) = 0;

    friend class CVImageLoop;
};

class CVImageProcessorImpl : public CVImageProcessor {
   public:
    CVImageProcessorImpl(CVImageCallback callback);

   protected:
    virtual void process(cv::Mat img) override;

   private:
    CVImageCallback callback_;
};

class CVImageLoop {
   public:
    CVImageLoop(const std::string &file_name, CVImageCallback callback, int waiting_ms = 0);
    CVImageLoop(const std::string &file_name, std::shared_ptr<CVImageProcessor> Processor,
                int waiting_ms = 0);

   private:
    std::string file_name_;
    std::shared_ptr<CVImageProcessor> processor_;
    int delay_ms_;
};
