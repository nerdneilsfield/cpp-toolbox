#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>
#include <cpp-toolbox/metrics/point_metric_adapter.hpp>

namespace toolbox::pcl
{

// Generic brute-force KNN implementation
template<typename Element, typename Metric>
std::size_t bfknn_generic_t<Element, Metric>::set_input_impl(const container_type& data)
{
  m_data = std::make_shared<container_type>(data);
  return m_data->size();
}

template<typename Element, typename Metric>
std::size_t bfknn_generic_t<Element, Metric>::set_input_impl(const container_ptr& data)
{
  m_data = data;
  return m_data ? m_data->size() : 0;
}

template<typename Element, typename Metric>
void bfknn_generic_t<Element, Metric>::set_metric_impl(const metric_type& metric)
{
  m_compile_time_metric = metric;
  m_use_runtime_metric = false;
}

template<typename Element, typename Metric>
template<typename T>
void bfknn_generic_t<Element, Metric>::set_metric_impl(
    std::shared_ptr<toolbox::metrics::IMetric<T>> metric)
{
  // For point types, we need to create an adapter
  if constexpr (std::is_same_v<Element, toolbox::types::point_t<T>>) {
    m_runtime_metric = std::make_shared<toolbox::metrics::PointMetricAdapter<T>>(metric);
  } else {
    m_runtime_metric = metric;
  }
  m_use_runtime_metric = true;
}

template<typename Element, typename Metric>
template<typename T>
void bfknn_generic_t<Element, Metric>::set_metric_impl(
    std::unique_ptr<toolbox::metrics::IMetric<T>> metric)
{
  // Convert unique_ptr to shared_ptr
  auto shared_metric = std::shared_ptr<toolbox::metrics::IMetric<T>>(std::move(metric));
  set_metric_impl(shared_metric);
}

template<typename Element, typename Metric>
bool bfknn_generic_t<Element, Metric>::kneighbors_impl(
    const element_type& query,
    std::size_t num_neighbors,
    std::vector<std::size_t>& indices,
    std::vector<distance_type>& distances)
{
  if (!m_data || m_data->empty())
  {
    return false;
  }

  const std::size_t data_size = m_data->size();
  num_neighbors = std::min(num_neighbors, data_size);

  // Compute all distances
  std::vector<std::pair<distance_type, std::size_t>> distance_index_pairs;
  distance_index_pairs.reserve(data_size);

  for (std::size_t i = 0; i < data_size; ++i)
  {
    distance_type dist;
    if (m_use_runtime_metric && m_runtime_metric)
    {
      if constexpr (std::is_same_v<Element, toolbox::types::point_t<typename Element::value_type>>) {
        // For point types, convert to arrays and use distance method
        value_type arr_query[3] = {query.x, query.y, query.z};
        value_type arr_data[3] = {(*m_data)[i].x, (*m_data)[i].y, (*m_data)[i].z};
        dist = m_runtime_metric->distance(arr_query, arr_data, 3);
      } else {
        // For generic types, assume they have data() and size() methods
        dist = m_runtime_metric->distance(query, (*m_data)[i]);
      }
    }
    else
    {
      dist = m_compile_time_metric(query, (*m_data)[i]);
    }
    distance_index_pairs.emplace_back(dist, i);
  }

  // Partial sort to get k nearest neighbors
  std::partial_sort(distance_index_pairs.begin(),
                    distance_index_pairs.begin() + num_neighbors,
                    distance_index_pairs.end(),
                    [](const auto& a, const auto& b) { return a.first < b.first; });

  // Extract results
  indices.resize(num_neighbors);
  distances.resize(num_neighbors);
  for (std::size_t i = 0; i < num_neighbors; ++i)
  {
    distances[i] = distance_index_pairs[i].first;
    indices[i] = distance_index_pairs[i].second;
  }

  return true;
}

template<typename Element, typename Metric>
bool bfknn_generic_t<Element, Metric>::radius_neighbors_impl(
    const element_type& query,
    distance_type radius,
    std::vector<std::size_t>& indices,
    std::vector<distance_type>& distances)
{
  if (!m_data || m_data->empty() || radius <= 0)
  {
    return false;
  }

  indices.clear();
  distances.clear();

  const std::size_t data_size = m_data->size();
  std::vector<std::pair<distance_type, std::size_t>> distance_index_pairs;

  for (std::size_t i = 0; i < data_size; ++i)
  {
    distance_type dist;
    if (m_use_runtime_metric && m_runtime_metric)
    {
      if constexpr (std::is_same_v<Element, toolbox::types::point_t<typename Element::value_type>>) {
        // For point types, convert to arrays and use distance method
        value_type arr_query[3] = {query.x, query.y, query.z};
        value_type arr_data[3] = {(*m_data)[i].x, (*m_data)[i].y, (*m_data)[i].z};
        dist = m_runtime_metric->distance(arr_query, arr_data, 3);
      } else {
        // For generic types, assume they have data() and size() methods
        dist = m_runtime_metric->distance(query, (*m_data)[i]);
      }
    }
    else
    {
      dist = m_compile_time_metric(query, (*m_data)[i]);
    }
    
    if (dist <= radius)
    {
      distance_index_pairs.emplace_back(dist, i);
    }
  }

  // Sort by distance
  std::sort(distance_index_pairs.begin(), distance_index_pairs.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });

  // Extract results
  indices.reserve(distance_index_pairs.size());
  distances.reserve(distance_index_pairs.size());
  for (const auto& [dist, idx] : distance_index_pairs)
  {
    distances.push_back(dist);
    indices.push_back(idx);
  }

  return true;
}

}  // namespace toolbox::pcl