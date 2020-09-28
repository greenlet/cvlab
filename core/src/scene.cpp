#include "scene.h"

Scene::Scene() : next_view_id_(0) {}

ViewPtr Scene::processImage(cv::Mat img) {
  ViewPtr view = std::make_shared<View>(next_view_id_++, img);

  view->calcKeypoints();

  // klt_tracker_.processView(view);

  views_.push_back(view);

  if (views_.size() > 10) {
    views_.erase(views_.begin());
    std::cout << "After erasure: " << views_.size() << std::endl;
  }

  MatchesPtr matches;
  if (views_.size() > 1) {
    matches = cashash_matcher_.match(*views_[nv - 2], *views_[nv - 1]);
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

      if (kpts_pre.size() > 100) {
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
      std::cout << "After filtering: " << matches_->size() << std::endl;
    }
  }

  return view;
}

cv::Mat Scene::visualizeMatches() {
  cv::Mat res;
  if (!matches_ || matches_->size() == 0) {
    return res;
  }

  ViewPtr view = views_[views_.size() - 1];
  CVKeyPoints keypoints;
  keypoints.reserve(matches_->size());
  for (const Match &match : *matches_) {
    KeyPointId kpt_id = std::get<1>(match);
    const cv::KeyPoint &kpt = view->keypoints()[kpt_id];
    keypoints.push_back(kpt);
  }

  return view->visualizeKeypoints(keypoints);
}
