#include "scene.h"

Scene::Scene() : next_view_id_(0) {}

ViewPtr Scene::processImage(cv::Mat img) {
    ViewPtr view = std::make_shared<View>(next_view_id_++, img);

    view->calcORBFeaturesCollection(500);

    // klt_tracker_.processView(view);

    views_.push_back(view);

    if (views_.size() > 10) {
        views_.erase(views_.begin());
        std::cout << "After erasure: " << views_.size() << std::endl;
    }

    MatchesPtr matches;
    size_t nv = views_.size();
    if (nv > 1) {
        matches = cashash_matcher_.match(*views_[nv - 2], *views_[nv - 1]);

        std::cout << std::setprecision(4) << std::fixed;
        for (int i = 0; i < std::min((int)matches->size(), 100); i++) {
            const CVKeyPoints &kpts1 = views_[nv - 2]->orb_features_collection()->keypoints();
            const CVKeyPoints &kpts2 = views_[nv - 1]->orb_features_collection()->keypoints();
            cv::Mat descs1 = views_[nv - 2]->orb_features_collection()->descriptors();
            cv::Mat descs2 = views_[nv - 1]->orb_features_collection()->descriptors();
            const Match &match = (*matches)[i];
            int i1 = std::get<0>(match);
            int i2 = std::get<1>(match);
            cv::KeyPoint kpt1 = kpts1[i1];
            cv::KeyPoint kpt2 = kpts2[i2];
            cv::Mat desc1 = descs1.row(i1);
            cv::Mat desc2 = descs2.row(i2);

            std::cout << "(" << cv::norm(kpt1.pt - kpt2.pt) << " - " << cv::norm(desc1, desc2)
                      << ") " << std::endl;
        }
    }

    if (matches) {
        std::cout << "Views: " << views_[nv - 2]->id() << " - " << views_[nv - 1]->id() << " : "
                  << matches->size() << std::endl;

        if (!matches_) {
            matches_ = matches;
        } else {
            std::unordered_set<KeyPointId> kpts_pre;
            std::unordered_set<KeyPointId> kpts_cur;
            for (const auto &match : *matches_) {
                kpts_pre.insert(std::get<1>(match));
            }
            for (const Match &match : *matches) {
                KeyPointId kpt_id = std::get<0>(match);
                if (kpts_pre.count(kpt_id)) {
                    kpts_cur.insert(kpt_id);
                }
            }

            if (kpts_cur.size() >= 10) {
                matches_->clear();
                for (const Match &match : *matches) {
                    KeyPointId kpt_id = std::get<0>(match);
                    if (kpts_cur.count(kpt_id)) {
                        matches_->push_back(match);
                    }
                }
            } else {
                matches_ = matches;
            }

            assert(matches_->size() <=
                   views_[nv - 2]->orb_features_collection()->keypoints().size());
            assert(matches_->size() <=
                   views_[nv - 1]->orb_features_collection()->keypoints().size());

            std::cout << "After filtering: " << matches_->size() << std::endl;
        }
    } else {
        matches_ = matches;
    }

    return view;
}

cv::Mat Scene::visualizeMatches() {
    cv::Mat res;
    if (!matches_ || matches_->size() == 0) {
        return res;
    }

    // ViewPtr view = views_[views_.size() - 1];
    // CVKeyPoints keypoints;
    // keypoints.reserve(matches_->size());
    // for (const Match &match : *matches_) {
    //     KeyPointId kpt_id = std::get<1>(match);
    //     const cv::KeyPoint &kpt = view->orb_features_collection()->keypoints()[kpt_id];
    //     keypoints.push_back(kpt);
    // }

    // return view->visualizeKeypoints(keypoints);

    ViewPtr view1 = views_[views_.size() - 2];
    ViewPtr view2 = views_[views_.size() - 1];
    view2->img().copyTo(res);

    for (const Match &match : *matches_) {
        KeyPointId kpt1_id = std::get<0>(match);
        KeyPointId kpt2_id = std::get<1>(match);
        const cv::KeyPoint &kpt1 = view1->orb_features_collection()->keypoints()[kpt1_id];
        const cv::KeyPoint &kpt2 = view2->orb_features_collection()->keypoints()[kpt2_id];
        cv::circle(res, kpt1.pt, 5, cv::Scalar(200, 0, 0), cv::FILLED, 8, 0);
        cv::circle(res, kpt2.pt, 3, cv::Scalar(0, 0, 200), cv::FILLED, 8, 0);
        cv::line(res, kpt1.pt, kpt2.pt, cv::Scalar(0, 200, 0));
    }

    return res;
}
