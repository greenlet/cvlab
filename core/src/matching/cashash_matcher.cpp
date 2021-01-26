#include "cashash_matcher.h"

#include "opencv2/core/eigen.hpp"
#include "utils.h"

MatchesPtr CashashMatcher::match(View &view1, View &view2) {

    // TODO: Move features_collection out of here. Only ids and features make sense
    MatchesPtr res;
    const ORBFeaturesCollectionPtr &feat_coll1 = view1.orb_features_collection();
    const ORBFeaturesCollectionPtr &feat_coll2 = view2.orb_features_collection();
    if (feat_coll1->empty() || feat_coll2->empty()) {
        return res;
    }

    std::cout << "Features1 w x h: " << feat_coll1->features()->cols() << " x " << feat_coll1->features()->rows() << std::endl;
    std::cout << "Features2 w x h: " << feat_coll2->features()->cols() << " x " << feat_coll2->features()->rows() << std::endl;

    ORBCascadeHasher::ContainerPtr container1 = cascade_hasher_.make_hash(feat_coll1->features());
    ORBCascadeHasher::ContainerPtr container2 = cascade_hasher_.make_hash(feat_coll2->features());
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
