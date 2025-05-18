#pragma once

#include <cpp-toolbox/pcl/filters/filters.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/utils/random.hpp>

namespace toolbox::pcl
{

template<typename DataType>
class CPP_TOOLBOX_EXPORT voxel_grid_downsampling_t
    : public filter_t<voxel_grid_downsampling_t<DataType>, DataType>
{
public:
  using data_type = DataType;
  using base_type = filter_t<voxel_grid_downsampling_t<DataType>, DataType>;
  using point_cloud = toolbox::types::point_cloud_t<data_type>;
  using point_cloud_ptr =
      std::shared_ptr<toolbox::types::point_cloud_t<data_type>>;

  explicit voxel_grid_downsampling_t(float voxel_size)
      : m_voxel_size(voxel_size)
  {
  }
  ~voxel_grid_downsampling_t() = default;

public:
  voxel_grid_downsampling_t(const voxel_grid_downsampling_t&) = delete;
  voxel_grid_downsampling_t& operator=(const voxel_grid_downsampling_t&) =
      delete;
  voxel_grid_downsampling_t(voxel_grid_downsampling_t&&) = delete;
  voxel_grid_downsampling_t& operator=(voxel_grid_downsampling_t&&) = delete;

  std::size_t set_input_impl(const point_cloud& cloud);
  std::size_t set_input_impl(const point_cloud_ptr& cloud);
  void enable_parallel_impl(bool enable);
  point_cloud filter_impl();
  void filter_impl(point_cloud_ptr output);

private:
  float m_voxel_size = 1.0F;
  bool m_enable_parallel = false;
  point_cloud_ptr m_cloud;
};

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/filters/impl/voxel_grid_downsampling_impl.hpp>