#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/features/base_feature_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <Eigen/Dense>

namespace toolbox::pcl
{

template<typename DataType, typename KNN>
class CPP_TOOLBOX_EXPORT mls_keypoint_extractor_t
    : public base_keypoint_extractor_t<mls_keypoint_extractor_t<DataType, KNN>,
                                       DataType,
                                       KNN>
{
public:
  using base_type = base_keypoint_extractor_t<mls_keypoint_extractor_t<DataType, KNN>,
                                              DataType,
                                              KNN>;
  using data_type = typename base_type::data_type;
  using knn_type = typename base_type::knn_type;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;
  using indices_vector = typename base_type::indices_vector;

  // Polynomial order enum
  enum class polynomial_order_t : int {
    NONE = 0,      // No polynomial fitting, only plane
    LINEAR = 1,    // First order polynomial
    QUADRATIC = 2  // Second order polynomial
  };

  mls_keypoint_extractor_t() = default;

  // Implementation methods for CRTP
  std::size_t set_input_impl(const point_cloud& cloud);
  std::size_t set_input_impl(const point_cloud_ptr& cloud);
  std::size_t set_knn_impl(const knn_type& knn);
  std::size_t set_search_radius_impl(data_type radius);
  void enable_parallel_impl(bool enable);

  indices_vector extract_impl();
  void extract_impl(indices_vector& keypoint_indices);
  point_cloud extract_keypoints_impl();
  void extract_keypoints_impl(point_cloud_ptr output);

  // MLS-specific parameters
  void set_polynomial_order(polynomial_order_t order) { m_polynomial_order = order; }
  void set_sqr_gauss_param(data_type sqr_gauss_param) { m_sqr_gauss_param = sqr_gauss_param; }
  void set_compute_curvatures(bool compute) { m_compute_curvatures = compute; }
  void set_variation_threshold(data_type threshold) { m_variation_threshold = threshold; }
  void set_curvature_threshold(data_type threshold) { m_curvature_threshold = threshold; }
  void set_non_maxima_radius(data_type radius) { m_non_maxima_radius = radius; }
  void set_min_neighbors(std::size_t min_neighbors) { m_min_neighbors = min_neighbors; }

  [[nodiscard]] polynomial_order_t get_polynomial_order() const { return m_polynomial_order; }
  [[nodiscard]] data_type get_sqr_gauss_param() const { return m_sqr_gauss_param; }
  [[nodiscard]] bool get_compute_curvatures() const { return m_compute_curvatures; }
  [[nodiscard]] data_type get_variation_threshold() const { return m_variation_threshold; }
  [[nodiscard]] data_type get_curvature_threshold() const { return m_curvature_threshold; }
  [[nodiscard]] data_type get_non_maxima_radius() const { return m_non_maxima_radius; }
  [[nodiscard]] std::size_t get_min_neighbors() const { return m_min_neighbors; }

  // Structure to hold MLS results for a point
  struct MLSResult
  {
    bool valid;
    data_type variation;     // Surface variation metric
    data_type curvature;     // Mean curvature
    Eigen::Vector3f normal;  // Refined normal
    
    MLSResult() : valid(false), variation(0), curvature(0), normal(Eigen::Vector3f::Zero()) {}
  };

private:
  // Core computation methods
  MLSResult compute_mls_surface(std::size_t point_idx);
  std::vector<MLSResult> compute_all_mls_surfaces();
  indices_vector apply_non_maxima_suppression(const std::vector<MLSResult>& mls_results);
  
  void compute_mls_range(std::vector<MLSResult>& mls_results,
                        std::size_t start_idx, 
                        std::size_t end_idx);
  
  // Helper methods for polynomial fitting
  int get_polynomial_coefficients_size() const;
  void compute_polynomial_coefficients(
      const std::vector<Eigen::Vector3f>& points,
      const std::vector<data_type>& weights,
      const Eigen::Vector3f& mean_point,
      Eigen::VectorXf& coefficients);
  
  data_type compute_surface_variation(
      const std::vector<Eigen::Vector3f>& points,
      const Eigen::Vector3f& mean_point,
      const Eigen::VectorXf& coefficients);

  // Member variables
  bool m_enable_parallel = false;
  polynomial_order_t m_polynomial_order = polynomial_order_t::QUADRATIC;
  data_type m_search_radius = static_cast<data_type>(1.0);
  data_type m_sqr_gauss_param = static_cast<data_type>(0.0);  // 0 means radius^2
  bool m_compute_curvatures = true;
  data_type m_variation_threshold = static_cast<data_type>(0.001);  // Surface variation threshold
  data_type m_curvature_threshold = static_cast<data_type>(0.1);    // Curvature threshold
  data_type m_non_maxima_radius = static_cast<data_type>(0.5);
  std::size_t m_min_neighbors = 10;
  
  point_cloud_ptr m_cloud;
  knn_type* m_knn = nullptr;

  // Parallel processing threshold
  static constexpr std::size_t k_parallel_threshold = 1000;
};  // class mls_keypoint_extractor_t

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/features/impl/mls_keypoints_impl.hpp>