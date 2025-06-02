#pragma once

#include <array>
#include <cmath>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/descriptors/base_descriptor_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/norm/pca_norm.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>

namespace toolbox::pcl
{

/**
 * @brief FPFH (Fast Point Feature Histogram) signature
 *
 * @tparam DataType The data type (float or double)
 *
 * @details FPFH descriptor with 33 bins (11 bins for each of the 3 angular
 * features)
 */
template<typename DataType>
struct fpfh_signature_t : public base_signature_t<DataType, fpfh_signature_t<DataType>>
{
  using value_type = DataType;  // 为了兼容KNN接口 / For KNN interface compatibility
  static constexpr std::size_t HISTOGRAM_SIZE = 33;
  std::array<DataType, HISTOGRAM_SIZE> histogram {};

  // 提供data()和size()方法以兼容IMetric接口 / Provide data() and size() for IMetric compatibility
  const DataType* data() const { return histogram.data(); }
  DataType* data() { return histogram.data(); }
  constexpr std::size_t size() const { return HISTOGRAM_SIZE; }

  DataType distance_impl(const fpfh_signature_t& other) const
  {
    DataType dist = 0;
    for (std::size_t i = 0; i < HISTOGRAM_SIZE; ++i) {
      DataType diff = histogram[i] - other.histogram[i];
      dist += diff * diff;
    }
    return std::sqrt(dist);
  }
};

/**
 * @brief FPFH (Fast Point Feature Histogram) descriptor extractor
 *
 * @tparam DataType The data type (float or double)
 * @tparam KNN The K-nearest neighbor search algorithm type
 *
 * @details FPFH is a fast variant of PFH that achieves computational efficiency
 * by caching previously computed values and using a simplified neighborhood
 * weighting scheme.
 *
 * Reference: Rusu, R. B., Blodow, N., & Beetz, M. (2009). Fast point feature
 * histograms (FPFH) for 3D registration.
 *
 * @code
 * // Basic usage example
 * using data_type = float;
 * point_cloud_t<data_type> cloud = load_point_cloud();
 *
 * // Create FPFH descriptor extractor
 * fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
 *
 * // Set parameters
 * extractor.set_input(cloud);
 * extractor.set_search_radius(0.05f);  // Search radius for neighbors
 * extractor.set_num_neighbors(50);     // Maximum number of neighbors
 *
 * // Set up KNN search
 * kdtree_t<data_type> kdtree;
 * extractor.set_knn(kdtree);
 *
 * // Extract descriptors for keypoints
 * std::vector<std::size_t> keypoint_indices = {10, 20, 30};
 * std::vector<fpfh_signature_t<data_type>> descriptors;
 * extractor.compute(cloud, keypoint_indices, descriptors);
 * @endcode
 */
template<typename DataType, 
         typename KNN = kdtree_generic_t<point_t<DataType>, toolbox::metrics::L2Metric<DataType>>>
class CPP_TOOLBOX_EXPORT fpfh_extractor_t
    : public base_descriptor_extractor_t<fpfh_extractor_t<DataType, KNN>,
                                         DataType,
                                         fpfh_signature_t<DataType>>
{
public:
  using base_type = base_descriptor_extractor_t<fpfh_extractor_t<DataType, KNN>,
                                                DataType,
                                                fpfh_signature_t<DataType>>;
  using data_type = DataType;
  using signature_type = fpfh_signature_t<DataType>;
  using knn_type = KNN;
  using point_cloud = toolbox::types::point_cloud_t<DataType>;
  using point_cloud_ptr = std::shared_ptr<point_cloud>;

  fpfh_extractor_t() = default;

  /**
   * @brief Set the input point cloud
   */
  std::size_t set_input(const point_cloud& cloud);
  std::size_t set_input(const point_cloud_ptr& cloud);

  /**
   * @brief Set the KNN search algorithm
   */
  std::size_t set_knn(const knn_type& knn);

  /**
   * @brief Set the search radius for neighbor search
   */
  std::size_t set_search_radius(data_type radius);

  /**
   * @brief Set the maximum number of neighbors to consider
   */
  std::size_t set_num_neighbors(std::size_t num_neighbors);

  /**
   * @brief Set the point cloud normals (optional, will be computed if not
   * provided)
   */
  void set_normals(const point_cloud_ptr& normals);

  /**
   * @brief Enable or disable parallel processing
   */
  void enable_parallel_impl(bool enable);

  /**
   * @brief Compute descriptors for given keypoints
   */
  void compute_impl(const point_cloud& cloud,
                    const std::vector<std::size_t>& keypoint_indices,
                    std::vector<signature_type>& descriptors) const;

  void compute_impl(
      const point_cloud& cloud,
      const std::vector<std::size_t>& keypoint_indices,
      std::unique_ptr<std::vector<signature_type>>& descriptors) const;

private:
  struct spfh_signature_t
  {
    std::array<DataType, 11> f1 {};  // alpha histogram
    std::array<DataType, 11> f2 {};  // phi histogram
    std::array<DataType, 11> f3 {};  // theta histogram
  };

  void compute_spfh(const point_cloud& cloud,
                    const point_cloud& normals,
                    std::size_t index,
                    const std::vector<std::size_t>& neighbor_indices,
                    spfh_signature_t& spfh) const;

  void compute_fpfh_feature(const point_cloud& cloud,
                            const point_cloud& normals,
                            std::size_t index,
                            const std::vector<spfh_signature_t>& spfh_features,
                            signature_type& fpfh) const;

  // 优化相关数据结构 / Optimization related data structures
  template<typename T>
  struct neighbor_info_t
  {
    std::vector<std::size_t> indices;      ///< 邻居索引 / Neighbor indices
    std::vector<T> distances;              ///< 邻居距离 / Neighbor distances
    bool computed = false;                 ///< 是否已计算 / Whether computed
    
    void reserve(std::size_t n) {
      indices.reserve(n);
      distances.reserve(n);
    }
    
    void clear() {
      indices.clear();
      distances.clear();
      computed = false;
    }
  };

  template<typename T, typename K>
  class spfh_cache_manager_t
  {
  public:
    using spfh_signature_type = spfh_signature_t;
    
    explicit spfh_cache_manager_t(std::size_t cloud_size) 
      : cloud_size_(cloud_size)
      , needs_spfh_(cloud_size, false)
      , point_to_spfh_idx_(cloud_size, -1)
    {
      spfh_features_.reserve(cloud_size / 4);
    }
    
    void mark_needed(std::size_t point_idx) {
      if (point_idx < cloud_size_ && !needs_spfh_[point_idx]) {
        needs_spfh_[point_idx] = true;
        point_to_spfh_idx_[point_idx] = static_cast<int>(spfh_features_.size());
        spfh_features_.emplace_back();
      }
    }
    
    bool is_needed(std::size_t point_idx) const {
      return point_idx < cloud_size_ && needs_spfh_[point_idx];
    }
    
    spfh_signature_type& get_spfh(std::size_t point_idx) {
      int spfh_idx = point_to_spfh_idx_[point_idx];
      return spfh_features_[spfh_idx];
    }
    
    const spfh_signature_type& get_spfh(std::size_t point_idx) const {
      int spfh_idx = point_to_spfh_idx_[point_idx];
      return spfh_features_[spfh_idx];
    }
    
    std::size_t size() const { return spfh_features_.size(); }
    
    std::vector<std::size_t> get_needed_points() const {
      std::vector<std::size_t> needed_points;
      needed_points.reserve(spfh_features_.size());
      
      for (std::size_t i = 0; i < cloud_size_; ++i) {
        if (needs_spfh_[i]) {
          needed_points.push_back(i);
        }
      }
      return needed_points;
    }

  private:
    std::size_t cloud_size_;
    std::vector<bool> needs_spfh_;
    std::vector<int> point_to_spfh_idx_;
    std::vector<spfh_signature_type> spfh_features_;
  };

  void compute_fpfh_feature_optimized(const point_cloud& cloud,
                                    const point_cloud& normals,
                                    std::size_t index,
                                    const neighbor_info_t<data_type>& neighbor_info,
                                    const spfh_cache_manager_t<data_type, knn_type>& spfh_cache,
                                    signature_type& fpfh) const;

  void compute_pair_features(const point_t<data_type>& p1,
                             const point_t<data_type>& n1,
                             const point_t<data_type>& p2,
                             const point_t<data_type>& n2,
                             data_type& f1,
                             data_type& f2,
                             data_type& f3) const;

  std::size_t compute_bin_index(data_type value,
                                data_type min_val,
                                data_type max_val,
                                std::size_t num_bins) const;

  bool m_enable_parallel = false;
  data_type m_search_radius = 0.05;
  std::size_t m_num_neighbors = 50;
  point_cloud_ptr m_cloud;
  point_cloud_ptr m_normals;
  knn_type* m_knn = nullptr;
};

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/descriptors/impl/fpfh_extractor_impl.hpp>