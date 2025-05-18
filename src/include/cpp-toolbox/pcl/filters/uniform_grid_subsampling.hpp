#pragma once

#include <cpp-toolbox/pcl/filters/filters.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/utils/random.hpp>

namespace toolbox::pcl
{

template<typename DataType>
class CPP_TOOLBOX_EXPORT uniform_grid_subsampling_t
    : public filter_t<uniform_grid_subsampling_t<DataType>, DataType>
{
public:
  using data_type = DataType;
  using base_type = filter_t<uniform_grid_subsampling_t<DataType>, DataType>;
  using point_cloud = toolbox::types::point_cloud_t<data_type>;
  using point_cloud_ptr =
      std::shared_ptr<toolbox::types::point_cloud_t<data_type>>;

  explicit uniform_grid_subsampling_t(float grid_size)
      : m_grid_size(grid_size)
  {
  }
  ~uniform_grid_subsampling_t() = default;

public:
  uniform_grid_subsampling_t(const uniform_grid_subsampling_t&) = delete;
  uniform_grid_subsampling_t& operator=(const uniform_grid_subsampling_t&) =
      delete;
  uniform_grid_subsampling_t(uniform_grid_subsampling_t&&) = delete;
  uniform_grid_subsampling_t& operator=(uniform_grid_subsampling_t&&) = delete;

  std::size_t set_input_impl(const point_cloud& cloud);
  std::size_t set_input_impl(const point_cloud_ptr& cloud);
  void enable_parallel_impl(bool enable);
  point_cloud filter_impl();
  void filter_impl(point_cloud_ptr output);

private:
  float m_grid_size = 1.0F;
  bool m_enable_parallel = false;
  point_cloud_ptr m_cloud;
};

}  // namespace toolbox::pcl