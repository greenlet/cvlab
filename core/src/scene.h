#pragma once

#include "common.h"
#include "matching/cashash_matcher.h"
#include "tracking/klt_tracker.h"
#include "view.h"

class Scene {
  public:
    Scene();

    ViewPtr processImage(cv::Mat img);
    cv::Mat visualizeMatches();

    const std::vector<ViewPtr>& views() { return views_; }
    const KLTTracker& klt_tracker() const { return klt_tracker_; }
    const CashashMatcher& cashash_matcher() const { return cashash_matcher_; }

  private:
    int next_view_id_;
    std::vector<ViewPtr> views_;
    KLTTracker klt_tracker_;
    CashashMatcher cashash_matcher_;
    MatchesPtr matches_;
};
