#pragma once

#include <cpp-toolbox/pcl/filters/filters.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/utils/random.hpp>

namespace toolbox::pcl
{

template<typename DataType>
class CPP_TOOLBOX_EXPORT random_downsampling_t
    : public filter_t<random_downsampling_t<DataType>, DataType>
{
public:
  using data_type = DataType;
  using base_type = filter_t<random_downsampling_t<DataType>, DataType>;
  using point_cloud = toolbox::types::point_cloud_t<data_type>;
  using point_cloud_ptr =
      std::shared_ptr<toolbox::types::point_cloud_t<data_type>>;

  explicit random_downsampling_t(float ration)
      : m_ration(ration)
  {
  }
  ~random_downsampling_t() = default;

public:
  random_downsampling_t(const random_downsampling_t&) = delete;
  random_downsampling_t& operator=(const random_downsampling_t&) = delete;
  random_downsampling_t(random_downsampling_t&&) = delete;
  random_downsampling_t& operator=(random_downsampling_t&&) = delete;

  std::size_t set_input_impl(const point_cloud& cloud);
  std::size_t set_input_impl(const point_cloud_ptr& cloud);
  void enable_parallel_impl(bool enable);
  point_cloud filter_impl();
  void filter_impl(point_cloud_ptr output);

private:
  float m_ration = 1.0F;
  bool m_enable_parallel = false;
  point_cloud_ptr m_cloud;
};

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/filters/impl/random_downsampling_impl.hpp>