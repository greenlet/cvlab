#pragma once

#include "common.h"
#include "matching/matcher.h"
#include "matching/cascade_hasher.h"

class CashashMatcher : public Matcher {
 public:
  using ORBCascadeHasher = CascadeHasher<128, 32>;

  MatchesPtr match(View &view1, View &view2) override;

 private:
  ORBCascadeHasher cascade_hasher_;
};
