#pragma once

#include <memory>
#include <type_traits>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>
#include <cpp-toolbox/metrics/metric_factory.hpp>

namespace toolbox::pcl
{

using toolbox::types::point_t;

/**
 * @brief KNN算法的特征定义 / KNN algorithm traits definition
 * @tparam Element 元素类型 / Element type
 * @tparam Metric 度量类型 / Metric type
 */
template<typename Element, typename Metric>
struct knn_traits
{
  using element_type = Element;     ///< 元素类型 / Element type
  using metric_type = Metric;       ///< 度量类型 / Metric type
  using distance_type = typename Metric::result_type;  ///< 距离类型 / Distance type
};

/**
 * @brief KNN算法的基类（CRTP模式） / Base class for KNN algorithms (CRTP pattern)
 * 
 * 这是一个通用的K近邻搜索算法基类，使用CRTP（Curiously Recurring Template Pattern）实现静态多态。
 * 支持任意元素类型和度量方式。 / This is a generic base class for K-nearest neighbor search algorithms,
 * using CRTP (Curiously Recurring Template Pattern) for static polymorphism. Supports arbitrary
 * element types and metrics.
 * 
 * @tparam Derived 派生类类型 / Derived class type
 * @tparam Element 元素类型（如point_t<float>） / Element type (e.g., point_t<float>)
 * @tparam Metric 度量类型（默认为L2度量） / Metric type (default is L2 metric)
 * 
 * @code
 * // 使用示例 / Usage example
 * using MyKNN = bfknn_generic_t<point_t<float>, L2Metric<float>>;
 * MyKNN knn;
 * 
 * // 设置输入数据 / Set input data
 * std::vector<point_t<float>> points = {...};
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
 * // 使用点云数据 / Using point cloud data
 * point_cloud_t<float> cloud = load_point_cloud();
 * knn.set_input(cloud);  // 自动使用cloud.points / Automatically uses cloud.points
 * 
 * // 半径搜索 / Radius search
 * float radius = 2.0f;
 * knn.radius_neighbors(query, radius, indices, distances);
 * @endcode
 */
template<typename Derived, typename Element, typename Metric = toolbox::metrics::L2Metric<typename Element::value_type>>
class CPP_TOOLBOX_EXPORT base_knn_generic_t
{
public:
  using traits_type = knn_traits<Element, Metric>;
  using element_type = typename traits_type::element_type;
  using metric_type = typename traits_type::metric_type;
  using distance_type = typename traits_type::distance_type;
  using container_type = std::vector<element_type>;
  using container_ptr = std::shared_ptr<container_type>;

  /**
   * @brief 设置输入数据 / Set input data
   * @param data 输入数据容器 / Input data container
   * @return 数据点的数量 / Number of data points
   */
  std::size_t set_input(const container_type& data)
  {
    return static_cast<Derived*>(this)->set_input_impl(data);
  }

  /**
   * @brief 设置输入数据（智能指针版本） / Set input data (smart pointer version)
   * @param data 输入数据容器的智能指针 / Smart pointer to input data container
   * @return 数据点的数量 / Number of data points
   */
  std::size_t set_input(const container_ptr& data)
  {
    return static_cast<Derived*>(this)->set_input_impl(data);
  }
  
  /**
   * @brief 设置点云输入数据（仅当Element为point_t时可用） / Set point cloud input data (only available when Element is point_t)
   * @tparam T 点云数据类型 / Point cloud data type
   * @param cloud 输入点云 / Input point cloud
   * @return 点的数量 / Number of points
   */
  template<typename T = typename Element::value_type, 
           typename = std::enable_if_t<std::is_same_v<Element, point_t<T>>>>
  std::size_t set_input(const toolbox::types::point_cloud_t<T>& cloud)
  {
    return set_input(cloud.points);
  }
  
  /**
   * @brief 设置点云输入数据（智能指针版本） / Set point cloud input data (smart pointer version)
   * @tparam T 点云数据类型 / Point cloud data type
   * @param cloud 输入点云的智能指针 / Smart pointer to input point cloud
   * @return 点的数量 / Number of points
   */
  template<typename T = typename Element::value_type,
           typename = std::enable_if_t<std::is_same_v<Element, point_t<T>>>>
  std::size_t set_input(const std::shared_ptr<toolbox::types::point_cloud_t<T>>& cloud)
  {
    if (!cloud) return 0;
    return set_input(cloud->points);
  }

