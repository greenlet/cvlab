#include "calibrator.h"

Calibrator::Calibrator() : num_views_(0), num_kpts_(0), image_width_(0), image_height_(0) {}

Calibrator::~Calibrator() { deinitPosesDepths(); }

void Calibrator::addView(ViewPtr view) {
    views_.push_back(view);
    num_views_++;
    if (num_views_ == 1) {
        cv::Size img_size = views_[0]->img().size();
        image_width_ = img_size.width;
        image_height_ = img_size.height;
    } else {
        cv::Size img_size = views_[num_views_ - 1]->img().size();
        assert(image_width_ == img_size.width && image_height_ == img_size.height);
    }
}

void Calibrator::calc() {
    if (views_.size() <= 1) {
        std::cout << "Cannot start calibration. No or too little views added (" << views_.size()
                  << ")" << std::endl;
        return;
    }

    track();
    if (!stopped_) {
        bundleAdjustment();
    }
    if (!stopped_) {
        calcUndistParams();
    }
}

void Calibrator::track(CalcCallback callback) {
    int max_corners = 20000;
    int n_views = views_.size();

    ViewPtr view_pre = views_[0];
    const cv::Mat& img_gray_pre = view_pre->img_grayscale();
    CVPoints corners_pre;
    cv::goodFeaturesToTrack(view_pre->img_grayscale(), corners_pre, max_corners, 1e-10, 10);
    int num_corners = corners_pre.size();

    if (num_corners < min_tracked_points_) {
        callback(CalcState(
            0, n_views, true, "Too little views/points tracked"
        ));
        stopped_ = true;
        return;
    }
    std::cout << "num_corners = " << num_corners << std::endl;

    std::vector<bool> inlier_mask(num_corners);
    cv::Mat features = cv::Mat(2 * n_views, num_corners, CV_32FC1);
    for (int j = 0; j < num_corners; j++) {
        features.at<float>(0, j) = corners_pre[j].x;
        features.at<float>(1, j) = corners_pre[j].y;
        inlier_mask[j] = true;
    }

    if (callback && !callback(CalcState(
        0, n_views, false, "", inlier_mask, corners_pre
        ))) {
        stopped_ = true;
        return;
    }

    CVPoints corners_fwd;
    CVPoints corners_bwd;
    std::vector<unsigned char> status_fwd;
    std::vector<unsigned char> status_bwd;
    std::vector<float> err_fwd;
    std::vector<float> err_bwd;
    corners_fwd.reserve(num_corners);
    corners_bwd.reserve(num_corners);
    status_fwd.reserve(num_corners);
    status_bwd.reserve(num_corners);
    err_fwd.reserve(num_corners);
    err_bwd.reserve(num_corners);

    int num_inliers = 0;
    int num_inliers_pre = 0;
    std::vector<bool> inlier_mask_pre = inlier_mask;
    for (int i = 1; i < views_.size(); i++) {
        ViewPtr view_cur = views_[i];
        const CVMats& pyr_pre = view_pre->pyramid();
        const CVMats& pyr_cur = view_cur->pyramid();

        cv::calcOpticalFlowPyrLK(pyr_pre, pyr_cur, corners_pre, corners_fwd, status_fwd, err_fwd);
        cv::calcOpticalFlowPyrLK(pyr_cur, pyr_pre, corners_fwd, corners_bwd, status_bwd, err_bwd);

        for (int j = 0; j < num_corners; j++) {
            features.at<float>(2 * i + 0, j) = corners_fwd[j].x;
            features.at<float>(2 * i + 1, j) = corners_fwd[j].y;

            float bidir_err = cv::norm(corners_bwd[j] - corners_pre[j]);
            if (bidir_err > 0.1 || status_fwd[i] == 0 || status_bwd[i] == 0) {
                inlier_mask[j] = false;
            }
        }

        num_inliers = std::accumulate(inlier_mask.begin(), inlier_mask.end(), 0);
        std::cout << i << ". num_inliers = " << num_inliers << std::endl;

        if (num_inliers < min_tracked_points_) {
            if (i < min_tracked_views_) {
                if (callback) callback(CalcState(
                    i, n_views, true, "Too little views/points tracked"
                ));
                stopped_ = true;
                return;
            } else {
                if (callback) callback(CalcState(
                    i, n_views, false, "",
                    inlier_mask, corners_fwd
                ));
                n_views = i;
                inlier_mask = inlier_mask_pre;
                num_inliers = num_inliers_pre;
                break;
            }
        } else {
            if (callback && !callback(CalcState(
                i, n_views, false, "",
                inlier_mask, corners_fwd
            ))) {
                stopped_ = true;
                return;
            }
        }

        view_pre = view_cur;
        corners_pre = std::move(corners_fwd);
        inlier_mask_pre = inlier_mask;
        num_inliers_pre = num_inliers;
    }

    views_kpts_ = cv::Mat(n_views, num_inliers, CV_32FC1);
    for (int r = 0; r < 2 * n_views; r++) {
        int c_inlier = 0;
        for (int c = 0; c < features.cols; c++) {
            if (inlier_mask[c]) {
                assert(c_inlier < num_inliers);
                views_kpts_.at<float>(r, c_inlier++) = features.at<float>(r, c);
            }
        }
    }

    num_kpts_ = num_inliers;
}

