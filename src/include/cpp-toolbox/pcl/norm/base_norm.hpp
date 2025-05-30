#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/knn/base_knn.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::pcl
{

template<typename Derived, typename DataType, typename KNN>
class CPP_TOOLBOX_EXPORT base_norm_extractor_t
{
public:
  using data_type = DataType;
  using knn_type = KNN;
  using point_cloud = toolbox::types::point_cloud_t<data_type>;
  using point_cloud_ptr =
      std::shared_ptr<toolbox::types::point_cloud_t<data_type>>;

  base_norm_extractor_t() = default;
  ~base_norm_extractor_t() = default;

  std::size_t set_input(const point_cloud& cloud)
  {
    return static_cast<Derived*>(this)->set_input_impl(cloud);
  }

  std::size_t set_input(const point_cloud_ptr& cloud)
  {
    return static_cast<Derived*>(this)->set_input_impl(cloud);
  }

  std::size_t set_num_neighbors(std::size_t num_neighbors)
  {
    m_num_neighbors = num_neighbors;
    return static_cast<Derived*>(this)->set_num_neighbors_impl(num_neighbors);
  }

  [[nodiscard]] std::size_t get_num_neighbors() const noexcept
  {
    return m_num_neighbors;
  }

  std::size_t set_knn(const knn_type& knn)
  {
    return static_cast<Derived*>(this)->set_knn_impl(knn);
  }

  point_cloud extract() { return static_cast<Derived*>(this)->extract_impl(); }

  void extract(point_cloud_ptr output)
  {
    return static_cast<Derived*>(this)->extract_impl(output);
  }

  base_norm_extractor_t(const base_norm_extractor_t&) = delete;
  base_norm_extractor_t& operator=(const base_norm_extractor_t&) = delete;
  base_norm_extractor_t(base_norm_extractor_t&&) = delete;
  base_norm_extractor_t& operator=(base_norm_extractor_t&&) = delete;

private:
  std::size_t m_num_neighbors = 0;
};  // class base_norm_extractor_t

}  // namespace toolbox::pcl