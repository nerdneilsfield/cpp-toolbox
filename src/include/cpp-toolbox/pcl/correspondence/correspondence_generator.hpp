#pragma once

#include <cpp-toolbox/pcl/descriptors/base_descriptor_extractor.hpp>
#include <cpp-toolbox/pcl/knn/base_knn.hpp>

namespace toolbox::pcl
{

struct correspondence_t
{
  std::size_t src_idx;
  std::size_t dst_idx;
  float distance;
};  // struct correspondence_t

template<typename DataType, typename Signature, typename KNN>
class correspondence_generator_t
{
public:
  using point_cloud = toolbox::types::point_cloud_t<DataType>;
  using point_cloud_ptr = std::shared_ptr<point_cloud>;
  using signatures_ptr = std::shared_ptr<std::vector<Signature>>;
  using keypoint_indices_ptr = std::shared_ptr<std::vector<std::size_t>>;

  correspondence_generator_t() = default;
  correspondence_generator_t(const correspondence_generator_t&) = delete;
  correspondence_generator_t(correspondence_generator_t&&) = default;
  correspondence_generator_t& operator=(const correspondence_generator_t&) =
      delete;
  correspondence_generator_t& operator=(correspondence_generator_t&&) = default;
  ~correspondence_generator_t() = default;

  void set_knn(std::shared_ptr<KNN> knn) { m_knn = knn; }

  void set_source(const point_cloud_ptr& src_cloud,
                  const signatures_ptr& src_descriptors,
                  const keypoint_indices_ptr& src_keypoint_indices)
  {
    m_src_cloud = src_cloud;
    m_src_descriptors = src_descriptors;
    m_src_keypoint_indices = src_keypoint_indices;
  }

  void set_destination(const point_cloud_ptr& dst_cloud,
                       const signatures_ptr& dst_descriptors,
                       const keypoint_indices_ptr& dst_keypoint_indices)
  {
    m_dst_cloud = dst_cloud;
    m_dst_descriptors = dst_descriptors;
    m_dst_keypoint_indices = dst_keypoint_indices;
  }

  void set_ratio(float ratio) { m_ratio = ratio; }

  void set_mutual_verification(bool mutual_verification)
  {
    m_mutual_verification = mutual_verification;
  }

  void set_distance_threshold(float distance_threshold)
  {
    m_distance_threshold = distance_threshold;
  }

  void compute(std::vector<correspondence_t>& correspondences)
  {
    // !TODO: implement, you can separate the computation into small private
    // functions
  }

private:
  std::shared_ptr<KNN> m_knn;
  float m_ratio = 0.8F;
  float m_distance_threshold = 0.05F;
  // do mutual verification
  bool m_mutual_verification = true;

  point_cloud_ptr m_src_cloud;
  point_cloud_ptr m_dst_cloud;
  signatures_ptr m_src_descriptors;
  signatures_ptr m_dst_descriptors;
  keypoint_indices_ptr m_src_keypoint_indices;
  keypoint_indices_ptr m_dst_keypoint_indices;

  std::vector<correspondence_t> m_correspondences;
};  // namespace toolbox::pcl

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/correspondence/impl/correspondence_generator_impl.hpp>