struct BACostFunction {
    BACostFunction(double focal_length, double cx, double cy, double u0, double v0, double u,
                   double v)
        : focal_length(focal_length), cx(cx), cy(cy), u0(u0), v0(v0), u(u), v(v) {}

    template <typename T>
    bool operator()(const T* const camera, const T* const point, const T* const intrinsic,
                    T* residuals) const {
        T f = intrinsic[0] * T(focal_length);
        T k1 = intrinsic[1] / T(10.0);
        T k2 = intrinsic[2] / T(10.0);

        T du0_ = T(u0) - T(cx);
        T dv0_ = T(v0) - T(cy);
        T dx0_ = du0_ / f;
        T dy0_ = dv0_ / f;
        T r20_ = dx0_ * dx0_ + dy0_ * dy0_;

        T distort0_ = T(1.0) + k1 * r20_ + k2 * r20_ * r20_;

        T du0 = du0_ * distort0_;
        T dv0 = dv0_ * distort0_;
        T w = point[0];

        T p[3];
        T rx = camera[0], ry = camera[1], rz = camera[2];
        T tx = camera[3], ty = camera[4], tz = camera[5];

        p[0] = du0 - rz * dv0 + f * ry + f * tx * w;
        p[1] = rz * du0 + dv0 - f * rx + f * ty * w;
        p[2] = -ry * du0 + rx * dv0 + f + f * tz * w;

        T up = f * p[0] / p[2];
        T vp = f * p[1] / p[2];

        T du_ = T(u) - T(cx);
        T dv_ = T(v) - T(cy);
        T dx_ = du_ / f;
        T dy_ = dv_ / f;
        T r2_ = dx_ * dx_ + dy_ * dy_;

        T distort_ = T(1.0) + k1 * r2_ + k2 * r2_ * r2_;

        T du = du_ * distort_;
        T dv = dv_ * distort_;

        residuals[0] = du - up;
        residuals[1] = dv - vp;

        return true;
    }

    static ceres::CostFunction* Create(const double focal_length, const double cx, const double cy,
                                       const double u0, const double v0, const double u,
                                       const double v) {
        return (new ceres::AutoDiffCostFunction<BACostFunction, 2, 6, 1, 3>(
            new BACostFunction(focal_length, cx, cy, u0, v0, u, v)));
    }

    double focal_length;
    double cx;
    double cy;
    double u0;
    double v0;
    double u;
    double v;
};

void Calibrator::initPosesDepths() {
    if (poses_ == nullptr) {
        poses_ = new double[6 * num_views_];
        inv_depths_ = new double[num_kpts_];
    }

    memset(poses_, 0, 6 * num_views_ * sizeof(double));

    srand(time(NULL));
    double w_min = 0.01, w_max = 1.0;
    for (int i = 0; i < num_kpts_; i++) {
        inv_depths_[i] = w_min + (w_max - w_min) * double(rand()) / RAND_MAX;
    }
}

void Calibrator::deinitPosesDepths() {
    if (poses_ != nullptr) {
        delete[] poses_;
        delete[] inv_depths_;
        poses_ = nullptr;
        inv_depths_ = nullptr;
    }
}

