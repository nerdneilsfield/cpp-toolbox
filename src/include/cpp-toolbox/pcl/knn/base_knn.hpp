#pragma once

#include <memory>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::pcl
{

using toolbox::types::point_t;

enum class CPP_TOOLBOX_EXPORT metric_type_t : std::uint8_t
{
  euclidean,
  manhattan,
  chebyshev,
  minkowski,
};

template<typename Derived, typename DataType>
class CPP_TOOLBOX_EXPORT base_knn_t
{
public:
  using data_type = DataType;
  using point_cloud = toolbox::types::point_cloud_t<data_type>;
  using point_cloud_ptr =
      std::shared_ptr<toolbox::types::point_cloud_t<data_type>>;

  std::size_t set_input(const point_cloud& cloud)
  {
    return static_cast<Derived*>(this)->set_input_impl(cloud);
  }

  std::size_t set_input(const point_cloud_ptr& cloud)
  {
    return static_cast<Derived*>(this)->set_input_impl(cloud);
  }

  std::size_t set_metric(metric_type_t metric)
  {
    return static_cast<Derived*>(this)->set_metric_impl(metric);
  }

  [[nodiscard]] metric_type_t get_metric() const noexcept { return m_metric; }

  bool kneighbors(const point_t<data_type>& query_point,
                  std::size_t num_neighbors,
                  std::vector<std::size_t>& indices,
                  std::vector<data_type>& distances)
  {
    return static_cast<Derived*>(this)->kneighbors_impl(
        query_point, num_neighbors, indices, distances);
  }

  bool radius_neighbors(const point_t<data_type>& query_point,
                        data_type radius,
                        std::vector<std::size_t>& indices,
                        std::vector<data_type>& distances)
  {
    return static_cast<Derived*>(this)->radius_neighbors_impl(
        query_point, radius, indices, distances);
  }

protected:
  base_knn_t() = default;
  ~base_knn_t() = default;

public:
  base_knn_t(const base_knn_t&) = delete;
  base_knn_t& operator=(const base_knn_t&) = delete;
  base_knn_t(base_knn_t&&) = delete;
  base_knn_t& operator=(base_knn_t&&) = delete;

private:
  metric_type_t m_metric = metric_type_t::euclidean;
};  // class base_knn_t

}  // namespace toolbox::pcl