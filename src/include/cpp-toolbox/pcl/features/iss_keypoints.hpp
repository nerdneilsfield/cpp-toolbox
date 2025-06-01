#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/features/base_feature_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>

namespace toolbox::pcl
{

/**
 * @brief ISS (Intrinsic Shape Signatures) 关键点提取器 / ISS (Intrinsic Shape Signatures) keypoint extractor
 * 
 * @tparam DataType 数据类型（float或double） / Data type (float or double)
 * @tparam KNN 最近邻搜索算法类型，默认使用 kdtree_generic_t / K-nearest neighbor search algorithm type, defaults to kdtree_generic_t
 * 
 * @details ISS算法通过计算点云局部区域的特征值比率来检测显著的几何特征点。该算法对噪声鲁棒，能够提取稳定的关键点 / The ISS algorithm detects salient geometric feature points by computing eigenvalue ratios in local regions. It is robust to noise and can extract stable keypoints
 * 
 * @code
 * // 基本使用示例 / Basic usage example
 * using data_type = float;
 * point_cloud_t<data_type> cloud = load_point_cloud();
 * 
 * // 创建ISS关键点提取器 / Create ISS keypoint extractor
 * iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
 * 
 * // 设置参数 / Set parameters
 * extractor.set_input(cloud);
 * extractor.set_salient_radius(1.0f);      // 显著性计算半径 / Saliency computation radius
 * extractor.set_non_maxima_radius(0.5f);   // 非极大值抑制半径 / Non-maxima suppression radius
 * extractor.set_threshold21(0.975f);       // λ2/λ1 阈值 / λ2/λ1 threshold
 * extractor.set_threshold32(0.975f);       // λ3/λ2 阈值 / λ3/λ2 threshold
 * extractor.set_min_neighbors(5);
 * 
 * // 设置KNN算法 / Set KNN algorithm
 * kdtree_t<data_type> kdtree;
 * extractor.set_knn(kdtree);
 * 
 * // 提取关键点 / Extract keypoints
 * auto keypoints = extractor.extract();
 * std::cout << "找到 " << keypoints.size() << " 个ISS关键点 / Found " << keypoints.size() << " ISS keypoints" << std::endl;
 * @endcode
 * 
 * @code
 * // 参数调优示例 / Parameter tuning example
 * // 更严格的阈值以获得更少但更显著的关键点 / Stricter thresholds for fewer but more salient keypoints
 * extractor.set_threshold21(0.99f);   // 要求更高的特征值比率 / Require higher eigenvalue ratio
 * extractor.set_threshold32(0.99f);
 * 
 * // 更大的半径以捕获更大尺度的特征 / Larger radius to capture larger scale features
 * extractor.set_salient_radius(2.0f);
 * extractor.set_non_maxima_radius(1.0f);
 * @endcode
 */
template<typename DataType, typename KNN = kdtree_generic_t<point_t<DataType>, toolbox::metrics::L2Metric<DataType>>>
class CPP_TOOLBOX_EXPORT iss_keypoint_extractor_t
    : public base_keypoint_extractor_t<iss_keypoint_extractor_t<DataType, KNN>,
                                       DataType,
                                       KNN>
{
public:
  using base_type = base_keypoint_extractor_t<iss_keypoint_extractor_t<DataType, KNN>,
                                              DataType,
                                              KNN>;
  using data_type = typename base_type::data_type;
  using knn_type = typename base_type::knn_type;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;
  using indices_vector = typename base_type::indices_vector;

  iss_keypoint_extractor_t() = default;

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
   * @brief 设置显著性半径 / Set saliency radius
   * @param radius 用于计算显著性的邻域半径 / Neighborhood radius for saliency computation
   * 
   * @details 较大的半径会考虑更大的局部区域，适合检测大尺度特征 / Larger radius considers larger local regions, suitable for detecting large-scale features
   */
  void set_salient_radius(data_type radius) { m_salient_radius = radius; }
  
  /**
   * @brief 设置非极大值抑制半径 / Set non-maxima suppression radius
   * @param radius 非极大值抑制的半径 / Radius for non-maxima suppression
   * 
   * @details 在此半径内只保留显著性最高的点 / Only the point with highest saliency within this radius is kept
   */
  void set_non_maxima_radius(data_type radius) { m_non_maxima_radius = radius; }
  
  /**
   * @brief 设置λ2/λ1阈值 / Set λ2/λ1 threshold
   * @param threshold 第二特征值与第一特征值的比率阈值 / Ratio threshold for second to first eigenvalue
   * 
   * @details 较高的阈值(接近1)要求更均匀的特征值分布，产生更少的关键点 / Higher threshold (close to 1) requires more uniform eigenvalue distribution, producing fewer keypoints
   */
  void set_threshold21(data_type threshold) { m_threshold21 = threshold; }
  
  /**
   * @brief 设置λ3/λ2阈值 / Set λ3/λ2 threshold
   * @param threshold 第三特征值与第二特征值的比率阈值 / Ratio threshold for third to second eigenvalue
   * 
   * @details 较高的阈值(接近1)要求更均匀的特征值分布，产生更少的关键点 / Higher threshold (close to 1) requires more uniform eigenvalue distribution, producing fewer keypoints
   */
  void set_threshold32(data_type threshold) { m_threshold32 = threshold; }
  
  /**
   * @brief 设置最小邻居数量 / Set minimum number of neighbors
   * @param min_neighbors 有效计算所需的最小邻居数 / Minimum neighbors required for valid computation
   */
  void set_min_neighbors(std::size_t min_neighbors) { m_min_neighbors = min_neighbors; }

  /**
   * @brief 获取显著性半径 / Get saliency radius
   * @return 当前的显著性半径 / Current saliency radius
   */
  [[nodiscard]] data_type get_salient_radius() const { return m_salient_radius; }
  
  /**
   * @brief 获取非极大值抑制半径 / Get non-maxima suppression radius
   * @return 当前的非极大值抑制半径 / Current non-maxima suppression radius
   */
  [[nodiscard]] data_type get_non_maxima_radius() const { return m_non_maxima_radius; }
  
  /**
   * @brief 获取λ2/λ1阈值 / Get λ2/λ1 threshold
   * @return 当前的λ2/λ1阈值 / Current λ2/λ1 threshold
   */
  [[nodiscard]] data_type get_threshold21() const { return m_threshold21; }
  
  /**
   * @brief 获取λ3/λ2阈值 / Get λ3/λ2 threshold
   * @return 当前的λ3/λ2阈值 / Current λ3/λ2 threshold
   */
  [[nodiscard]] data_type get_threshold32() const { return m_threshold32; }
  
  /**
   * @brief 获取最小邻居数量 / Get minimum number of neighbors
   * @return 当前的最小邻居数量 / Current minimum number of neighbors
   */
  [[nodiscard]] std::size_t get_min_neighbors() const { return m_min_neighbors; }

private:
  /**
   * @brief ISS信息结构体 / ISS information structure
   * 
   * @details 存储点的特征值和显著性信息 / Stores eigenvalues and saliency information for a point
   */
  struct ISSInfo
  {
    data_type eigenvalue1;  ///< 最大特征值 / Largest eigenvalue
    data_type eigenvalue2;  ///< 中间特征值 / Middle eigenvalue  
    data_type eigenvalue3;  ///< 最小特征值 / Smallest eigenvalue
    data_type saliency;     ///< ISS显著性度量 / ISS saliency measure
    bool is_valid;          ///< 计算是否有效 / Whether computation is valid
  };

  /**
   * @brief 计算单个点的ISS响应 / Compute ISS response for a single point
   * @param point_idx 点的索引 / Point index
   * @return ISS信息 / ISS information
   */
  ISSInfo compute_iss_response(std::size_t point_idx);
  
  /**
   * @brief 计算所有点的ISS响应 / Compute ISS responses for all points
   * @return 所有点的ISS信息 / ISS information for all points
   */
  std::vector<ISSInfo> compute_all_iss_responses();
  
  /**
   * @brief 应用非极大值抑制 / Apply non-maxima suppression
   * @param iss_responses 所有点的ISS响应 / ISS responses for all points
   * @return 经过抑制后的关键点索引 / Keypoint indices after suppression
   */
  indices_vector apply_non_maxima_suppression(const std::vector<ISSInfo>& iss_responses);
  
  /**
   * @brief 计算指定范围内点的ISS响应（用于并行处理） / Compute ISS responses for points in specified range (for parallel processing)
   * @param[out] iss_responses ISS响应数组 / ISS response array
   * @param start_idx 起始索引 / Start index
   * @param end_idx 结束索引 / End index
   */
  void compute_iss_range(std::vector<ISSInfo>& iss_responses, 
                        std::size_t start_idx, 
                        std::size_t end_idx);

  // 成员变量 / Member variables
  bool m_enable_parallel = false;                               ///< 是否启用并行处理 / Whether to enable parallel processing
  data_type m_salient_radius = static_cast<data_type>(1.0);    ///< 显著性半径 / Saliency radius
  data_type m_non_maxima_radius = static_cast<data_type>(0.5); ///< 非极大值抑制半径 / Non-maxima suppression radius
  data_type m_threshold21 = static_cast<data_type>(0.975);     ///< λ2/λ1阈值 / λ2/λ1 threshold
  data_type m_threshold32 = static_cast<data_type>(0.975);     ///< λ3/λ2阈值 / λ3/λ2 threshold
  std::size_t m_min_neighbors = 5;                             ///< 最小邻居数量 / Minimum number of neighbors
  
  point_cloud_ptr m_cloud;                                      ///< 输入点云 / Input point cloud
  knn_type* m_knn = nullptr;                                    ///< KNN算法指针 / KNN algorithm pointer

  /**
   * @brief 并行处理阈值 / Parallel processing threshold
   */
  static constexpr std::size_t k_parallel_threshold = 1000;
};  // class iss_keypoint_extractor_t

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/features/impl/iss_keypoints_impl.hpp>