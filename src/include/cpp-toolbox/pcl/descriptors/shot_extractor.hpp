#pragma once

#include <array>
#include <cmath>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/descriptors/base_descriptor_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/norm/pca_norm.hpp>
#include <cpp-toolbox/math/matrix.hpp>

namespace toolbox::pcl
{

/**
 * @brief SHOT (Signature of Histograms of Orientations) signature
 * 
 * @tparam DataType The data type (float or double)
 * 
 * @details SHOT descriptor with 352 bins (32 spatial divisions × 11 angular bins)
 */
template<typename DataType>
struct shot_signature_t : public base_signature_t<shot_signature_t<DataType>>
{
  static constexpr std::size_t HISTOGRAM_SIZE = 352;  // 32 spatial × 11 angular
  std::array<DataType, HISTOGRAM_SIZE> histogram{};

  bool operator==(const shot_signature_t& other) const
  {
    for (std::size_t i = 0; i < HISTOGRAM_SIZE; ++i)
    {
      if (std::abs(histogram[i] - other.histogram[i]) > DataType(1e-6))
        return false;
    }
    return true;
  }

  DataType distance(const shot_signature_t& other) const
  {
    DataType dist = 0;
    for (std::size_t i = 0; i < HISTOGRAM_SIZE; ++i)
    {
      DataType diff = histogram[i] - other.histogram[i];
      dist += diff * diff;
    }
    return std::sqrt(dist);
  }
};

/**
 * @brief SHOT (Signature of Histograms of Orientations) descriptor extractor
 * 
 * @tparam DataType The data type (float or double)
 * @tparam KNN The K-nearest neighbor search algorithm type
 * 
 * @details SHOT is a 3D descriptor that encodes point distributions in a local reference frame
 * using a combination of spatial and angular histograms. It is rotation invariant and highly
 * discriminative.
 * 
 * Reference: Tombari, F., Salti, S., & Di Stefano, L. (2010). Unique signatures of histograms 
 * for local surface description.
 * 
 * @code
 * // Basic usage example
 * using data_type = float;
 * point_cloud_t<data_type> cloud = load_point_cloud();
 * 
 * // Create SHOT descriptor extractor
 * shot_extractor_t<data_type, kdtree_t<data_type>> extractor;
 * 
 * // Set parameters
 * extractor.set_input(cloud);
 * extractor.set_search_radius(0.1f);   // Search radius for neighbors
 * extractor.set_num_neighbors(100);    // Maximum number of neighbors
 * 
 * // Set up KNN search
 * kdtree_t<data_type> kdtree;
 * extractor.set_knn(kdtree);
 * 
 * // Extract descriptors for keypoints
 * std::vector<std::size_t> keypoint_indices = {10, 20, 30};
 * std::vector<shot_signature_t<data_type>> descriptors;
 * extractor.compute(cloud, keypoint_indices, descriptors);
 * @endcode
 */
template<typename DataType, typename KNN>
class CPP_TOOLBOX_EXPORT shot_extractor_t
    : public base_descriptor_extractor_t<shot_extractor_t<DataType, KNN>,
                                         DataType,
                                         shot_signature_t<DataType>>
{
public:
  using base_type = base_descriptor_extractor_t<shot_extractor_t<DataType, KNN>,
                                                DataType,
                                                shot_signature_t<DataType>>;
  using data_type = DataType;
  using signature_type = shot_signature_t<DataType>;
  using knn_type = KNN;
  using point_cloud = toolbox::types::point_cloud_t<DataType>;
  using point_cloud_ptr = std::shared_ptr<point_cloud>;

  shot_extractor_t() = default;

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
   * @brief Set the point cloud normals (optional, will be computed if not provided)
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

  void compute_impl(const point_cloud& cloud,
                    const std::vector<std::size_t>& keypoint_indices,
                    std::unique_ptr<std::vector<signature_type>>& descriptors) const;

private:
  struct local_rf_t
  {
    point_t<data_type> x_axis;
    point_t<data_type> y_axis;
    point_t<data_type> z_axis;
  };

  void compute_local_reference_frame(const point_cloud& cloud,
                                     const point_cloud& normals,
                                     std::size_t index,
                                     const std::vector<std::size_t>& neighbor_indices,
                                     local_rf_t& lrf) const;

  void compute_shot_feature(const point_cloud& cloud,
                            const point_cloud& normals,
                            std::size_t index,
                            const std::vector<std::size_t>& neighbor_indices,
                            const local_rf_t& lrf,
                            signature_type& shot) const;

  void compute_weighted_covariance(const point_cloud& cloud,
                                   std::size_t center_idx,
                                   const std::vector<std::size_t>& indices,
                                   const std::vector<data_type>& weights,
                                   data_type cov[9]) const;

  std::size_t compute_spatial_bin(const point_t<data_type>& point,
                                  const point_t<data_type>& center,
                                  const local_rf_t& lrf,
                                  data_type radius) const;

  std::size_t compute_angular_bin(const point_t<data_type>& normal,
                                  const local_rf_t& lrf) const;

  bool m_enable_parallel = false;
  data_type m_search_radius = 0.1;
  std::size_t m_num_neighbors = 100;
  point_cloud_ptr m_cloud;
  point_cloud_ptr m_normals;
  knn_type* m_knn = nullptr;
};

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/descriptors/impl/shot_extractor_impl.hpp>