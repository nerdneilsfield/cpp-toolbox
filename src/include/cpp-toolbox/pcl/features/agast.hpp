#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/features/base_features.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::pcl
{

// AGAST Features
template<typename DataType, typename KNN>
class CPP_TOOLBOX_EXPORT agast_features_extractor_t
    : public base_features_extractor_t<
          agast_features_extractor_t<DataType, KNN>,
          DataType>
{
public:
  using base_type =
      base_features_extractor_t<agast_features_extractor_t<DataType, KNN>,
                                DataType>;
  using data_type = typename base_type::data_type;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;
  using knn_type = KNN;

  agast_features_extractor_t() = default;
  ~agast_features_extractor_t() = default;

  std::size_t set_input_impl(const point_cloud& cloud);

  std::size_t set_input_impl(const point_cloud_ptr& cloud);

  point_cloud extract_impl();

  void extract_impl(point_cloud_ptr output);

  std::size_t set_num_neighbors(std::size_t num_neighbors)
  {
    m_num_neighbors = num_neighbors;
    return m_num_neighbors;
  }

  [[nodiscard]] std::size_t get_num_neighbors() const noexcept
  {
    return m_num_neighbors;
  }

  agast_features_extractor_t(const agast_features_extractor_t&) = delete;
  agast_features_extractor_t(agast_features_extractor_t&&) = delete;
  agast_features_extractor_t& operator=(const agast_features_extractor_t&) =
      delete;
  agast_features_extractor_t& operator=(agast_features_extractor_t&&) = delete;

private:
  std::size_t m_num_neighbors = 0;
  point_cloud_ptr m_input_cloud = nullptr;
  std::unique_ptr<knn_type> m_knn = nullptr;
};  // class agast_features_extractor_t

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/features/impl/agast_impl.hpp>