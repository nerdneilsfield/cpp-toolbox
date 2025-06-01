#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>
#include <numeric>
#include <vector>

#include <cpp-toolbox/base/thread_pool_singleton.hpp>

namespace toolbox::pcl
{

// Generic parallel brute-force KNN implementation
template<typename Element, typename Metric>
std::size_t bfknn_parallel_generic_t<Element, Metric>::set_input_impl(const container_type& data)
{
  m_data = std::make_shared<container_type>(data);
  return m_data->size();
}

template<typename Element, typename Metric>
std::size_t bfknn_parallel_generic_t<Element, Metric>::set_input_impl(const container_ptr& data)
{
  m_data = data;
  return m_data ? m_data->size() : 0;
}

template<typename Element, typename Metric>
void bfknn_parallel_generic_t<Element, Metric>::set_metric_impl(const metric_type& metric)
{
  m_compile_time_metric = metric;
  m_use_runtime_metric = false;
}

template<typename Element, typename Metric>
void bfknn_parallel_generic_t<Element, Metric>::set_metric_impl(
    std::shared_ptr<toolbox::metrics::IMetric<typename Element::value_type>> metric)
{
  m_runtime_metric = metric;
  m_use_runtime_metric = true;
}

template<typename Element, typename Metric>
bool bfknn_parallel_generic_t<Element, Metric>::kneighbors_impl(
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

  // For small datasets or when parallel is disabled, use sequential version
  if (!m_parallel_enabled || data_size < k_parallel_threshold)
  {
    // Sequential implementation (same as non-parallel version)
    std::vector<std::pair<distance_type, std::size_t>> distance_index_pairs;
    distance_index_pairs.reserve(data_size);

    for (std::size_t i = 0; i < data_size; ++i)
    {
      distance_type dist;
      if (m_use_runtime_metric && m_runtime_metric)
      {
        dist = (*m_runtime_metric)(query, (*m_data)[i]);
      }
      else
      {
        dist = m_compile_time_metric(query, (*m_data)[i]);
      }
      distance_index_pairs.emplace_back(dist, i);
    }

    std::partial_sort(distance_index_pairs.begin(),
                      distance_index_pairs.begin() + num_neighbors,
                      distance_index_pairs.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });

    indices.resize(num_neighbors);
    distances.resize(num_neighbors);
    for (std::size_t i = 0; i < num_neighbors; ++i)
    {
      distances[i] = distance_index_pairs[i].first;
      indices[i] = distance_index_pairs[i].second;
    }
  }
  else
  {
    // Parallel implementation
    auto& thread_pool = toolbox::base::thread_pool_singleton_t::instance();
    const std::size_t num_threads = thread_pool.get_thread_count();
    const std::size_t chunk_size = (data_size + num_threads - 1) / num_threads;

    // Thread-local storage for distance-index pairs
    std::vector<std::vector<std::pair<distance_type, std::size_t>>> thread_results(num_threads);
    std::vector<std::future<void>> futures;

    // Launch parallel tasks
    for (std::size_t t = 0; t < num_threads; ++t)
    {
      const std::size_t start = t * chunk_size;
      const std::size_t end = std::min(start + chunk_size, data_size);
      
      if (start >= data_size) break;

      futures.emplace_back(thread_pool.submit([this, &query, start, end, t, &thread_results]() {
        auto& local_results = thread_results[t];
        local_results.reserve(end - start);

        for (std::size_t i = start; i < end; ++i)
        {
          distance_type dist;
          if (m_use_runtime_metric && m_runtime_metric)
          {
            dist = (*m_runtime_metric)(query, (*m_data)[i]);
          }
          else
          {
            dist = m_compile_time_metric(query, (*m_data)[i]);
          }
          local_results.emplace_back(dist, i);
        }
      }));
    }

    // Wait for all tasks to complete
    for (auto& future : futures)
    {
      future.wait();
    }

    // Merge results from all threads
    std::vector<std::pair<distance_type, std::size_t>> all_results;
    all_results.reserve(data_size);
    for (const auto& thread_result : thread_results)
    {
      all_results.insert(all_results.end(), thread_result.begin(), thread_result.end());
    }

    // Find k nearest neighbors
    std::partial_sort(all_results.begin(),
                      all_results.begin() + num_neighbors,
                      all_results.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });

    indices.resize(num_neighbors);
    distances.resize(num_neighbors);
    for (std::size_t i = 0; i < num_neighbors; ++i)
    {
      distances[i] = all_results[i].first;
      indices[i] = all_results[i].second;
    }
  }

  return true;
}

