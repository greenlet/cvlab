#pragma once

#include "common.h"
#include "matching/cascade_hasher.h"
#include "view.h"

class CasHashTracker {
 public:
  using ORBCascadeHasher = CascadeHasher<128, 32>;

  void processView(ViewPtr view);

 private:
  ORBCascadeHasher cascade_hasher_;
  ORBCascadeHasher::ContainerPtr last_container_;
};
