#pragma once

#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <algorithm>
#include <cmath>
#include <future>
#include <vector>

namespace toolbox::pcl
{

template<typename DataType, typename KNN>
std::size_t agast_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  initialize_test_pattern();
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t agast_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  initialize_test_pattern();
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t agast_keypoint_extractor_t<DataType, KNN>::set_knn_impl(const knn_type& knn)
{
  m_knn = const_cast<knn_type*>(&knn);
  if (m_cloud) {
    m_knn->set_input(*m_cloud);
  }
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t agast_keypoint_extractor_t<DataType, KNN>::set_search_radius_impl(data_type radius)
{
  m_pattern_radius = radius;
  initialize_test_pattern();
  return 0;
}

template<typename DataType, typename KNN>
void agast_keypoint_extractor_t<DataType, KNN>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

template<typename DataType, typename KNN>
void agast_keypoint_extractor_t<DataType, KNN>::initialize_test_pattern()
{
  m_test_pattern.clear();
  m_test_pattern.reserve(m_num_test_points);
  
  // Generate test points uniformly distributed on a sphere
  // Using Fibonacci spiral for uniform distribution
  const data_type golden_angle = static_cast<data_type>(M_PI * (3.0 - std::sqrt(5.0)));  // Golden angle
  
  for (std::size_t i = 0; i < m_num_test_points; ++i) {
    const data_type y = 1.0 - (2.0 * i) / (m_num_test_points - 1);  // y goes from 1 to -1
    const data_type radius = std::sqrt(1.0 - y * y);
    const data_type theta = golden_angle * i;
    
    TestPoint test_point;
    test_point.x = std::cos(theta) * radius * m_pattern_radius;
    test_point.y = y * m_pattern_radius;
    test_point.z = std::sin(theta) * radius * m_pattern_radius;
    
    m_test_pattern.push_back(test_point);
  }
}

template<typename DataType, typename KNN>
typename agast_keypoint_extractor_t<DataType, KNN>::data_type
agast_keypoint_extractor_t<DataType, KNN>::compute_test_value(
    const point_t<data_type>& center, 
    const TestPoint& test_point)
{
  // For 3D AGAST, we use local point density as the test value
  // Alternative: could use local height, curvature, or other geometric properties
  
  point_t<data_type> test_location;
  test_location.x = center.x + test_point.x;
  test_location.y = center.y + test_point.y;
  test_location.z = center.z + test_point.z;
  
  std::vector<std::size_t> neighbor_indices;
  std::vector<data_type> neighbor_distances;
  
  // Count points within a small radius around the test location
  const data_type count_radius = m_pattern_radius * 0.3;  // 30% of pattern radius
  m_knn->radius_neighbors(test_location, count_radius, neighbor_indices, neighbor_distances);
  
  // Return normalized density
  return static_cast<data_type>(neighbor_indices.size()) / (count_radius * count_radius * count_radius);
}

template<typename DataType, typename KNN>
bool agast_keypoint_extractor_t<DataType, KNN>::is_consecutive_arc(
    const std::vector<bool>& brighter, 
    const std::vector<bool>& darker)
{
  const std::size_t n = brighter.size();
  
  // Check for consecutive brighter points
  std::size_t max_consecutive_bright = 0;
  std::size_t current_consecutive_bright = 0;
  
  for (std::size_t i = 0; i < 2 * n; ++i) {  // Check twice to handle circular nature
    if (brighter[i % n]) {
      current_consecutive_bright++;
      max_consecutive_bright = std::max(max_consecutive_bright, current_consecutive_bright);
    } else {
      current_consecutive_bright = 0;
    }
  }
  
  if (max_consecutive_bright >= m_min_arc_length) {
    return true;
  }
  
  // Check for consecutive darker points
  std::size_t max_consecutive_dark = 0;
  std::size_t current_consecutive_dark = 0;
  
  for (std::size_t i = 0; i < 2 * n; ++i) {  // Check twice to handle circular nature
    if (darker[i % n]) {
      current_consecutive_dark++;
      max_consecutive_dark = std::max(max_consecutive_dark, current_consecutive_dark);
    } else {
      current_consecutive_dark = 0;
    }
  }
  
  return max_consecutive_dark >= m_min_arc_length;
}

template<typename DataType, typename KNN>
typename agast_keypoint_extractor_t<DataType, KNN>::AGASTInfo
agast_keypoint_extractor_t<DataType, KNN>::compute_agast_response(std::size_t point_idx)
{
  if (!m_cloud || !m_knn || point_idx >= m_cloud->size() || m_test_pattern.empty()) {
    return AGASTInfo{0, false};
  }

  const auto& center_point = m_cloud->points[point_idx];
  
  // Compute center value
  const data_type center_value = compute_test_value(center_point, TestPoint{0, 0, 0});
  
  // Test all pattern points
  std::vector<bool> brighter(m_test_pattern.size(), false);
  std::vector<bool> darker(m_test_pattern.size(), false);
  data_type score = 0;
  
  for (std::size_t i = 0; i < m_test_pattern.size(); ++i) {
    const data_type test_value = compute_test_value(center_point, m_test_pattern[i]);
    const data_type diff = test_value - center_value;
    
    if (diff > m_threshold) {
      brighter[i] = true;
      score += diff;
    } else if (diff < -m_threshold) {
      darker[i] = true;
      score += -diff;
    }
  }
  
  // Check if we have a consecutive arc of similar points
  const bool is_keypoint = is_consecutive_arc(brighter, darker);
  
  return AGASTInfo{score, is_keypoint};
}

template<typename DataType, typename KNN>
void agast_keypoint_extractor_t<DataType, KNN>::compute_agast_range(
    std::vector<AGASTInfo>& agast_responses,
    std::size_t start_idx, 
    std::size_t end_idx)
{
  for (std::size_t i = start_idx; i < end_idx; ++i) {
    agast_responses[i] = compute_agast_response(i);
  }
}

template<typename DataType, typename KNN>
std::vector<typename agast_keypoint_extractor_t<DataType, KNN>::AGASTInfo>
agast_keypoint_extractor_t<DataType, KNN>::compute_all_agast_responses()
{
  if (!m_cloud) {
    return {};
  }

  const std::size_t num_points = m_cloud->size();
  std::vector<AGASTInfo> agast_responses(num_points);

  if (m_enable_parallel && num_points > k_parallel_threshold) {
    // Parallel computation
    const std::size_t num_threads = toolbox::base::thread_pool_singleton_t::instance().get_thread_count();
    const std::size_t chunk_size = (num_points + num_threads - 1) / num_threads;
    
    std::vector<std::future<void>> futures;
    for (std::size_t t = 0; t < num_threads; ++t) {
      const std::size_t start_idx = t * chunk_size;
      const std::size_t end_idx = std::min(start_idx + chunk_size, num_points);
      
      if (start_idx < end_idx) {
        futures.emplace_back(
          toolbox::base::thread_pool_singleton_t::instance().submit(
            [this, &agast_responses, start_idx, end_idx]() {
              compute_agast_range(agast_responses, start_idx, end_idx);
            }
          )
        );
      }
    }
    
    // Wait for all threads to complete
    for (auto& future : futures) {
      future.wait();
    }
  } else {
    // Sequential computation
    compute_agast_range(agast_responses, 0, num_points);
  }

  return agast_responses;
}

template<typename DataType, typename KNN>
typename agast_keypoint_extractor_t<DataType, KNN>::indices_vector
agast_keypoint_extractor_t<DataType, KNN>::apply_non_maxima_suppression(
    const std::vector<AGASTInfo>& agast_responses)
{
  if (!m_cloud || agast_responses.empty()) {
    return {};
  }

  indices_vector keypoints;
  const std::size_t num_points = m_cloud->size();

  for (std::size_t i = 0; i < num_points; ++i) {
    const auto& current_response = agast_responses[i];
    
    // Skip non-keypoints
    if (!current_response.is_keypoint || current_response.score <= 0) {
      continue;
    }

    // Find neighbors within non-maxima suppression radius
    const auto& query_point = m_cloud->points[i];
    std::vector<std::size_t> neighbor_indices;
    std::vector<data_type> neighbor_distances;
    
    m_knn->radius_neighbors(query_point, m_non_maxima_radius, neighbor_indices, neighbor_distances);

    // Check if current point is local maximum
    bool is_local_maximum = true;
    for (const auto& neighbor_idx : neighbor_indices) {
      if (neighbor_idx != i && neighbor_idx < agast_responses.size()) {
        if (agast_responses[neighbor_idx].is_keypoint && 
            agast_responses[neighbor_idx].score > current_response.score) {
          is_local_maximum = false;
          break;
        }
      }
    }

    if (is_local_maximum) {
      keypoints.push_back(i);
    }
  }

  return keypoints;
}

template<typename DataType, typename KNN>
typename agast_keypoint_extractor_t<DataType, KNN>::indices_vector
agast_keypoint_extractor_t<DataType, KNN>::extract_impl()
{
  if (!m_cloud || !m_knn) {
    return {};
  }

  // Compute AGAST responses for all points
  auto agast_responses = compute_all_agast_responses();
  
  // Apply non-maxima suppression to find keypoints
  return apply_non_maxima_suppression(agast_responses);
}

template<typename DataType, typename KNN>
void agast_keypoint_extractor_t<DataType, KNN>::extract_impl(indices_vector& keypoint_indices)
{
  keypoint_indices = extract_impl();
}

template<typename DataType, typename KNN>
typename agast_keypoint_extractor_t<DataType, KNN>::point_cloud
agast_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl()
{
  auto keypoint_indices = extract_impl();
  
  point_cloud keypoints;
  keypoints.points.reserve(keypoint_indices.size());
  
  for (const auto& idx : keypoint_indices) {
    keypoints.points.push_back(m_cloud->points[idx]);
  }
  
  return keypoints;
}

template<typename DataType, typename KNN>
void agast_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl(point_cloud_ptr output)
{
  auto keypoint_indices = extract_impl();
  
  output->points.clear();
  output->points.reserve(keypoint_indices.size());
  
  for (const auto& idx : keypoint_indices) {
    output->points.push_back(m_cloud->points[idx]);
  }
}

}  // namespace toolbox::pcl