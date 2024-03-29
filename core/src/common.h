#pragma once

#include <bitset>
#include <Eigen/Dense>
#include <functional>
#include <memory>
#include <opencv2/opencv.hpp>
#include "opencv2/core/eigen.hpp"
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <iomanip>


using CVPoints = std::vector<cv::Point2f>;
using CVKeyPoints = std::vector<cv::KeyPoint>;
using CVDescriptors = cv::Mat;
using CVMats = std::vector<cv::Mat>;
using CVPoints2f = std::vector<cv::Point2f>;

