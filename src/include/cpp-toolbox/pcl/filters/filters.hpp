#pragma once

#include <memory>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::pcl
{

template<typename Derived, typename DataType>
class CPP_TOOLBOX_EXPORT filter_t
{
public:
  using data_type = DataType;
  using point_cloud = toolbox::types::point_cloud_t<data_type>;
  using point_cloud_ptr =
      std::shared_ptr<toolbox::types::point_cloud_t<data_type>>;

  std::size_t set_input(const point_cloud& cloud)
  {
    return static_cast<const Derived*>(this)->set_input_impl(cloud);
  }

  std::size_t set_input(const point_cloud_ptr& cloud)
  {
    return static_cast<const Derived*>(this)->set_input_impl(cloud);
  }

  void enable_parrallel(bool enable)
  {
    return static_cast<const Derived*>(this)->enable_parallel_impl(enable);
  }

  point_cloud filter()
  {
    return static_cast<const Derived*>(this)->filter_impl();
  }

  void filter(point_cloud_ptr output)
  {
    return static_cast<const Derived*>(this)->filter_impl(output);
  }

protected:
  filter_t() = default;
  ~filter_t() = default;

public:
  filter_t(const filter_t&) = delete;
  filter_t& operator=(const filter_t&) = delete;
  filter_t(filter_t&&) = delete;
  filter_t& operator=(filter_t&&) = delete;

};  // class filter_t

}  // namespace toolbox::pcl