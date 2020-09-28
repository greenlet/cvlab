#include "cashash_matcher.h"

#include "opencv2/core/eigen.hpp"
#include "utils.h"


MatchesPtr CashashMatcher::match(View &view1, View &view2) {
  view1.calcKeypoints();
  view2.calcKeypoints();
  
  MatchesPtr res;
  const CVDescriptors &desc1 = view1.descriptors();
  const CVDescriptors &desc2 = view2.descriptors();
  if (desc1.rows == 0 || desc2.rows == 0) {
    return res;
  }

  ORBCascadeHasher::FeaturesMat features1(desc1.rows, desc1.cols);
  ORBCascadeHasher::FeaturesMat features2(desc2.rows, desc2.cols);

  std::cout << "Descriptors: " << cvmat2str(view1.descriptors()) << std::endl;

  cv2eigen(view1.descriptors(), features1);
  cv2eigen(view2.descriptors(), features2);

  std::cout << "Features w x h: " << features1.cols() << " x " << features1.rows() << std::endl;

  ORBCascadeHasher::ContainerPtr container1 = cascade_hasher_.make_hash(std::move(features1));
  ORBCascadeHasher::ContainerPtr container2 = cascade_hasher_.make_hash(std::move(features2));
  ORBCascadeHasher::Matches matches;
  container1->match(*container2, matches);
  std::cout << "Matches: " << matches.size() << std::endl;
  
  res = std::make_shared<Matches>();
  res->reserve(matches.size());
  for (const auto &match : matches) {
    res->emplace_back(match.feature1_id, match.feature2_id, match.distance);
  }

  return res;
}

