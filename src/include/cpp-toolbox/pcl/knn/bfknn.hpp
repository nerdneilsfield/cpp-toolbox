#pragma once

#include <cpp-toolbox/pcl/knn/base_knn.hpp>
#include <cpp-toolbox/metrics/metric_factory.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>

namespace toolbox::pcl
{

/**
 * @brief 暴力K近邻搜索算法的通用实现 / Generic brute-force K-nearest neighbors search implementation
 * 
 * 该类通过遍历所有数据点来查找最近邻，适用于小规模数据集或需要精确结果的场景。
 * 支持任意元素类型和度量方式。 / This class finds nearest neighbors by iterating through all data points,
 * suitable for small datasets or scenarios requiring exact results. Supports arbitrary element types and metrics.
 * 
 * @tparam Element 元素类型（如point_t<float>） / Element type (e.g., point_t<float>)
 * @tparam Metric 度量类型（默认为L2度量） / Metric type (default is L2 metric)
 * 
 * @code
 * // 基本使用示例 / Basic usage example
 * bfknn_generic_t<point_t<float>, L2Metric<float>> knn;
 * std::vector<point_t<float>> points = generate_points();
 * knn.set_input(points);
 * 
 * // K近邻搜索 / K-nearest neighbors search
 * point_t<float> query = {1.0f, 2.0f, 3.0f};
 * std::vector<std::size_t> indices;
 * std::vector<float> distances;
 * knn.kneighbors(query, 5, indices, distances);
 * @endcode
 * 
 * @code
 * // 使用不同的度量 / Using different metrics
 * bfknn_generic_t<point_t<float>, L1Metric<float>> knn_l1;
 * knn_l1.set_input(points);
 * 
 * // 运行时切换度量 / Runtime metric switching
 * auto metric = MetricFactory<float>::instance().create("cosine");
 * knn.set_metric(std::move(metric));
 * @endcode
 */
template<typename Element, typename Metric = toolbox::metrics::L2Metric<typename Element::value_type>>
class CPP_TOOLBOX_EXPORT bfknn_generic_t : public base_knn_generic_t<bfknn_generic_t<Element, Metric>, Element, Metric>
{
public:
  using base_type = base_knn_generic_t<bfknn_generic_t<Element, Metric>, Element, Metric>;
  using traits_type = typename base_type::traits_type;
  using element_type = typename traits_type::element_type;
  using metric_type = typename traits_type::metric_type;
  using distance_type = typename traits_type::distance_type;
  using container_type = typename base_type::container_type;
  using container_ptr = typename base_type::container_ptr;
  using value_type = typename Element::value_type;

  bfknn_generic_t() = default;
  ~bfknn_generic_t() = default;

  bfknn_generic_t(const bfknn_generic_t&) = delete;
  bfknn_generic_t& operator=(const bfknn_generic_t&) = delete;
  bfknn_generic_t(bfknn_generic_t&&) = delete;
  bfknn_generic_t& operator=(bfknn_generic_t&&) = delete;

  /**
   * @brief 设置输入数据的实现 / Implementation of setting input data
   * @param data 输入数据容器 / Input data container
   * @return 数据点的数量 / Number of data points
   */
  std::size_t set_input_impl(const container_type& data);
  
  /**
   * @brief 设置输入数据的实现（智能指针版本） / Implementation of setting input data (smart pointer version)
   * @param data 输入数据容器的智能指针 / Smart pointer to input data container
   * @return 数据点的数量 / Number of data points
   */
  std::size_t set_input_impl(const container_ptr& data);
  
  /**
   * @brief 设置度量方式的实现（编译时版本） / Implementation of setting metric (compile-time version)
   * @param metric 度量对象 / Metric object
   */
  void set_metric_impl(const metric_type& metric);
  
  /**
   * @brief 设置度量方式的实现（运行时版本 - shared_ptr） / Implementation of setting metric (runtime version - shared_ptr)
   * @tparam T 数据类型 / Data type
   * @param metric 度量接口的智能指针 / Smart pointer to metric interface
   */
  template<typename T = typename Element::value_type>
  void set_metric_impl(std::shared_ptr<toolbox::metrics::IMetric<T>> metric);
  
  /**
   * @brief 设置度量方式的实现（运行时版本 - unique_ptr） / Implementation of setting metric (runtime version - unique_ptr)
   * @tparam T 数据类型 / Data type
   * @param metric 度量接口的独占指针 / Unique pointer to metric interface
   */
  template<typename T = typename Element::value_type>
  void set_metric_impl(std::unique_ptr<toolbox::metrics::IMetric<T>> metric);

  /**
   * @brief K近邻搜索的实现 / Implementation of K-nearest neighbors search
   * @param query 查询点 / Query point
   * @param num_neighbors 要查找的最近邻数量 / Number of nearest neighbors to find
   * @param indices [out] 找到的最近邻索引 / Indices of found nearest neighbors
   * @param distances [out] 对应的距离 / Corresponding distances
   * @return 是否成功 / Whether successful
   */
  bool kneighbors_impl(const element_type& query,
                       std::size_t num_neighbors,
                       std::vector<std::size_t>& indices,
                       std::vector<distance_type>& distances);

  /**
   * @brief 半径近邻搜索的实现 / Implementation of radius neighbors search
   * @param query 查询点 / Query point
   * @param radius 搜索半径 / Search radius
   * @param indices [out] 半径内所有点的索引 / Indices of all points within radius
   * @param distances [out] 对应的距离 / Corresponding distances
   * @return 是否成功 / Whether successful
   */
  bool radius_neighbors_impl(const element_type& query,
                             distance_type radius,
                             std::vector<std::size_t>& indices,
                             std::vector<distance_type>& distances);

private:
  container_ptr m_data;  ///< 存储的数据点 / Stored data points
  metric_type m_compile_time_metric;  ///< 编译时度量对象 / Compile-time metric object
  std::shared_ptr<toolbox::metrics::IMetric<typename Element::value_type>> m_runtime_metric;  ///< 运行时度量对象 / Runtime metric object
  bool m_use_runtime_metric = false;  ///< 是否使用运行时度量 / Whether to use runtime metric
};

/**
 * @brief 用于点云的暴力KNN类型别名 / Type alias for brute-force KNN with point clouds
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or double)
 * 
 * @code
 * // 使用类型别名简化代码 / Using type alias to simplify code
 * bfknn_t<float> knn;
 * point_cloud_t<float> cloud = load_point_cloud("data.pcd");
 * knn.set_input(cloud);
 * 
 * point_t<float> query = {1.0f, 2.0f, 3.0f};
 * std::vector<std::size_t> indices;
 * std::vector<float> distances;
 * knn.kneighbors(query, 10, indices, distances);
 * @endcode
 */
template<typename DataType>
using bfknn_t = bfknn_generic_t<point_t<DataType>, toolbox::metrics::L2Metric<DataType>>;

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/knn/impl/bfknn_impl.hpp>