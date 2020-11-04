#include "utils.h"

CVPoints2f KeyPointsToPoints2f(const CVKeyPoints &keypoints) {
    CVPoints2f points(keypoints.size());
    for (size_t i = 0; i < keypoints.size(); i++) {
        points[i] = keypoints[i].pt;
    }
    return points;
}

std::string cvmat2str(cv::Mat m) {
    std::ostringstream os;

    int type = m.type();
    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch (depth) {
        case CV_8U:
            os << "8U";
            break;
        case CV_8S:
            os << "8S";
            break;
        case CV_16U:
            os << "16U";
            break;
        case CV_16S:
            os << "16S";
            break;
        case CV_32S:
            os << "32S";
            break;
        case CV_32F:
            os << "32F";
            break;
        case CV_64F:
            os << "64F";
            break;
        default:
            os << "User";
            break;
    }

    os << "C" << unsigned(chans);
    os << " " << m.rows << "x" << m.cols;

    return os.str();
}
