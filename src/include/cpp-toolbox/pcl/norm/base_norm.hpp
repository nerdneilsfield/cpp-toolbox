#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/knn/base_knn.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::pcl
{

/**
 * @brief 法向量提取器的基类（CRTP模式） / Base class for normal extractors (CRTP pattern)
 * 
 * 该类定义了所有法向量提取算法的通用接口，使用CRTP模式实现静态多态。
 * 法向量是点云处理中的重要特征，用于描述局部表面的方向。
 * This class defines the common interface for all normal extraction algorithms,
 * using CRTP pattern for static polymorphism. Normals are important features in
 * point cloud processing, describing the direction of local surfaces.
 * 
 * @tparam Derived 派生类类型 / Derived class type
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or double)
 * @tparam KNN KNN搜索算法类型 / KNN search algorithm type
 * 
 * @code
 * // 使用示例 / Usage example
 * pca_norm_extractor_t<float> norm_extractor;
 * point_cloud_t<float> cloud = load_point_cloud("data.pcd");
 * 
 * // 设置输入和参数 / Set input and parameters
 * norm_extractor.set_input(cloud);
 * norm_extractor.set_num_neighbors(30);
 * 
 * // 提取法向量 / Extract normals
 * point_cloud_t<float> normals = norm_extractor.extract();
 * 
 * // 每个法向量点的xyz分量表示法向量方向 / xyz components of each normal point represent the normal direction
 * for (const auto& normal : normals.points) {
 *     std::cout << "法向量 / Normal: (" << normal.x << ", " << normal.y << ", " << normal.z << ")\n";
 * }
 * @endcode
 */
template<typename Derived, typename DataType, typename KNN>
class CPP_TOOLBOX_EXPORT base_norm_extractor_t
{
public:
  using data_type = DataType;
  using knn_type = KNN;
  using point_type = toolbox::types::point_t<data_type>;
  using point_cloud = toolbox::types::point_cloud_t<data_type>;
  using point_cloud_ptr =
      std::shared_ptr<toolbox::types::point_cloud_t<data_type>>;

  base_norm_extractor_t() = default;
  ~base_norm_extractor_t() = default;

  /**
   * @brief 设置输入点云 / Set input point cloud
   * @param cloud 输入点云 / Input point cloud
   * @return 点云中的点数 / Number of points in the cloud
   */
  std::size_t set_input(const point_cloud& cloud)
  {
    return static_cast<Derived*>(this)->set_input_impl(cloud);
  }

  /**
   * @brief 设置输入点云（智能指针版本） / Set input point cloud (smart pointer version)
   * @param cloud 输入点云的智能指针 / Smart pointer to input point cloud
   * @return 点云中的点数 / Number of points in the cloud
   */
  std::size_t set_input(const point_cloud_ptr& cloud)
  {
    return static_cast<Derived*>(this)->set_input_impl(cloud);
  }

  /**
   * @brief 设置用于法向量估计的近邻数量 / Set number of neighbors for normal estimation
   * @param num_neighbors 近邻数量 / Number of neighbors
   * @return 实际设置的近邻数量 / Actually set number of neighbors
   * 
   * @note 近邻数量影响法向量估计的平滑程度，较大的值产生更平滑的法向量 /
   * The number of neighbors affects the smoothness of normal estimation,
   * larger values produce smoother normals
   */
  std::size_t set_num_neighbors(std::size_t num_neighbors)
  {
    m_num_neighbors = num_neighbors;
    return static_cast<Derived*>(this)->set_num_neighbors_impl(num_neighbors);
  }

  /**
   * @brief 获取当前的近邻数量设置 / Get current number of neighbors setting
   * @return 近邻数量 / Number of neighbors
   */
  [[nodiscard]] std::size_t get_num_neighbors() const noexcept
  {
    return m_num_neighbors;
  }

  /**
   * @brief 设置KNN搜索算法 / Set KNN search algorithm
   * @param knn KNN搜索算法对象 / KNN search algorithm object
   * @return 设置结果 / Setting result
   * 
   * @code
   * // 使用不同的KNN算法 / Using different KNN algorithms
   * kdtree_t<float> kdtree;
   * norm_extractor.set_knn(kdtree);
   * @endcode
   */
  std::size_t set_knn(const knn_type& knn)
  {
    return static_cast<Derived*>(this)->set_knn_impl(knn);
  }

  /**
   * @brief 提取法向量 / Extract normals
   * @return 包含法向量的点云，每个点的xyz表示法向量方向 / 
   * Point cloud containing normals, xyz of each point represents normal direction
   * 
   * @code
   * // 提取并验证法向量 / Extract and verify normals
   * auto normals = norm_extractor.extract();
   * 
   * // 检查法向量是否归一化 / Check if normals are normalized
   * for (const auto& n : normals.points) {
   *     float length = std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
   *     assert(std::abs(length - 1.0f) < 1e-6f);
   * }
   * @endcode
   */
  point_cloud extract() { return static_cast<Derived*>(this)->extract_impl(); }

  /**
   * @brief 提取法向量到指定输出 / Extract normals to specified output
   * @param output [out] 输出点云的智能指针 / Smart pointer to output point cloud
   */
  void extract(point_cloud_ptr output)
  {
    return static_cast<Derived*>(this)->extract_impl(output);
  }

  base_norm_extractor_t(const base_norm_extractor_t&) = delete;
  base_norm_extractor_t& operator=(const base_norm_extractor_t&) = delete;
  base_norm_extractor_t(base_norm_extractor_t&&) = delete;
  base_norm_extractor_t& operator=(base_norm_extractor_t&&) = delete;

private:
  std::size_t m_num_neighbors = 0;  ///< 近邻数量 / Number of neighbors
};  // class base_norm_extractor_t

}  // namespace toolbox::pcl