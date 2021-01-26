#include <iomanip>

#include "calibrator.h"
#include "common.h"
#include "image_loop.h"
#include "utils.h"

int main(int argc, char **argv) {
    std::cout << "Starting CV Lab Calibration" << std::endl;

    if (argc != 2 && argc != 3) {
        std::cout << "Usage: " << argv[0] << " <video-file-path>" << std::endl;
        return 1;
    }

    std::string file_path(argv[1]);
    std::cout << "Opening file: " << file_path << std::endl;

    const int n_frames = 30;
    const double inter_frame_step_ms = 0;

    Calibrator calib;
    cv::VideoCapture cap = cv::VideoCapture(file_path, cv::CAP_ANY);
    double fps = cap.get(cv::CAP_PROP_FPS);
    std::cout << "Source fps: " << std::setprecision(2) << fps << std::endl;
    int frame_ind = 0;
    int last_used_frame_ind = -1;
    int n_frames_used = 0;
    while (cap.isOpened() && n_frames_used < n_frames) {
        frame_ind++;
        cv::Mat frame;
        if (!cap.read(frame)) {
            continue;
        }

        double time_from_last_used_frame_ms = (frame_ind - last_used_frame_ind) / fps * 1000;
        if (last_used_frame_ind < 0 || time_from_last_used_frame_ms >= inter_frame_step_ms) {
            // std::cout << "Frame: " << frame_ind << ". Time ms: " << std::setprecision(3)
            //           << std::fixed << frame_ind / fps * 1000 << std::endl;

            ViewPtr view = std::make_shared<View>(frame_ind, frame);
            calib.addView(view);
            last_used_frame_ind = frame_ind;
            n_frames_used++;
        }
    }

    calib.calc();
    cv::Mat &view_kpts = calib.views_keypoints();

    std::cout << "Views total: " << calib.views().size() << std::endl;
    std::cout << "Views keypoints: " << view_kpts.size << std::endl;
    for (int i = 0; i < calib.views().size(); i++) {
        cv::Mat img = calib.views()[i]->img().clone();

        for (int j = 0; j < view_kpts.cols; j++) {
            cv::Point2f kpt_cur{view_kpts.at<float>(2 * i, j), view_kpts.at<float>(2 * i + 1, j)};
            if (i > 0) {
                cv::Point2f kpt_pre{view_kpts.at<float>(2 * (i - 1), j), view_kpts.at<float>(2 * (i - 1) + 1, j)};
                cv::circle(img, kpt_pre, 3, cv::Scalar(0, 0, 200), cv::FILLED);
                cv::line(img, kpt_pre, kpt_cur, cv::Scalar(0, 200, 0));
            }
            cv::circle(img, kpt_cur, 3, cv::Scalar(200, 0, 0), cv::FILLED);
        }

        cv::imshow("keypoints", img);
        // cv::waitKey(std::max(int(inter_frame_step_ms), 100));
        cv::waitKey(0);
    }

    return 0;
}
