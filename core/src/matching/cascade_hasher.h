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
    using Features_Mat = Eigen::Matrix<float, Eigen::Dynamic, FeatureSize>;
    using Features_MatPtr = std::shared_ptr<Features_Mat>;
    using RandomFeature_To_FullHash_Mat = Eigen::Matrix<float, HashSize, FeatureSize>;
    using RandomFeature_To_BucketHash_Mat = Eigen::Matrix<float, BucketHashSize, FeatureSize>;
    using BucketGroup_To_BucketHashing_Arr =
        std::array<RandomFeature_To_BucketHash_Mat, BucketGroupsSize>;
    using FeatureHash_Bitset = std::bitset<HashSize>;
    using BucketGroup_To_BucketId_Arr = std::array<BucketId, BucketGroupsSize>;
    struct FeatureHashAndBuckets_Struct {
        FeatureHash_Bitset feature_hash;
        BucketGroup_To_BucketId_Arr bucket_group_to_bucket_id;
    };
    using FeatureId_To_HashAndBuckets_Vec = std::vector<FeatureHashAndBuckets_Struct>;
    using FeatureIds_Vec = std::vector<FeatureId>;
    using BucketHash_To_FeatureIds_Arr = std::array<FeatureIds_Vec, 1 << BucketHashSize>;
    // using BucketHash_To_FeatureIds_Arr = std::unordered_map<BucketId, FeatureIds_Vec>;
    using BucketGroup_To_BucketHashes_Arr =
        std::array<BucketHash_To_FeatureIds_Arr, BucketGroupsSize>;
    using Feature_To_HammingDistances_Mat = Eigen::Matrix<FeatureId, Eigen::Dynamic, HashSize + 1>;
    using HummingDistance_To_FeaturesNum_Arr = std::array<unsigned, HashSize + 1>;
    struct Match {
        FeatureId feature1_id;
        FeatureId feature2_id;
        float distance;

        Match(FeatureId feature1_id, FeatureId feature2_id, float distance)
            : feature1_id(feature1_id), feature2_id(feature2_id), distance(distance) {}
    };
    using Matches = std::vector<Match>;

    class Container {
      public:
        void match(Container &other, Matches &matches, unsigned NN = 1) {
            matches.clear();

            Feature_To_HammingDistances_Mat feature_to_hamming_distances(
                feature_id_to_hash_and_buckets_.size(), HashSize + 1);
            HummingDistance_To_FeaturesNum_Arr hamming_distance_to_features_num;
            const unsigned kMaxNearestFeatures = 10;
            std::vector<std::pair<float, FeatureId>> dist_feature_candidates;
            dist_feature_candidates.reserve(kMaxNearestFeatures);
            feature_to_hamming_distances.setZero();
            std::vector<bool> used_features(feature_id_to_hash_and_buckets_.size());

            for (FeatureId feature2_id = 0;
                 feature2_id < other.feature_id_to_hash_and_buckets_.size(); feature2_id++) {
                FeatureHash_Bitset &feature2_hash =
                    other.feature_id_to_hash_and_buckets_[feature2_id].feature_hash;
                hamming_distance_to_features_num.fill(0);
                dist_feature_candidates.clear();

                BucketGroup_To_BucketId_Arr &other_bgb_ids =
                    other.feature_id_to_hash_and_buckets_[feature2_id].bucket_group_to_bucket_id;
                for (unsigned i_bgroup = 0; i_bgroup < BucketGroupsSize; i_bgroup++) {
                    BucketId bucket_id = other_bgb_ids[i_bgroup];

                    for (FeatureId feature1_id :
                         bucket_group_to_bucket_hashes_[i_bgroup][bucket_id]) {
                        if (used_features[feature1_id]) {
                            continue;
                        }
                        used_features[feature1_id] = true;
                        FeatureHash_Bitset &feature1_hash =
                            feature_id_to_hash_and_buckets_[feature1_id].feature_hash;
                        unsigned ham_dist = (feature2_hash ^ feature1_hash).count();
                        unsigned i_feature1 = hamming_distance_to_features_num[ham_dist]++;
                        feature_to_hamming_distances(i_feature1, ham_dist) = feature1_id;
                    }
                }

                for (unsigned i_bgroup = 0; i_bgroup < BucketGroupsSize; i_bgroup++) {
                    BucketId bucket_id = other_bgb_ids[i_bgroup];
                    for (FeatureId feature1_id :
                         bucket_group_to_bucket_hashes_[i_bgroup][bucket_id]) {
                        used_features[feature1_id] = false;
                    }
                }

                for (unsigned ham_dist = 0;
                     ham_dist <= HashSize && dist_feature_candidates.size() <= kMaxNearestFeatures;
                     ham_dist++) {
                    unsigned num_features1 = hamming_distance_to_features_num[ham_dist];

                    Feature feature2 = other.features_->row(feature2_id);
                    for (unsigned i_feature1 = 0;
                         i_feature1 < num_features1 &&
                         dist_feature_candidates.size() <= kMaxNearestFeatures;
                         i_feature1++) {
                        FeatureId feature1_id = feature_to_hamming_distances(i_feature1, ham_dist);
                        Feature feature1 = features_->row(feature1_id);
                        float dist = (feature2 - feature1).norm();
                        dist_feature_candidates.emplace_back(dist, feature1_id);
                    }
                }

                if (dist_feature_candidates.size() >= NN) {
                    std::partial_sort(dist_feature_candidates.begin(),
                                      dist_feature_candidates.begin() + NN,
                                      dist_feature_candidates.end());
                    for (unsigned i_match = 0; i_match < NN; i_match++) {
                        const auto &cand = dist_feature_candidates[i_match];
                        matches.emplace_back(cand.second, feature2_id, cand.first);
                    }
                }
            }

            std::sort(matches.begin(), matches.end(), [](const Match &m1, const Match &m2) {
                return m1.feature1_id == m2.feature1_id ? m1.distance < m2.distance
                                                        : m1.feature1_id < m2.feature1_id;
            });
            auto last = std::unique(
                matches.begin(), matches.end(),
                [](const Match &m1, const Match &m2) { return m1.feature1_id == m2.feature1_id; });
            matches.erase(last, matches.end());
        }

        Container(Features_MatPtr features,
                  FeatureId_To_HashAndBuckets_Vec &&feature_id_to_hash_and_buckets,
                  BucketGroup_To_BucketHashes_Arr &&bucket_group_to_bucket_hashes)
            : features_(features),
              feature_id_to_hash_and_buckets_(std::move(feature_id_to_hash_and_buckets)),
              bucket_group_to_bucket_hashes_(std::move(bucket_group_to_bucket_hashes)) {}

      private:
        Features_MatPtr features_;
        FeatureId_To_HashAndBuckets_Vec feature_id_to_hash_and_buckets_;
        BucketGroup_To_BucketHashes_Arr bucket_group_to_bucket_hashes_;
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

    ContainerPtr make_hash(Features_MatPtr features) {
        Feature features_mean = features->colwise().mean();
        unsigned n_features = features->rows();
        FeatureId_To_HashAndBuckets_Vec feature_id_to_hash_and_buckets(n_features);
        BucketGroup_To_BucketHashes_Arr bucket_group_to_bucket_hashes;

        for (FeatureId feature_id = 0; feature_id < n_features; feature_id++) {
            Feature feature = features->row(feature_id) - features_mean;
            Eigen::Matrix<float, HashSize, 1> feature_hash_vec =
                hash_feature_mat_ * feature.transpose();
            FeatureHash_Bitset &feature_hash =
                feature_id_to_hash_and_buckets[feature_id].feature_hash;
            for (HashSizeT i_hash = 0; i_hash < HashSize; i_hash++) {
                feature_hash[i_hash] = feature_hash_vec[i_hash] > 0;
            }

            BucketGroup_To_BucketId_Arr &bucket_group_to_bucket_id =
                feature_id_to_hash_and_buckets[feature_id].bucket_group_to_bucket_id;

            for (unsigned i_bgroup = 0; i_bgroup < BucketGroupsSize; i_bgroup++) {
                Eigen::Matrix<float, BucketHashSize, 1> feature_bhash_vec =
                    bucket_groups_hash_feature_mats_[i_bgroup] * feature.transpose();
                BucketId bucket_id = 0;
                for (BucketHashSizeT i_hash = 0; i_hash < BucketHashSize; i_hash++) {
                    bucket_id = (bucket_id << 1) + (feature_bhash_vec[i_hash] > 0 ? 1 : 0);
                }

                bucket_group_to_bucket_id[i_bgroup] = bucket_id;
                bucket_group_to_bucket_hashes[i_bgroup][bucket_id].push_back(feature_id);
            }
        }

        return std::make_shared<Container>(features, std::move(feature_id_to_hash_and_buckets),
                                           std::move(bucket_group_to_bucket_hashes));
    }

  private:
    RandomFeature_To_FullHash_Mat hash_feature_mat_;
    BucketGroup_To_BucketHashing_Arr bucket_groups_hash_feature_mats_;
};

using ORBCascadeHasher = CascadeHasher<128, 32>;
