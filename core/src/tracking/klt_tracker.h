#pragma once

#include "common.h"
#include "view.h"

class KLTTracker {
   public:
    KLTTracker();

    void processView(ViewPtr view);

    const CVKeyPoints& tracked_keypoints() const { return tracked_keypoints_; }
    const CVPoints2f& tracked_points() const { return tracked_points_; }

   private:
    ViewPtr last_view_;
    CVKeyPoints tracked_keypoints_;
    CVPoints2f tracked_points_;
};