ceres::CallbackReturnType Calibrator::operator()(const ceres::IterationSummary& summary) {
    if (ba_callback_ && !ba_callback_({summary.iteration, max_ba_iterations_, false, ""})) {
        stopped_ = true;
        return ceres::CallbackReturnType::SOLVER_ABORT;
    }
    return ceres::CallbackReturnType::SOLVER_CONTINUE;
}

void Calibrator::bundleAdjustment(CalcCallback callback) {
    if (stopped_) {
        if (callback) {
            callback(CalcState(0, max_ba_iterations_, true, "Cannot run BA in stopped state"));
        }
        return;
    }
    if (!num_views_ || !num_kpts_) {
        if (callback) {
            callback(CalcState(0, max_ba_iterations_, true, "Too few views or keypoints to run BA"));
        }
        return;
    }

    double f_init = std::max(image_width_, image_height_);
    double k1_init = 0.0;
    double k2_init = 0.0;

    // fix principal point at the center
    cx_ = image_width_ / 2;
    cy_ = image_height_ / 2;

    initPosesDepths();

    double variables[3];
    variables[0] = 1.0;             // scaling factor w.r.t. f_init
    variables[1] = k1_init * 10.0;  // regularize scale
    variables[2] = k2_init * 10.0;  // regularize scale

    ceres::Problem problem;
    for (int i = 0; i < num_kpts_; i++) {
        double u0 = views_kpts_.at<float>(0, i);
        double v0 = views_kpts_.at<float>(1, i);

        for (int j = 1; j < num_views_; j++) {
            double u = views_kpts_.at<float>(j * 2 + 0, i);
            double v = views_kpts_.at<float>(j * 2 + 1, i);

            ceres::CostFunction* cost_function =
                BACostFunction::Create(f_init, cx_, cy_, u0, v0, u, v);
            problem.AddResidualBlock(cost_function, new ceres::HuberLoss(1.0), poses_ + (j * 6),
                                     inv_depths_ + (i), variables);
        }
    }

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::ITERATIVE_SCHUR;
    options.max_num_iterations = max_ba_iterations_;
    //    options.num_threads = 8;
    options.minimizer_progress_to_stdout = true;
    options.callbacks.push_back(this);

    ba_callback_ = callback;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
    std::cout << summary.BriefReport() << "\n";
    ba_callback_ = nullptr;
    
    if (stopped_) {
        return;
    }

    int n_iter = summary.iterations.size() - 1;
    if (callback && !callback(CalcState(n_iter, n_iter + 1))) {
        stopped_ = true;
        return;
    }

    f_ = f_init * variables[0];
    k1_ = variables[1] / 10.0;
    k2_ = variables[2] / 10.0;

    std::cout << "focal length: " << f_ << std::endl;
    std::cout << "radial distortion: " << k1_ << " " << k2_ << std::endl;

    int cnt = 0;
    for (int i = 0; i < num_kpts_; i++) {
        if (inv_depths_[i] < 0) cnt++;
    }
    // when result is flipped
    if (cnt > num_kpts_ - cnt) {
        for (int i = 0; i < num_kpts_; i++) {
            inv_depths_[i] *= -1.0;
        }
        for (int i = 0; i < num_views_; i++) {
            poses_[i * 6 + 3] *= -1.0;
            poses_[i * 6 + 4] *= -1.0;
            poses_[i * 6 + 5] *= -1.0;
        }
    }
}

