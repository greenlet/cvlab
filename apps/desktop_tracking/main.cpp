#include "image_loop.h"
#include "scene.h"
#include "utils.h"
#include "view.h"

int main(int argc, char **argv) {
    std::cout << "Starting CV Lab Tracking" << std::endl;

    if (argc != 2 && argc != 3) {
        std::cout << "Usage: " << argv[0] << " <video-file-path> <delay-ms>?" << std::endl;
        return 1;
    }

    std::string file_path(argv[1]);
    std::cout << "Opening file: " << file_path << std::endl;

    int delay_ms = 0;
    if (argc == 3) {
        delay_ms = atoi(argv[2]);
    }

    Scene scene;
    CVImageLoop(
        file_path,
        [&scene](cv::Mat img) {
            ViewPtr view = scene.processImage(img);
            // std::cout << "View: " << view->id() << ". Keypoints: " << view->keypoints().size() <<
            // std::endl; cv::Mat img_vis = view->visualizeKeypoints();

            // const CVKeyPoints &keypoints = scene.klt_tracker().tracked_keypoints();
            // cv::Mat img_vis = view->visualizeKeypoints(keypoints);

            cv::Mat img_vis = scene.visualizeMatches();

            if (!img_vis.empty()) {
                cv::imshow("matches", img_vis);
            }
        },
        delay_ms);

    return 0;
}
