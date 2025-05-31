#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/features/base_feature_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>

namespace toolbox::pcl
{

/**
 * @brief 基于曲率的关键点提取器 / Curvature-based keypoint extractor
 * 
 * @tparam DataType 数据类型（float或double） / Data type (float or double)
 * @tparam KNN 最近邻搜索算法类型 / K-nearest neighbor search algorithm type
 * 
 * @details 该类通过计算每个点的主曲率来检测关键点。曲率较大的点通常对应于角点、边缘等几何特征 / This class detects keypoints by computing principal curvatures at each point. Points with high curvature typically correspond to corners, edges, and other geometric features
 * 
 * @code
 * // 基本使用示例 / Basic usage example
 * using data_type = float;
 * point_cloud_t<data_type> cloud = load_point_cloud();
 * 
 * // 创建曲率关键点提取器 / Create curvature keypoint extractor
 * curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
 * 
 * // 设置参数 / Set parameters
 * extractor.set_input(cloud);
 * extractor.set_search_radius(0.5f);
 * extractor.set_curvature_threshold(0.01f);
 * extractor.set_non_maxima_radius(0.3f);
 * extractor.set_min_neighbors(10);
 * 
 * // 设置KNN算法 / Set KNN algorithm
 * kdtree_t<data_type> kdtree;
 * extractor.set_knn(kdtree);
 * 
 * // 提取关键点 / Extract keypoints
 * auto keypoints = extractor.extract();
 * std::cout << "找到 " << keypoints.size() << " 个曲率关键点 / Found " << keypoints.size() << " curvature keypoints" << std::endl;
 * @endcode
 * 
 * @code
 * // 并行处理示例 / Parallel processing example
 * curvature_keypoint_extractor_t<float, bfknn_parallel_t<float>> parallel_extractor;
 * parallel_extractor.set_input(large_cloud);
 * parallel_extractor.enable_parallel(true);  // 启用多线程 / Enable multithreading
 * 
 * bfknn_parallel_t<float> parallel_knn;
 * parallel_extractor.set_knn(parallel_knn);
 * 
 * auto keypoints = parallel_extractor.extract();
 * @endcode
 */
template<typename DataType, typename KNN>
class CPP_TOOLBOX_EXPORT curvature_keypoint_extractor_t
    : public base_keypoint_extractor_t<curvature_keypoint_extractor_t<DataType, KNN>,
                                       DataType,
                                       KNN>
{
public:
  using base_type = base_keypoint_extractor_t<curvature_keypoint_extractor_t<DataType, KNN>,
                                              DataType,
                                              KNN>;
  using data_type = typename base_type::data_type;
  using knn_type = typename base_type::knn_type;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;
  using indices_vector = typename base_type::indices_vector;

  curvature_keypoint_extractor_t() = default;

  /**
   * @brief CRTP实现方法 - 设置输入点云 / CRTP implementation - set input point cloud
   */
  std::size_t set_input_impl(const point_cloud& cloud);
  std::size_t set_input_impl(const point_cloud_ptr& cloud);
  
  /**
   * @brief CRTP实现方法 - 设置KNN算法 / CRTP implementation - set KNN algorithm
   */
  std::size_t set_knn_impl(const knn_type& knn);
  
  /**
   * @brief CRTP实现方法 - 设置搜索半径 / CRTP implementation - set search radius
   */
  std::size_t set_search_radius_impl(data_type radius);
  
  /**
   * @brief CRTP实现方法 - 启用并行处理 / CRTP implementation - enable parallel processing
   */
  void enable_parallel_impl(bool enable);

  /**
   * @brief CRTP实现方法 - 提取关键点 / CRTP implementation - extract keypoints
   */
  indices_vector extract_impl();
  void extract_impl(indices_vector& keypoint_indices);
  point_cloud extract_keypoints_impl();
  void extract_keypoints_impl(point_cloud_ptr output);

  /**
   * @brief 设置曲率阈值 / Set curvature threshold
   * @param threshold 曲率阈值，超过此值的点被认为是关键点候选 / Curvature threshold, points exceeding this value are considered keypoint candidates
   * 
   * @details 较小的阈值会检测更多关键点，较大的阈值只检测曲率显著的点 / Smaller thresholds detect more keypoints, larger thresholds only detect points with significant curvature
   */
  void set_curvature_threshold(data_type threshold) { m_curvature_threshold = threshold; }
  
  /**
   * @brief 设置最小邻居数量 / Set minimum number of neighbors
   * @param min_neighbors 计算曲率所需的最小邻居点数 / Minimum number of neighbor points required for curvature computation
   * 
   * @details 较少的邻居可能导致曲率估计不稳定，建议至少使用10个邻居 / Fewer neighbors may lead to unstable curvature estimation, at least 10 neighbors are recommended
   */
  void set_min_neighbors(std::size_t min_neighbors) { m_min_neighbors = min_neighbors; }
  
  /**
   * @brief 设置非极大值抑制半径 / Set non-maxima suppression radius
   * @param radius 非极大值抑制的半径 / Radius for non-maxima suppression
   * 
   * @details 在此半径内只保留曲率最大的点作为关键点 / Only the point with maximum curvature within this radius is kept as keypoint
   */
  void set_non_maxima_radius(data_type radius) { m_non_maxima_radius = radius; }

  /**
   * @brief 获取曲率阈值 / Get curvature threshold
   * @return 当前的曲率阈值 / Current curvature threshold
   */
  [[nodiscard]] data_type get_curvature_threshold() const { return m_curvature_threshold; }
  
  /**
   * @brief 获取最小邻居数量 / Get minimum number of neighbors
   * @return 当前的最小邻居数量 / Current minimum number of neighbors
   */
  [[nodiscard]] std::size_t get_min_neighbors() const { return m_min_neighbors; }
  
  /**
   * @brief 获取非极大值抑制半径 / Get non-maxima suppression radius
   * @return 当前的非极大值抑制半径 / Current non-maxima suppression radius
   */
  [[nodiscard]] data_type get_non_maxima_radius() const { return m_non_maxima_radius; }

private:
  /**
   * @brief 曲率信息结构体 / Curvature information structure
   * 
   * @details 存储点的各种曲率测度 / Stores various curvature measures for a point
   */
  struct CurvatureInfo
  {
    data_type principal_curvature_1;  ///< 第一主曲率（最大曲率） / First principal curvature (maximum curvature)
    data_type principal_curvature_2;  ///< 第二主曲率（最小曲率） / Second principal curvature (minimum curvature)
    data_type mean_curvature;         ///< 平均曲率 (k1 + k2) / 2 / Mean curvature (k1 + k2) / 2
    data_type gaussian_curvature;     ///< 高斯曲率 k1 * k2 / Gaussian curvature k1 * k2
    data_type curvature_magnitude;    ///< 曲率大小 sqrt(k1^2 + k2^2) / Curvature magnitude sqrt(k1^2 + k2^2)
  };

  /**
   * @brief 计算单个点的曲率 / Compute curvature for a single point
   * @param point_idx 点的索引 / Point index
   * @return 曲率信息 / Curvature information
   */
  CurvatureInfo compute_curvature(std::size_t point_idx);
  
  /**
   * @brief 计算所有点的曲率 / Compute curvatures for all points
   * @return 所有点的曲率信息 / Curvature information for all points
   */
  std::vector<CurvatureInfo> compute_all_curvatures();
  
  /**
   * @brief 应用非极大值抑制 / Apply non-maxima suppression
   * @param curvatures 所有点的曲率信息 / Curvature information for all points
   * @return 经过抑制后的关键点索引 / Keypoint indices after suppression
   */
  indices_vector apply_non_maxima_suppression(const std::vector<CurvatureInfo>& curvatures);
  
  /**
   * @brief 计算指定范围内点的曲率（用于并行处理） / Compute curvatures for points in specified range (for parallel processing)
   * @param[out] curvatures 曲率信息数组 / Curvature information array
   * @param start_idx 起始索引 / Start index
   * @param end_idx 结束索引 / End index
   */
  void compute_curvatures_range(std::vector<CurvatureInfo>& curvatures, 
                               std::size_t start_idx, 
                               std::size_t end_idx);

  // 成员变量 / Member variables
  bool m_enable_parallel = false;                              ///< 是否启用并行处理 / Whether to enable parallel processing
  data_type m_search_radius = static_cast<data_type>(1.0);    ///< 搜索半径 / Search radius
  data_type m_curvature_threshold = static_cast<data_type>(0.01); ///< 曲率阈值 / Curvature threshold
  data_type m_non_maxima_radius = static_cast<data_type>(0.5);    ///< 非极大值抑制半径 / Non-maxima suppression radius
  std::size_t m_min_neighbors = 10;                           ///< 最小邻居数量 / Minimum number of neighbors
  
  point_cloud_ptr m_cloud;                                     ///< 输入点云 / Input point cloud
  knn_type* m_knn = nullptr;                                   ///< KNN算法指针 / KNN algorithm pointer

  /**
   * @brief 并行处理阈值，点数超过此值时启用并行 / Parallel processing threshold, enable parallel when point count exceeds this value
   */
  static constexpr std::size_t k_parallel_threshold = 1000;
};  // class curvature_keypoint_extractor_t

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/features/impl/curvature_keypoints_impl.hpp>