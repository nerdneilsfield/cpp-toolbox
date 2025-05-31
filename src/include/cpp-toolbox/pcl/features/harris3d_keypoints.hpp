#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/features/base_feature_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>

namespace toolbox::pcl
{

/**
 * @brief Harris 3D 关键点提取器 / Harris 3D keypoint extractor
 * 
 * @tparam DataType 数据类型（float或double） / Data type (float or double)
 * @tparam KNN 最近邻搜索算法类型 / K-nearest neighbor search algorithm type
 * 
 * @details Harris 3D算法是经典Harris角点检测算法在3D点云中的扩展。它通过计算点云局部邻域的协方差矩阵来检测几何特征显著的点 / The Harris 3D algorithm extends the classic Harris corner detection to 3D point clouds. It detects geometrically salient points by computing the covariance matrix of local neighborhoods
 * 
 * @code
 * // 基本使用示例 / Basic usage example
 * using data_type = float;
 * point_cloud_t<data_type> cloud = load_point_cloud();
 * 
 * // 创建Harris3D关键点提取器 / Create Harris3D keypoint extractor
 * harris3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
 * 
 * // 设置参数 / Set parameters
 * extractor.set_input(cloud);
 * extractor.set_search_radius(0.5f);       // 搜索半径 / Search radius
 * extractor.set_threshold(0.01f);          // Harris响应阈值 / Harris response threshold
 * extractor.set_harris_k(0.04f);           // Harris参数k / Harris parameter k
 * extractor.set_suppression_radius(0.1f);  // 非极大值抑制半径 / Non-maxima suppression radius
 * extractor.set_num_neighbors(20);         // K近邻数量 / Number of k-neighbors
 * 
 * // 设置KNN算法 / Set KNN algorithm
 * kdtree_t<data_type> kdtree;
 * extractor.set_knn(kdtree);
 * 
 * // 提取关键点 / Extract keypoints
 * auto keypoints = extractor.extract();
 * std::cout << "找到 " << keypoints.size() << " 个Harris3D关键点 / Found " << keypoints.size() << " Harris3D keypoints" << std::endl;
 * @endcode
 * 
 * @code
 * // 调整参数以获得不同的检测结果 / Adjust parameters for different detection results
 * // 更严格的阈值，获得更少但更可靠的关键点 / Stricter threshold for fewer but more reliable keypoints
 * extractor.set_threshold(0.05f);
 * 
 * // 使用固定K近邻而非半径搜索 / Use fixed k-neighbors instead of radius search
 * extractor.set_num_neighbors(50);  // 使用50个最近邻 / Use 50 nearest neighbors
 * 
 * // 调整Harris参数k以改变角点响应 / Adjust Harris parameter k to change corner response
 * extractor.set_harris_k(0.01f);    // 更敏感的检测 / More sensitive detection
 * @endcode
 */
template<typename DataType, typename KNN>
class CPP_TOOLBOX_EXPORT harris3d_keypoint_extractor_t
    : public base_keypoint_extractor_t<harris3d_keypoint_extractor_t<DataType, KNN>,
                                       DataType,
                                       KNN>
{
public:
  using base_type = base_keypoint_extractor_t<harris3d_keypoint_extractor_t<DataType, KNN>,
                                              DataType,
                                              KNN>;
  using data_type = typename base_type::data_type;
  using knn_type = typename base_type::knn_type;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;
  using indices_vector = typename base_type::indices_vector;

  harris3d_keypoint_extractor_t() = default;

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
   * @brief 设置Harris响应阈值 / Set Harris response threshold
   * @param threshold Harris响应的最小阈值 / Minimum threshold for Harris response
   * 
   * @details 只有Harris响应值大于此阈值的点才会被考虑为关键点候选 / Only points with Harris response greater than this threshold are considered as keypoint candidates
   */
  void set_threshold(data_type threshold) { m_threshold = threshold; }
  
  /**
   * @brief 设置Harris参数k / Set Harris parameter k
   * @param k Harris角点响应函数中的k参数 / Parameter k in Harris corner response function
   * 
   * @details 通常取值范围为[0.04, 0.06]。较小的k值会产生更多的角点 / Typical range is [0.04, 0.06]. Smaller k values produce more corners
   */
  void set_harris_k(data_type k) { m_harris_k = k; }
  
  /**
   * @brief 设置非极大值抑制半径 / Set non-maxima suppression radius
   * @param radius 非极大值抑制的半径 / Radius for non-maxima suppression
   * 
   * @details 在此半径内只保留Harris响应最大的点 / Only the point with maximum Harris response within this radius is kept
   */
  void set_suppression_radius(data_type radius) { m_suppression_radius = radius; }
  
  /**
   * @brief 设置近邻数量 / Set number of neighbors
   * @param num_neighbors 计算Harris响应时使用的近邻数量 / Number of neighbors used for Harris response computation
   * 
   * @details 当使用固定K近邻而非半径搜索时使用此参数 / This parameter is used when using fixed k-neighbors instead of radius search
   */
  void set_num_neighbors(std::size_t num_neighbors) { m_num_neighbors = num_neighbors; }

  /**
   * @brief 获取Harris响应阈值 / Get Harris response threshold
   * @return 当前的Harris响应阈值 / Current Harris response threshold
   */
  [[nodiscard]] data_type get_threshold() const { return m_threshold; }
  
  /**
   * @brief 获取Harris参数k / Get Harris parameter k
   * @return 当前的Harris参数k / Current Harris parameter k
   */
  [[nodiscard]] data_type get_harris_k() const { return m_harris_k; }
  
  /**
   * @brief 获取非极大值抑制半径 / Get non-maxima suppression radius
   * @return 当前的非极大值抑制半径 / Current non-maxima suppression radius
   */
  [[nodiscard]] data_type get_suppression_radius() const { return m_suppression_radius; }
  
  /**
   * @brief 获取近邻数量 / Get number of neighbors
   * @return 当前的近邻数量 / Current number of neighbors
   */
  [[nodiscard]] std::size_t get_num_neighbors() const { return m_num_neighbors; }

private:
  /**
   * @brief Harris3D信息结构体 / Harris3D information structure
   */
  struct Harris3DInfo
  {
    data_type harris_response;  ///< Harris响应值 / Harris response value
    bool is_valid;              ///< 计算是否有效 / Whether computation is valid
  };

  /**
   * @brief 计算单个点的Harris3D响应 / Compute Harris3D response for a single point
   * @param point_idx 点的索引 / Point index
   * @return Harris3D信息 / Harris3D information
   */
  Harris3DInfo compute_harris3d_response(std::size_t point_idx);
  
  /**
   * @brief 计算所有点的Harris响应 / Compute Harris responses for all points
   * @return 所有点的Harris信息 / Harris information for all points
   */
  std::vector<Harris3DInfo> compute_all_harris_responses();
  
  /**
   * @brief 应用非极大值抑制 / Apply non-maxima suppression
   * @param harris_responses 所有点的Harris响应 / Harris responses for all points
   * @return 经过抑制后的关键点索引 / Keypoint indices after suppression
   */
  indices_vector apply_non_maxima_suppression(const std::vector<Harris3DInfo>& harris_responses);
  
  /**
   * @brief 计算指定范围内点的Harris响应（用于并行处理） / Compute Harris responses for points in specified range (for parallel processing)
   * @param[out] harris_responses Harris响应数组 / Harris response array
   * @param start_idx 起始索引 / Start index
   * @param end_idx 结束索引 / End index
   */
  void compute_harris_range(std::vector<Harris3DInfo>& harris_responses, 
                           std::size_t start_idx, 
                           std::size_t end_idx);

  // 成员变量 / Member variables
  bool m_enable_parallel = false;                                ///< 是否启用并行处理 / Whether to enable parallel processing
  data_type m_threshold = static_cast<data_type>(0.01);         ///< Harris响应阈值 / Harris response threshold
  data_type m_harris_k = static_cast<data_type>(0.04);          ///< Harris参数k / Harris parameter k
  data_type m_suppression_radius = static_cast<data_type>(0.1); ///< 非极大值抑制半径 / Non-maxima suppression radius
  std::size_t m_num_neighbors = 20;                             ///< 近邻数量 / Number of neighbors
  
  point_cloud_ptr m_cloud;                                       ///< 输入点云 / Input point cloud
  knn_type* m_knn = nullptr;                                     ///< KNN算法指针 / KNN algorithm pointer

  /**
   * @brief 并行处理阈值 / Parallel processing threshold
   */
  static constexpr std::size_t k_parallel_threshold = 1000;
};  // class harris3d_keypoint_extractor_t

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/features/impl/harris3d_keypoints_impl.hpp>