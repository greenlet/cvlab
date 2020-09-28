#pragma once

#include "common.h"


CVPoints2f KeyPointsToPoints2f(const CVKeyPoints &keypoints);

std::string cvmat2str(cv::Mat m);
