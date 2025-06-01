#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/norm/base_norm.hpp>

namespace toolbox::pcl
{

/**
 * @brief 基于PCA的法向量提取器 / PCA-based normal extractor
 * 
 * 该类使用主成分分析（PCA）来估计点云中每个点的法向量。通过分析每个点的邻域，
 * 计算协方差矩阵的特征向量，最小特征值对应的特征向量即为法向量方向。
 * This class uses Principal Component Analysis (PCA) to estimate normals for each point
 * in the point cloud. By analyzing the neighborhood of each point, it computes the
 * eigenvectors of the covariance matrix, where the eigenvector corresponding to the
 * smallest eigenvalue represents the normal direction.
 * 
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or double)
 * @tparam KNN KNN搜索算法类型，默认为kdtree_generic_t / KNN search algorithm type, default is kdtree_generic_t
 * 
 * @code
 * // 基本使用示例 / Basic usage example
 * pca_norm_extractor_t<float> norm_extractor;
 * point_cloud_t<float> cloud = load_point_cloud("data.pcd");
 * 
 * // 设置输入和参数 / Set input and parameters
 * norm_extractor.set_input(cloud);
 * norm_extractor.set_num_neighbors(30);  // 使用30个近邻点 / Use 30 neighbors
 * 
 * // 提取法向量 / Extract normals
 * point_cloud_t<float> normals = norm_extractor.extract();
 * @endcode
 * 
 * @code
 * // 使用并行计算加速 / Using parallel computation for acceleration
 * pca_norm_extractor_t<float> norm_extractor;
 * norm_extractor.enable_parallel(true);  // 启用并行 / Enable parallel
 * norm_extractor.set_input(large_cloud);
 * norm_extractor.set_num_neighbors(50);
 * 
 * // 使用自定义KNN算法 / Using custom KNN algorithm
 * bfknn_parallel_t<float> custom_knn;
 * norm_extractor.set_knn(custom_knn);
 * 
 * auto normals = norm_extractor.extract();
 * @endcode
 */
template<typename DataType, typename KNN = kdtree_generic_t<point_t<DataType>, toolbox::metrics::L2Metric<DataType>>>
class CPP_TOOLBOX_EXPORT pca_norm_extractor_t
    : public base_norm_extractor_t<pca_norm_extractor_t<DataType, KNN>,
                                   DataType,
                                   KNN>
{
public:
  using base_type =
      base_norm_extractor_t<pca_norm_extractor_t<DataType, KNN>, DataType, KNN>;
  using data_type = typename base_type::data_type;
  using knn_type = typename base_type::knn_type;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;

  /**
   * @brief 设置输入点云的实现 / Implementation of setting input point cloud
   * @param cloud 输入点云 / Input point cloud
   * @return 点云中的点数 / Number of points in the cloud
   */
  std::size_t set_input_impl(const point_cloud& cloud);

  /**
   * @brief 设置输入点云的实现（智能指针版本） / Implementation of setting input point cloud (smart pointer version)
   * @param cloud 输入点云的智能指针 / Smart pointer to input point cloud
   * @return 点云中的点数 / Number of points in the cloud
   */
  std::size_t set_input_impl(const point_cloud_ptr& cloud);

  /**
   * @brief 设置KNN搜索算法的实现 / Implementation of setting KNN search algorithm
   * @param knn KNN搜索算法对象 / KNN search algorithm object
   * @return 设置结果 / Setting result
   */
  std::size_t set_knn_impl(const knn_type& knn);

  /**
   * @brief 设置近邻数量的实现 / Implementation of setting number of neighbors
   * @param num_neighbors 近邻数量 / Number of neighbors
   * @return 实际设置的近邻数量 / Actually set number of neighbors
   * 
   * @note 建议使用10-50个近邻点，太少可能导致不稳定，太多会过度平滑 /
   * It's recommended to use 10-50 neighbors, too few may cause instability,
   * too many will over-smooth
   */
  std::size_t set_num_neighbors_impl(std::size_t num_neighbors);

  /**
   * @brief 提取法向量的实现 / Implementation of extracting normals
   * @return 包含法向量的点云 / Point cloud containing normals
   */
  point_cloud extract_impl();

  /**
   * @brief 提取法向量到指定输出的实现 / Implementation of extracting normals to specified output
   * @param output [out] 输出点云的智能指针 / Smart pointer to output point cloud
   */
  void extract_impl(point_cloud_ptr output);

  /**
   * @brief 启用或禁用并行计算 / Enable or disable parallel computation
   * @param enable true启用并行，false禁用 / true to enable parallel, false to disable
   * 
   * @code
   * // 对于大规模点云，启用并行可显著提升性能 / For large point clouds, enabling parallel can significantly improve performance
   * if (cloud.size() > 10000) {
   *     norm_extractor.enable_parallel(true);
   * }
   * @endcode
   */
  void enable_parallel(bool enable) { m_enable_parallel = enable; }

private:
  /**
   * @brief 计算指定范围内点的法向量 / Compute normals for points in specified range
   * @param output 输出点云 / Output point cloud
   * @param start_idx 起始索引 / Start index
   * @param end_idx 结束索引 / End index
   */
  void compute_normals_range(point_cloud_ptr output, std::size_t start_idx, std::size_t end_idx);
  
  /**
   * @brief 使用PCA计算单个点的法向量 / Compute normal for a single point using PCA
   * @param indices 近邻点的索引 / Indices of neighbor points
   * @return 计算得到的法向量 / Computed normal vector
   */
  point_t<data_type> compute_pca_normal(const std::vector<std::size_t>& indices);

  bool m_enable_parallel = false;  ///< 是否启用并行计算 / Whether to enable parallel computation
  std::size_t m_num_neighbors = 0;  ///< 近邻数量 / Number of neighbors
  point_cloud_ptr m_cloud;  ///< 输入点云 / Input point cloud
  knn_type* m_knn = nullptr;  ///< KNN搜索算法指针 / Pointer to KNN search algorithm
};  // class pca_norm_extractor_t

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/norm/impl/pca_norm_impl.hpp>