template<typename Element, typename Metric>
bool bfknn_parallel_generic_t<Element, Metric>::radius_neighbors_impl(
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

  // For small datasets or when parallel is disabled, use sequential version
  if (!m_parallel_enabled || data_size < k_parallel_threshold)
  {
    std::vector<std::pair<distance_type, std::size_t>> distance_index_pairs;

    for (std::size_t i = 0; i < data_size; ++i)
    {
      distance_type dist;
      if (m_use_runtime_metric && m_runtime_metric)
      {
        dist = (*m_runtime_metric)(query, (*m_data)[i]);
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

    std::sort(distance_index_pairs.begin(), distance_index_pairs.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    indices.reserve(distance_index_pairs.size());
    distances.reserve(distance_index_pairs.size());
    for (const auto& [dist, idx] : distance_index_pairs)
    {
      distances.push_back(dist);
      indices.push_back(idx);
    }
  }
  else
  {
    // Parallel implementation
    auto& thread_pool = toolbox::base::thread_pool_singleton_t::instance();
    const std::size_t num_threads = thread_pool.get_thread_count();
    const std::size_t chunk_size = (data_size + num_threads - 1) / num_threads;

    // Thread-local storage for results
    std::vector<std::vector<std::pair<distance_type, std::size_t>>> thread_results(num_threads);
    std::vector<std::future<void>> futures;

    // Launch parallel tasks
    for (std::size_t t = 0; t < num_threads; ++t)
    {
      const std::size_t start = t * chunk_size;
      const std::size_t end = std::min(start + chunk_size, data_size);
      
      if (start >= data_size) break;

      futures.emplace_back(thread_pool.submit([this, &query, radius, start, end, t, &thread_results]() {
        auto& local_results = thread_results[t];

        for (std::size_t i = start; i < end; ++i)
        {
          distance_type dist;
          if (m_use_runtime_metric && m_runtime_metric)
          {
            dist = (*m_runtime_metric)(query, (*m_data)[i]);
          }
          else
          {
            dist = m_compile_time_metric(query, (*m_data)[i]);
          }
          
          if (dist <= radius)
          {
            local_results.emplace_back(dist, i);
          }
        }
      }));
    }

    // Wait for all tasks to complete
    for (auto& future : futures)
    {
      future.wait();
    }

    // Merge and sort results
    std::vector<std::pair<distance_type, std::size_t>> all_results;
    for (const auto& thread_result : thread_results)
    {
      all_results.insert(all_results.end(), thread_result.begin(), thread_result.end());
    }

    std::sort(all_results.begin(), all_results.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    indices.reserve(all_results.size());
    distances.reserve(all_results.size());
    for (const auto& [dist, idx] : all_results)
    {
      distances.push_back(dist);
      indices.push_back(idx);
    }
  }

  return true;
}

// Legacy parallel brute-force KNN implementation
template<typename DataType>
std::size_t bfknn_parallel_t<DataType>::set_input_impl(const point_cloud& cloud)
{
  // Convert point cloud to vector of points
  std::vector<point_t<DataType>> points(cloud.points.begin(), cloud.points.end());
  return m_impl.set_input(points);
}

template<typename DataType>
std::size_t bfknn_parallel_t<DataType>::set_input_impl(const point_cloud_ptr& cloud)
{
  if (!cloud) return 0;
  return set_input_impl(*cloud);
}

template<typename DataType>
std::size_t bfknn_parallel_t<DataType>::set_metric_impl(metric_type_t metric)
{
  m_metric = metric;
  
  // Create appropriate metric based on enum
  switch (metric)
  {
    case metric_type_t::euclidean:
      m_impl.set_metric(toolbox::metrics::L2Metric<DataType>{});
      break;
    case metric_type_t::manhattan:
      m_impl.set_metric(toolbox::metrics::L1Metric<DataType>{});
      break;
    case metric_type_t::chebyshev:
      m_impl.set_metric(toolbox::metrics::LinfMetric<DataType>{});
      break;
    case metric_type_t::minkowski:
      m_impl.set_metric(toolbox::metrics::GeneralizedLpMetric<DataType>{3.0}); // Default p=3
      break;
    default:
      m_impl.set_metric(toolbox::metrics::L2Metric<DataType>{});
      break;
  }
  
  return 0;
}

template<typename DataType>
bool bfknn_parallel_t<DataType>::kneighbors_impl(
    const point_t<data_type>& query_point,
    std::size_t num_neighbors,
    std::vector<std::size_t>& indices,
    std::vector<data_type>& distances)
{
  return m_impl.kneighbors(query_point, num_neighbors, indices, distances);
}

template<typename DataType>
bool bfknn_parallel_t<DataType>::radius_neighbors_impl(
    const point_t<data_type>& query_point,
    data_type radius,
    std::vector<std::size_t>& indices,
    std::vector<data_type>& distances)
{
  return m_impl.radius_neighbors(query_point, radius, indices, distances);
}

}  // namespace toolbox::pcl