void Calibrator::calcUndistParams(CalcCallback callback) {
    using namespace cv;

    // find 'f_new' to keep every pixel
    double min_x = 0, max_x = 0;
    double min_y = 0, max_y = 0;

    for (int i = 0; i < image_height_; i++) {
        for (int j = 0; j < image_width_; j++) {
            double xij = (j - cx_) / f_;
            double yij = (i - cy_) / f_;
            double r2ij = xij * xij + yij * yij;
            double rad = 1 + k1_ * r2ij + k2_ * r2ij * r2ij;
            double xij_ = xij * rad;
            double yij_ = yij * rad;
            if (xij_ < min_x) min_x = xij_;
            if (xij_ > max_x) max_x = xij_;
            if (yij_ < min_y) min_y = yij_;
            if (yij_ > max_y) max_y = yij_;
        }
    }
    double f_min_x = -cx_ / min_x;
    double f_max_x = (image_width_ - cx_) / max_x;
    double f_min_y = -cy_ / min_y;
    double f_max_y = (image_height_ - cy_) / max_y;
    double tempx = (f_min_x < f_max_x) ? f_min_x : f_max_x;
    double tempy = (f_min_y < f_max_y) ? f_min_y : f_max_y;
    f_new_ = (tempx < tempy) ? tempx : tempy;

    // meshgrid
    float* x_ = new float[image_height_ * image_width_];
    float* y_ = new float[image_height_ * image_width_];
    for (int i = 0; i < image_height_; i++) {
        for (int j = 0; j < image_width_; j++) {
            x_[i * image_width_ + j] = (j - cx_) / f_new_;
            y_[i * image_width_ + j] = (i - cy_) / f_new_;
        }
    }
    Mat x = Mat(image_height_, image_width_, CV_32F, x_);
    Mat y = Mat(image_height_, image_width_, CV_32F, y_);
    Mat x_bak = x.clone();
    Mat y_bak = y.clone();

    if (callback && !callback(CalcState(0, max_undist_iterations_))) {
        stopped_ = true;
        return;
    }

    // iteratively find the inverse mapping
    for (int i = 0; i < max_undist_iterations_; i++) {
        Mat x1 = x.clone();
        Mat y1 = y.clone();
        Mat x2 = x1.mul(x1);
        Mat y2 = y1.mul(y1);
        Mat x3 = x2.mul(x1);
        Mat y3 = y2.mul(y1);
        Mat r2 = x2 + y2;
        Mat r4 = r2.mul(r2);
        Mat r2x = 2 * x1;
        Mat r2y = 2 * y1;
        Mat r4x = (4 * x3) + (4 * x1.mul(y2));
        Mat r4y = (4 * x2.mul(y1)) + (4 * y3);
        Mat dist = 1.0 + (k1_ * r2) + (k2_ * r4);
        Mat x__ = x1.mul(dist);
        Mat y__ = y1.mul(dist);
        Mat rx = x_bak - x__;
        Mat ry = y_bak - y__;
        Mat rxx = -1 - (k1_ * r2) - (k2_ * r4) - (k1_ * x1.mul(r2x)) - (k2_ * x1.mul(r4x));
        Mat rxy = -(k1_ * x1.mul(r2y)) - (k2_ * x1.mul(r4y));
        Mat ryx = -(k1_ * y1.mul(r2x)) - (k2_ * y1.mul(r4x));
        Mat ryy = -1 - (k1_ * r2) - (k2_ * r4) - (k1_ * y1.mul(r2y)) - (k2_ * y1.mul(r4y));
        Mat a = rxx.mul(rxx) + ryx.mul(ryx);
        Mat b = rxx.mul(rxy) + ryx.mul(ryy);
        Mat c = b;
        Mat d = rxy.mul(rxy) + ryy.mul(ryy);
        Mat det = (a.mul(d)) - (b.mul(c));
        Mat dx =
            -(1 / det).mul((d.mul(rxx) - b.mul(rxy)).mul(rx) + (d.mul(ryx) - b.mul(ryy)).mul(ry));
        Mat dy =
            -(1 / det).mul((-c.mul(rxx) + a.mul(rxy)).mul(rx) + (-c.mul(ryx) + a.mul(ryy)).mul(ry));
        x = x + dx;
        y = y + dy;
        
        if (callback && !callback(CalcState(i, max_undist_iterations_))) {
            stopped_ = true;
            return;
        }
    }
    Mat udx = (f_ * x + cx_);
    ud_mapx_ = udx.clone();
    Mat udy = (f_ * y + cy_);
    ud_mapy_ = udy.clone();

    delete[] x_;
    delete[] y_;
}

void Calibrator::undistort(cv::Mat img_src, cv::Mat img_dst) {
    cv::remap(img_src, img_dst, ud_mapx_, ud_mapy_, cv::INTER_CUBIC);
}
