#pragma once

#include <opencv2/opencv.hpp>

#include "common.h"

using CVImageCallback = std::function<void(const cv::Mat &img)>;

class CVImageProcessor {
 protected:
  virtual void process(const cv::Mat &frame) = 0;

  friend class CVImageLoop;
};

class CVImageProcessorImpl : public CVImageProcessor {
 public:
  CVImageProcessorImpl(CVImageCallback callback);

 protected:
  virtual void process(const cv::Mat &frame) override;

 private:
  CVImageCallback callback_;
};

class CVImageLoop {
 public:
  CVImageLoop(const std::string &file_name, CVImageCallback callback,
              int waiting_ms = 0);
  CVImageLoop(const std::string &file_name,
              std::shared_ptr<CVImageProcessor> Processor, int waiting_ms = 0);

 private:
  std::string file_name_;
  std::shared_ptr<CVImageProcessor> processor_;
  int delay_ms_;
};
