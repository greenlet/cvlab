#include "calibrator.h"

#include <ceres/ceres.h>

Calibrator::Calibrator() : num_views_(0), num_kpts_(0), img_size_{0, 0} {}

Calibrator::~Calibrator() {
    deinitPosesDepths();
}

void Calibrator::addView(ViewPtr view) {
    views_.push_back(view);
    num_views_++;
    if (num_views_ == 1) {
        img_size_ = views_[0]->img().size();
    } else {
        assert(img_size_ == views_[num_views_ - 1]->img().size());
    }
}

void Calibrator::calc() {
    if (views_.size() <= 1) {
        std::cout << "Cannot start calibration. No or too little views added (" << views_.size()
                  << ")" << std::endl;
        return;
    }
    track();
    bundleAdjustment();
}

void Calibrator::track() {
    int max_corners = 20000;
    int n_views = views_.size();

    ViewPtr view_pre = views_[0];
    const cv::Mat &img_gray_pre = view_pre->img_grayscale();
    std::vector<cv::Point2f> corners_pre;
    cv::goodFeaturesToTrack(view_pre->img_grayscale(), corners_pre, max_corners, 1e-10, 10);
    int num_corners = corners_pre.size();

    std::cout << "num_corners = " << num_corners << std::endl;

    bool inlier_mask[num_corners];
    cv::Mat features = cv::Mat(2 * n_views, num_corners, CV_32FC1);
    for (int j = 0; j < num_corners; j++) {
        features.at<float>(0, j) = corners_pre[j].x;
        features.at<float>(1, j) = corners_pre[j].y;
        inlier_mask[j] = true;
    }

    std::vector<cv::Point2f> corners_fwd;
    std::vector<cv::Point2f> corners_bwd;
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
    for (int i = 1; i < views_.size(); i++) {
        ViewPtr view_cur = views_[i];
        const CVMats &pyr_pre = view_pre->pyramid();
        const CVMats &pyr_cur = view_cur->pyramid();

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

        view_pre = view_cur;
        corners_pre = std::move(corners_fwd);

        num_inliers = std::accumulate(inlier_mask, inlier_mask + num_corners, 0);
        std::cout << i << ". num_inliers = " << num_inliers << std::endl;
    }

    if (num_inliers > 0) {
        views_kpts_ = cv::Mat(features.rows, num_inliers, CV_32FC1);
        for (int r = 0; r < features.rows; r++) {
            int c_inlier = 0;
            for (int c = 0; c < features.cols; c++) {
                if (inlier_mask[c]) {
                    assert(c_inlier < num_inliers);
                    views_kpts_.at<float>(r, c_inlier++) = features.at<float>(r, c);
                }
            }
        }
    }

    num_kpts_ = num_inliers;
}


struct BACostFunction {
    BACostFunction(double focal_length, double cx, double cy, double u0, double v0, double u, double v)
    : focal_length(focal_length), cx(cx), cy(cy), u0(u0), v0(v0), u(u), v(v) {}
    
    template <typename T>
    bool operator()(const T* const camera,
                    const T* const point,
                    const T* const intrinsic,
                    T* residuals) const {
        
        T f=intrinsic[0]*T(focal_length);
        T k1=intrinsic[1]/T(10.0);
        T k2=intrinsic[2]/T(10.0);
        
        T du0_= T(u0)-T(cx);
        T dv0_= T(v0)-T(cy);
        T dx0_= du0_/f;
        T dy0_= dv0_/f;
        T r20_= dx0_*dx0_ + dy0_*dy0_;
        
        T distort0_=T(1.0) + k1*r20_ + k2*r20_*r20_;
        
        T du0=du0_*distort0_;
        T dv0=dv0_*distort0_;
        T w=point[0];
        
        T p[3];
        T rx=camera[0], ry=camera[1], rz=camera[2];
        T tx=camera[3], ty=camera[4], tz=camera[5];
        
        p[0] = du0		- rz*dv0	+ f*ry	+ f*tx*w;
        p[1] = rz*du0	+ dv0		- f*rx	+ f*ty*w;
        p[2] = -ry*du0	+ rx*dv0	+ f		+ f*tz*w;
        
        T up = f * p[0] / p[2];
        T vp = f * p[1] / p[2];
        
        T du_ = T(u)-T(cx);
        T dv_ = T(v)-T(cy);
        T dx_ = du_/f;
        T dy_ = dv_/f;
        T r2_ = dx_*dx_ + dy_*dy_;
        
        T distort_=T(1.0) + k1*r2_ + k2*r2_*r2_;
        
        T du=du_*distort_;
        T dv=dv_*distort_;
        
        residuals[0] = du - up;
        residuals[1] = dv - vp;
        
        return true;
    }
    
    static ceres::CostFunction* Create(const double focal_length, const double cx, const double cy,
                                       const double u0, const double v0,
                                       const double u, const double v) {
        return (new ceres::AutoDiffCostFunction<BACostFunction, 2, 6, 1, 3>(new BACostFunction(focal_length,cx,cy,u0,v0,u,v)));
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


void Calibrator::bundleAdjustment() {
    if (!num_views_ || !num_kpts_) {
        return;
    }

    double f_init = std::max(img_size_.width, img_size_.height);
    double k1_init = 0.0;
    double k2_init = 0.0;

    // fix principal point at the center
    cx_ = img_size_.width / 2;
    cy_ = img_size_.height / 2;

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
            problem.AddResidualBlock(cost_function, new ceres::HuberLoss(1.0),
                                     poses_ + (j * 6), inv_depths_ + (i), variables);
        }
    }

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::ITERATIVE_SCHUR;
    options.max_num_iterations = 100;
    //    options.num_threads = 8;
    options.minimizer_progress_to_stdout = true;

    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
    std::cout << summary.BriefReport() << "\n";

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