  /**
   * @brief 设置度量方式（编译时版本） / Set metric (compile-time version)
   * @param metric 度量对象 / Metric object
   */
  void set_metric(const metric_type& metric)
  {
    static_cast<Derived*>(this)->set_metric_impl(metric);
  }

  /**
   * @brief 设置度量方式（运行时版本 - shared_ptr） / Set metric (runtime version - shared_ptr)
   * @tparam T 数据类型 / Data type
   * @param metric 度量接口的智能指针 / Smart pointer to metric interface
   */
  template<typename T = typename Element::value_type>
  void set_metric(std::shared_ptr<toolbox::metrics::IMetric<T>> metric)
  {
    static_cast<Derived*>(this)->set_metric_impl(metric);
  }
  
  /**
   * @brief 设置度量方式（运行时版本 - unique_ptr） / Set metric (runtime version - unique_ptr)
   * @tparam T 数据类型 / Data type
   * @param metric 度量接口的独占指针 / Unique pointer to metric interface
   */
  template<typename T = typename Element::value_type>
  void set_metric(std::unique_ptr<toolbox::metrics::IMetric<T>> metric)
  {
    static_cast<Derived*>(this)->set_metric_impl(std::move(metric));
  }

  /**
   * @brief K近邻搜索 / K-nearest neighbors search
   * @param query 查询点 / Query point
   * @param num_neighbors 要查找的最近邻数量 / Number of nearest neighbors to find
   * @param indices [out] 找到的最近邻索引 / Indices of found nearest neighbors
   * @param distances [out] 对应的距离 / Corresponding distances
   * @return 是否成功 / Whether successful
   * 
   * @code
   * // 示例：查找5个最近邻 / Example: Find 5 nearest neighbors
   * point_t<float> query = {1.0f, 2.0f, 3.0f};
   * std::vector<std::size_t> indices;
   * std::vector<float> distances;
   * 
   * if (knn.kneighbors(query, 5, indices, distances)) {
   *     for (size_t i = 0; i < indices.size(); ++i) {
   *         std::cout << "邻居 / Neighbor " << i << ": "
   *                   << "索引 / index = " << indices[i] 
   *                   << ", 距离 / distance = " << distances[i] << std::endl;
   *     }
   * }
   * @endcode
   */
  bool kneighbors(const element_type& query,
                  std::size_t num_neighbors,
                  std::vector<std::size_t>& indices,
                  std::vector<distance_type>& distances)
  {
    return static_cast<Derived*>(this)->kneighbors_impl(
        query, num_neighbors, indices, distances);
  }

  /**
   * @brief 半径近邻搜索 / Radius neighbors search
   * @param query 查询点 / Query point
   * @param radius 搜索半径 / Search radius
   * @param indices [out] 半径内所有点的索引 / Indices of all points within radius
   * @param distances [out] 对应的距离 / Corresponding distances
   * @return 是否成功 / Whether successful
   * 
   * @code
   * // 示例：查找半径2.0内的所有点 / Example: Find all points within radius 2.0
   * point_t<float> query = {1.0f, 2.0f, 3.0f};
   * std::vector<std::size_t> indices;
   * std::vector<float> distances;
   * 
   * if (knn.radius_neighbors(query, 2.0f, indices, distances)) {
   *     std::cout << "找到 / Found " << indices.size() 
   *               << " 个点在半径内 / points within radius" << std::endl;
   * }
   * @endcode
   */
  bool radius_neighbors(const element_type& query,
                        distance_type radius,
                        std::vector<std::size_t>& indices,
                        std::vector<distance_type>& distances)
  {
    return static_cast<Derived*>(this)->radius_neighbors_impl(
        query, radius, indices, distances);
  }

protected:
  base_knn_generic_t() = default;
  ~base_knn_generic_t() = default;

public:
  base_knn_generic_t(const base_knn_generic_t&) = delete;
  base_knn_generic_t& operator=(const base_knn_generic_t&) = delete;
  base_knn_generic_t(base_knn_generic_t&&) = delete;
  base_knn_generic_t& operator=(base_knn_generic_t&&) = delete;
};  // class base_knn_generic_t

}  // namespace toolbox::pcl