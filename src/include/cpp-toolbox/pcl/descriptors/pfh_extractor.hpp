#pragma once

#include <array>
#include <cmath>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/descriptors/base_descriptor_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/norm/pca_norm.hpp>

namespace toolbox::pcl
{

/**
 * @brief PFH (Point Feature Histogram) signature
 *
 * @tparam DataType The data type (float or double)
 *
 * @details PFH descriptor with 125 bins (5^3 for the 3 angular features)
 */
template<typename DataType>
struct pfh_signature_t : public base_signature_t<DataType, pfh_signature_t<DataType>>
{
  static constexpr std::size_t HISTOGRAM_SIZE = 125;  // 5 � 5 � 5
  std::array<DataType, HISTOGRAM_SIZE> histogram {};

  DataType distance_impl(const pfh_signature_t& other) const
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
 * @brief PFH (Point Feature Histogram) descriptor extractor
 *
 * @tparam DataType The data type (float or double)
 * @tparam KNN The K-nearest neighbor search algorithm type
 *
 * @details PFH encodes the local geometry around a point by computing angular
 * features between all pairs of points in the neighborhood. It captures
 * detailed geometric information but is computationally expensive with O(k^2)
 * complexity.
 *
 * Reference: Rusu, R. B., Blodow, N., Marton, Z. C., & Beetz, M. (2008).
 * Aligning point cloud views using persistent feature histograms.
 *
 * @code
 * // Basic usage example
 * using data_type = float;
 * point_cloud_t<data_type> cloud = load_point_cloud();
 *
 * // Create PFH descriptor extractor
 * pfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
 *
 * // Set parameters
 * extractor.set_input(cloud);
 * extractor.set_search_radius(0.05f);  // Search radius for neighbors
 * extractor.set_num_neighbors(30);     // Maximum number of neighbors
 *
 * // Set up KNN search
 * kdtree_t<data_type> kdtree;
 * extractor.set_knn(kdtree);
 *
 * // Extract descriptors for keypoints
 * std::vector<std::size_t> keypoint_indices = {10, 20, 30};
 * std::vector<pfh_signature_t<data_type>> descriptors;
 * extractor.compute(cloud, keypoint_indices, descriptors);
 * @endcode
 */
template<typename DataType, typename KNN>
class CPP_TOOLBOX_EXPORT pfh_extractor_t
    : public base_descriptor_extractor_t<pfh_extractor_t<DataType, KNN>,
                                         DataType,
                                         pfh_signature_t<DataType>>
{
public:
  using base_type = base_descriptor_extractor_t<pfh_extractor_t<DataType, KNN>,
                                                DataType,
                                                pfh_signature_t<DataType>>;
  using data_type = DataType;
  using signature_type = pfh_signature_t<DataType>;
  using knn_type = KNN;
  using point_cloud = toolbox::types::point_cloud_t<DataType>;
  using point_cloud_ptr = std::shared_ptr<point_cloud>;

  pfh_extractor_t() = default;

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
   * @brief Set the number of subdivision for each angular feature (default: 5)
   */
  void set_num_subdivisions(std::size_t subdivisions);

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
  void compute_pfh_feature(const point_cloud& cloud,
                           const point_cloud& normals,
                           std::size_t index,
                           const std::vector<std::size_t>& neighbor_indices,
                           signature_type& pfh) const;

  void compute_pair_features(const point_t<data_type>& p1,
                             const point_t<data_type>& n1,
                             const point_t<data_type>& p2,
                             const point_t<data_type>& n2,
                             data_type& f1,
                             data_type& f2,
                             data_type& f3,
                             data_type& f4) const;

  std::size_t compute_feature_bin_index(data_type f1,
                                        data_type f2,
                                        data_type f3,
                                        data_type f4) const;

  bool m_enable_parallel = false;
  data_type m_search_radius = 0.05;
  std::size_t m_num_neighbors = 30;
  std::size_t m_num_subdivisions = 5;
  point_cloud_ptr m_cloud;
  point_cloud_ptr m_normals;
  knn_type* m_knn = nullptr;
};

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/descriptors/impl/pfh_extractor_impl.hpp>