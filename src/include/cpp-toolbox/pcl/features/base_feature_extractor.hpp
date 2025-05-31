#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/knn/base_knn.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::pcl
{

/**
 * @brief 关键点提取器的基类，使用CRTP模式实现静态多态 / Base class for keypoint extractors using CRTP pattern for static polymorphism
 * 
 * @tparam Derived 派生类类型 / Derived class type
 * @tparam DataType 数据类型（通常为float或double） / Data type (usually float or double)
 * @tparam KNN 最近邻搜索算法类型 / K-nearest neighbor search algorithm type
 * 
 * @details 该类为所有关键点提取器提供统一接口，派生类需要实现具体的提取算法 / This class provides a unified interface for all keypoint extractors, derived classes need to implement specific extraction algorithms
 * 
 * @code
 * // 使用示例 / Usage example
 * using data_type = float;
 * using knn_type = kdtree_t<data_type>;
 * 
 * // 创建具体的关键点提取器 / Create specific keypoint extractor
 * curvature_keypoint_extractor_t<data_type, knn_type> extractor;
 * 
 * // 设置输入点云 / Set input point cloud
 * point_cloud_t<data_type> cloud = load_point_cloud();
 * extractor.set_input(cloud);
 * 
 * // 设置KNN算法 / Set KNN algorithm
 * knn_type knn;
 * extractor.set_knn(knn);
 * 
 * // 设置搜索半径 / Set search radius
 * extractor.set_search_radius(0.5f);
 * 
 * // 提取关键点 / Extract keypoints
 * auto keypoint_indices = extractor.extract();
 * @endcode
 */
template<typename Derived, typename DataType, typename KNN>
class CPP_TOOLBOX_EXPORT base_keypoint_extractor_t
{
public:
  using data_type = DataType;
  using knn_type = KNN;
  using point_cloud = toolbox::types::point_cloud_t<data_type>;
  using point_cloud_ptr = std::shared_ptr<toolbox::types::point_cloud_t<data_type>>;
  using indices_vector = std::vector<std::size_t>;

  base_keypoint_extractor_t() = default;
  ~base_keypoint_extractor_t() = default;

  /**
   * @brief 设置输入点云（值传递版本） / Set input point cloud (value passing version)
   * @param cloud 输入点云 / Input point cloud
   * @return 成功设置的点数 / Number of successfully set points
   */
  std::size_t set_input(const point_cloud& cloud)
  {
    return static_cast<Derived*>(this)->set_input_impl(cloud);
  }

  /**
   * @brief 设置输入点云（智能指针版本） / Set input point cloud (smart pointer version)
   * @param cloud 输入点云的智能指针 / Smart pointer to input point cloud
   * @return 成功设置的点数 / Number of successfully set points
   */
  std::size_t set_input(const point_cloud_ptr& cloud)
  {
    return static_cast<Derived*>(this)->set_input_impl(cloud);
  }

  /**
   * @brief 设置搜索半径 / Set search radius
   * @param radius 搜索半径，用于邻域搜索 / Search radius for neighborhood search
   * @return 受影响的点数 / Number of affected points
   * 
   * @details 搜索半径决定了邻域的大小，影响关键点检测的尺度 / Search radius determines the neighborhood size and affects the scale of keypoint detection
   */
  std::size_t set_search_radius(data_type radius)
  {
    m_search_radius = radius;
    return static_cast<Derived*>(this)->set_search_radius_impl(radius);
  }

  /**
   * @brief 获取当前搜索半径 / Get current search radius
   * @return 搜索半径 / Search radius
   */
  [[nodiscard]] data_type get_search_radius() const noexcept
  {
    return m_search_radius;
  }

  /**
   * @brief 设置最近邻搜索算法 / Set K-nearest neighbor search algorithm
   * @param knn KNN算法实例 / KNN algorithm instance
   * @return 成功建立索引的点数 / Number of successfully indexed points
   * 
   * @code
   * // KNN算法选择示例 / KNN algorithm selection example
   * // 使用KD树（适合低维数据） / Use KD-tree (suitable for low-dimensional data)
   * kdtree_t<float> kdtree;
   * extractor.set_knn(kdtree);
   * 
   * // 使用暴力搜索（适合小数据集） / Use brute force (suitable for small datasets)
   * bfknn_t<float> bfknn;
   * extractor.set_knn(bfknn);
   * 
   * // 使用并行暴力搜索（适合多核CPU） / Use parallel brute force (suitable for multi-core CPU)
   * bfknn_parallel_t<float> bfknn_parallel;
   * extractor.set_knn(bfknn_parallel);
   * @endcode
   */
  std::size_t set_knn(const knn_type& knn)
  {
    return static_cast<Derived*>(this)->set_knn_impl(knn);
  }

  /**
   * @brief 启用或禁用并行处理 / Enable or disable parallel processing
   * @param enable true启用并行，false禁用并行 / true to enable parallel, false to disable
   * 
   * @details 并行处理可以加速关键点提取，但可能会增加内存使用 / Parallel processing can speed up keypoint extraction but may increase memory usage
   */
  void enable_parallel(bool enable)
  {
    static_cast<Derived*>(this)->enable_parallel_impl(enable);
  }

  /**
   * @brief 提取关键点索引 / Extract keypoint indices
   * @return 关键点在原始点云中的索引 / Indices of keypoints in the original point cloud
   * 
   * @code
   * // 提取关键点索引示例 / Extract keypoint indices example
   * auto indices = extractor.extract();
   * std::cout << "找到 " << indices.size() << " 个关键点 / Found " << indices.size() << " keypoints" << std::endl;
   * 
   * // 访问关键点 / Access keypoints
   * for (auto idx : indices) {
   *     const auto& keypoint = cloud.points[idx];
   *     // 处理关键点 / Process keypoint
   * }
   * @endcode
   */
  indices_vector extract()
  {
    return static_cast<Derived*>(this)->extract_impl();
  }

  /**
   * @brief 提取关键点索引（输出参数版本） / Extract keypoint indices (output parameter version)
   * @param[out] keypoint_indices 存储关键点索引的向量 / Vector to store keypoint indices
   */
  void extract(indices_vector& keypoint_indices)
  {
    static_cast<Derived*>(this)->extract_impl(keypoint_indices);
  }

  /**
   * @brief 提取关键点云 / Extract keypoint cloud
   * @return 包含所有关键点的新点云 / New point cloud containing all keypoints
   * 
   * @code
   * // 直接获取关键点云示例 / Direct keypoint cloud extraction example
   * auto keypoint_cloud = extractor.extract_keypoints();
   * std::cout << "关键点云包含 " << keypoint_cloud.size() << " 个点 / Keypoint cloud contains " << keypoint_cloud.size() << " points" << std::endl;
   * 
   * // 保存关键点云 / Save keypoint cloud
   * save_point_cloud(keypoint_cloud, "keypoints.pcd");
   * @endcode
   */
  point_cloud extract_keypoints()
  {
    return static_cast<Derived*>(this)->extract_keypoints_impl();
  }

  /**
   * @brief 提取关键点云（输出参数版本） / Extract keypoint cloud (output parameter version)
   * @param[out] output 存储关键点的点云指针 / Point cloud pointer to store keypoints
   */
  void extract_keypoints(point_cloud_ptr output)
  {
    static_cast<Derived*>(this)->extract_keypoints_impl(output);
  }

  /**
   * @brief 禁用拷贝构造函数 / Disable copy constructor
   */
  base_keypoint_extractor_t(const base_keypoint_extractor_t&) = delete;
  
  /**
   * @brief 禁用拷贝赋值运算符 / Disable copy assignment operator
   */
  base_keypoint_extractor_t& operator=(const base_keypoint_extractor_t&) = delete;
  
  /**
   * @brief 禁用移动构造函数 / Disable move constructor
   */
  base_keypoint_extractor_t(base_keypoint_extractor_t&&) = delete;
  
  /**
   * @brief 禁用移动赋值运算符 / Disable move assignment operator
   */
  base_keypoint_extractor_t& operator=(base_keypoint_extractor_t&&) = delete;

private:
  /**
   * @brief 搜索半径，默认值为1.0 / Search radius, default value is 1.0
   */
  data_type m_search_radius = static_cast<data_type>(1.0);
};  // class base_keypoint_extractor_t

}  // namespace toolbox::pcl