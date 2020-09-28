#include "common.h"
#include "view.h"


using Match = std::tuple<KeyPointId, KeyPointId, float>;
using Matches = std::vector<Match>;
using MatchesPtr = std::shared_ptr<Matches>;

class Matcher {
 public:
  virtual MatchesPtr match(View &view1, View &view2) = 0;
};
