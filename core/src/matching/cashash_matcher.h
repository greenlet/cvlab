#pragma once

#include "common.h"
#include "matching/cascade_hasher.h"
#include "matching/matcher.h"

class CashashMatcher : public Matcher {
  public:
    MatchesPtr match(View &view1, View &view2) override;

  private:
    ORBCascadeHasher cascade_hasher_;
};
