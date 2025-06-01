#pragma once

#include <cstddef>
#include <memory>

#include <cpp-toolbox/types/point.hpp>

namespace toolbox::pcl
{

template<class Derived>
struct base_signature_t
{
  bool operator==(const base_signature_t& other) const
  {
    return static_cast<const Derived*>(this)->operator==(other);
  }

  bool operator!=(const base_signature_t& other) const
  {
    return static_cast<const Derived*>(this)->operator!=(other);
  }
};  // struct base_signature_t

template<class Derived, typename DataType, typename Signature>
class base_descriptor_extractor_t
{
public:
  base_descriptor_extractor_t() = default;
  base_descriptor_extractor_t(const base_descriptor_extractor_t&) = delete;
  base_descriptor_extractor_t(base_descriptor_extractor_t&&) = default;
  base_descriptor_extractor_t& operator=(const base_descriptor_extractor_t&) =
      delete;
  base_descriptor_extractor_t& operator=(base_descriptor_extractor_t&&) =
      default;
  virtual ~base_descriptor_extractor_t() = default;

  void enable_parallel(bool enable)
  {
    static_cast<Derived*>(this)->enable_parallel_impl(enable);
  }

  void compute(const toolbox::types::point_cloud_t<DataType>& cloud,
               const std::vector<std::size_t>& keypoint_indices,
               std::vector<Signature>& descriptors)
  {
    static_cast<const Derived*>(this)->compute_impl(
        cloud, keypoint_indices, descriptors);
  }

  void compute(const toolbox::types::point_cloud_t<DataType>& cloud,
               const std::vector<std::size_t>& keypoint_indices,
               std::unique_ptr<std::vector<Signature>> descriptors)
  {
    static_cast<const Derived*>(this)->compute_impl(
        cloud, keypoint_indices, descriptors);
  }
};  // base_descriptor_extractor_t

}  // namespace toolbox::pcl