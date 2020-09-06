#pragma once
#include "common.h"

template <unsigned HashSize = 128, unsigned FeatureSize = 128, unsigned BucketGroupsSize = 6,
          unsigned BucketHashSize = 10>
class CascadeHasher {
 public:
  using FeatureId = unsigned;
  using BucketId = unsigned;
  using BucketFeatureId = unsigned;
  using HashSizeT = unsigned;
  using BucketHashSizeT = unsigned;
  using FeatureSizeT = unsigned;
  using Feature = Eigen::Matrix<float, 1, FeatureSize>;
  using FeatureHashVec = Eigen::Matrix<float, HashSize, 1>;
  using FeatureBucketHashVec = Eigen::Matrix<float, BucketHashSize, 1>;
  using FeaturesMat = Eigen::Matrix<float, Eigen::Dynamic, FeatureSize>;
  using HashFeatureMat = Eigen::Matrix<float, HashSize, FeatureSize>;
  using BucketHashFeatureMat = Eigen::Matrix<float, BucketHashSize, FeatureSize>;
  using BucketGroupsHashFeatureMats = std::array<BucketHashFeatureMat, BucketGroupsSize>;
  using FeatureHash = std::bitset<HashSize>;
  using BucketGroupsBucketIds = std::array<BucketId, BucketGroupsSize>;
  struct FeatureHashBucketIds {
    FeatureHash feature_hash;
    BucketGroupsBucketIds bucket_groups_bucket_ids;
  };
  using FeaturesHashesBucketIds = std::vector<FeatureHashBucketIds>;
  using FeaturesBucket = std::vector<FeatureId>;
  using FeaturesBucketGroup = std::array<FeaturesBucket, 1 << BucketHashSize>;
  using FeaturesBucketGroups = std::array<FeaturesBucketGroup, BucketGroupsSize>;
  using FeaturesHammingDistances = Eigen::Matrix<FeatureId, Eigen::Dynamic, HashSize + 1>;
  using FeaturesNumHummingDistances = std::array<unsigned, HashSize + 1>;
  struct Match {
    FeatureId feature1_id;
    FeatureId feature2_id;
    float distance;
  };
  using Matches = std::vector<Match>;

  class Container {
    Container(FeaturesMat &&features, FeaturesHashesBucketIds &&features_hashes_bucket_ids,
              FeaturesBucketGroups &&features_bucket_groups)
        : features_(std::move(features)),
          features_hashes_bucket_ids_(std::move(features_hashes_bucket_ids)),
          features_bucket_groups_(std::move(features_bucket_groups)) {}

    void match(const Container &other, Matches &matches, unsigned NN = 1) {
      matches.clear();

      FeaturesHammingDistances features_hamming_distances(features_hashes_bucket_ids_.size());
      FeaturesNumHummingDistances features_num_hamming_distances;
      const unsigned kMaxNearestFeatures = 10;
      std::vector<std::pair<float, FeatureId>> dist_feature_candidates;
      dist_feature_candidates.reserve(kMaxNearestFeatures);
      features_hamming_distances.setZero();

      unsigned features_offset = 0;
      for (FeatureId feature1_id = 0; feature1_id < other.features_hashes_bucket_ids_.size();
           feature1_id++) {
        FeatureHash &feature1_hash = other.features_[feature1_id];
        features_num_hamming_distances.fill(0);
        dist_feature_candidates.clear();

        BucketGroupsBucketIds &other_bgb_ids =
            other.features_hashes_bucket_ids_[feature1_id].bucket_groups_bucket_ids;
        for (unsigned i_bgroup = 0; i_bgroup < BucketGroupsSize; i_bgroup++) {
          BucketId bucket_id = other_bgb_ids[i_bgroup];

          for (FeatureId feature2_id : features_bucket_groups_[i_bgroup][bucket_id]) {
            FeatureHash &feature2_hash = features_[feature2_id];
            unsigned ham_dist = (feature1_hash ^ feature2_hash).count();
            unsigned i_feature2 = features_num_hamming_distances[ham_dist]++;
            features_hamming_distances(i_feature2, ham_dist) = feature2_id;
          }
        }

        unsigned features_offset_delta = 0;

        for (unsigned ham_dist = 0;
             ham_dist <= HashSize && dist_feature_candidates.size() <= kMaxNearestFeatures;
             ham_dist++) {
          unsigned num_features2 = features_num_hamming_distances[ham_dist];
          features_offset_delta += num_features2;

          Feature feature1 = other.features_[feature1_id];
          for (unsigned i_feature2 = 0;
               i_feature2 < num_features2 && dist_feature_candidates.size() <= kMaxNearestFeatures;
               i_feature2++) {
            FeatureId feature2_id = features_hamming_distances(i_feature2, ham_dist);
            Feature feature2 = other.features_[feature2_id];
            float dist = (feature1 - feature2).norm();
            dist_feature_candidates.emplace_back(dist, feature2_id);
          }
        }

        if (dist_feature_candidates.size() >= NN) {
          std::partial_sort(dist_feature_candidates.begin(), dist_feature_candidates.begin() + NN,
                            dist_feature_candidates.end());
          for (unsigned i_match = 0; i_match < NN; i_match++) {
            auto [cand_dist, cand_feature_id] = dist_feature_candidates[i_match];
            matches.emplace(feature1_id, cand_feature_id, cand_dist);
          }
        }

        features_offset += features_offset_delta;
      }
    }

   private:
    FeaturesMat features_;
    FeaturesHashesBucketIds features_hashes_bucket_ids_;
    FeaturesBucketGroups features_bucket_groups_;
  };

  using ContainerPtr = std::shared_ptr<Container>;

  CascadeHasher() {
    std::mt19937 gen(std::mt19937::default_seed);
    std::normal_distribution<> d(0, 1);

    for (HashSizeT i_hash = 0; i_hash < HashSize; i_hash++) {
      for (FeatureSizeT i_feat = 0; i_feat < FeatureSize; i_feat++) {
        hash_feature_mat_(i_hash, i_feat) = d(gen);
      }
    }

    for (unsigned i_bgroup = 0; i_bgroup < BucketGroupsSize; i_bgroup++) {
      for (BucketHashSizeT i_bhash; i_bhash < BucketHashSize; i_bhash++) {
        for (FeatureSizeT i_feat = 0; i_feat < FeatureSize; i_feat++) {
          bucket_groups_hash_feature_mats_[i_bgroup](i_bhash, i_feat) = d(gen);
        }
      }
    }
  }

  ContainerPtr make_hash(FeaturesMat &&features) {
    Feature features_mean = features.colwise().mean();
    unsigned n_features = features.rows();
    FeaturesHashesBucketIds features_hashes_bucket_ids(n_features);
    FeaturesBucketGroups features_bucket_groups;

    for (FeatureId feature_id = 0; feature_id < n_features; feature_id++) {
      Feature feature = features.row(feature_id) - features_mean;
      FeatureHashVec feature_hash_vec = hash_feature_mat_ * feature.transpose();
      FeatureHash &feature_hash = features_hashes_bucket_ids[feature_id].feature_hash;
      BucketGroupsBucketIds &bucket_groups_bucket_ids =
          features_hashes_bucket_ids[feature_id].bucket_groups_bucket_ids;

      for (HashSizeT i_hash = 0; i_hash < HashSize; i_hash++) {
        feature_hash[i_hash] = feature_hash_vec[i_hash] > 0;
      }

      for (unsigned i_bgroup = 0; i_bgroup < BucketGroupsSize; i_bgroup++) {
        FeatureBucketHashVec feature_bhash_vec =
            bucket_groups_hash_feature_mats_[i_bgroup] * feature;
        BucketId bucket_id = 0;
        for (BucketHashSizeT i_hash = 0; i_hash < BucketHashSize; i_hash++) {
          bucket_id = (bucket_id << 1) + (feature_bhash_vec[i_hash] > 0 ? 1 : 0);
        }

        bucket_groups_bucket_ids[i_bgroup] = bucket_id;
        features_bucket_groups[i_bgroup][bucket_id].push_back(feature_id);
      }
    }

    return std::make_shared<Container>(std::move(features), std::move(features_hashes_bucket_ids),
                                       std::move(features_bucket_groups));
  }

 private:
  HashFeatureMat hash_feature_mat_;
  BucketGroupsHashFeatureMats bucket_groups_hash_feature_mats_;
};
