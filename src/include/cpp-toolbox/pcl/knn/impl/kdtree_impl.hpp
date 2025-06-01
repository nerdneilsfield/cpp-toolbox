#pragma once

#include <algorithm>
#include <cmath>
#include <vector>
#include <cpp-toolbox/pcl/knn/bfknn.hpp>

namespace toolbox::pcl
{

// Generic KD-tree implementation
template<typename Element, typename Metric>
std::size_t kdtree_generic_t<Element, Metric>::set_input_impl(const container_type& data)
{
  m_data = std::make_shared<container_type>(data);
  if (validate_metric())
  {
    build_tree();
  }
  return m_data->size();
}

template<typename Element, typename Metric>
std::size_t kdtree_generic_t<Element, Metric>::set_input_impl(const container_ptr& data)
{
  m_data = data;
  if (m_data && !m_data->empty() && validate_metric())
  {
    build_tree();
  }
  return m_data ? m_data->size() : 0;
}

template<typename Element, typename Metric>
void kdtree_generic_t<Element, Metric>::set_metric_impl(const metric_type& metric)
{
  m_compile_time_metric = metric;
  m_use_runtime_metric = false;
  if (m_data && !m_data->empty() && validate_metric())
  {
    build_tree();
  }
}

template<typename Element, typename Metric>
void kdtree_generic_t<Element, Metric>::set_metric_impl(
    std::shared_ptr<toolbox::metrics::IMetric<value_type>> metric)
{
  m_runtime_metric = metric;
  m_use_runtime_metric = true;
  // Note: Runtime metrics are not supported by nanoflann KD-tree
  // This will use brute-force fallback in kneighbors/radius_neighbors
}

template<typename Element, typename Metric>
bool kdtree_generic_t<Element, Metric>::validate_metric() const
{
  // nanoflann KD-tree only supports L2 metric efficiently
  // For other metrics, we would need to fall back to brute-force
  if (m_use_runtime_metric)
  {
    return false; // Runtime metrics not supported by KD-tree
  }
  
  // Check if metric is L2 (we can check this at compile time for known metrics)
  return std::is_same_v<Metric, toolbox::metrics::L2Metric<value_type>>;
}

template<typename Element, typename Metric>
void kdtree_generic_t<Element, Metric>::build_tree()
{
  if (!m_data || m_data->empty())
  {
    m_kdtree.reset();
    m_adaptor.reset();
    return;
  }

  // Create adaptor
  m_adaptor = std::make_unique<data_adaptor_t>(m_data);

  // Create and build KD-tree
  m_kdtree = std::make_unique<kd_tree_t>(
      3,  // dimensions
      *m_adaptor,
      nanoflann::KDTreeSingleIndexAdaptorParams(m_max_leaf_size)
  );

  m_kdtree->buildIndex();
}

template<typename Element, typename Metric>
bool kdtree_generic_t<Element, Metric>::kneighbors_impl(
    const element_type& query,
    std::size_t num_neighbors,
    std::vector<std::size_t>& indices,
    std::vector<distance_type>& distances)
{
  if (!m_data || m_data->empty())
  {
    return false;
  }

  // If metric is not supported by KD-tree, fall back to brute-force
  if (!validate_metric())
  {
    bfknn_generic_t<Element, Metric> bfknn;
    bfknn.set_input(m_data);
    if (m_use_runtime_metric)
    {
      bfknn.set_metric(m_runtime_metric);
    }
    else
    {
      bfknn.set_metric(m_compile_time_metric);
    }
    return bfknn.kneighbors(query, num_neighbors, indices, distances);
  }

  if (!m_kdtree)
  {
    return false;
  }

  const std::size_t data_size = m_data->size();
  num_neighbors = std::min(num_neighbors, data_size);

  // Prepare query point
  value_type query_pt[3] = {query.x, query.y, query.z};

  // Search
  indices.resize(num_neighbors);
  distances.resize(num_neighbors);

  m_kdtree->knnSearch(
      &query_pt[0],
      num_neighbors,
      &indices[0],
      &distances[0]
  );

  // nanoflann returns squared distances for L2, so we need to take square root
  if (std::is_same_v<Metric, toolbox::metrics::L2Metric<value_type>>)
  {
    for (auto& dist : distances)
    {
      dist = std::sqrt(dist);
    }
  }

  return true;
}

template<typename Element, typename Metric>
bool kdtree_generic_t<Element, Metric>::radius_neighbors_impl(
    const element_type& query,
    distance_type radius,
    std::vector<std::size_t>& indices,
    std::vector<distance_type>& distances)
{
  if (!m_data || m_data->empty() || radius <= 0)
  {
    return false;
  }

  // If metric is not supported by KD-tree, fall back to brute-force
  if (!validate_metric())
  {
    bfknn_generic_t<Element, Metric> bfknn;
    bfknn.set_input(m_data);
    if (m_use_runtime_metric)
    {
      bfknn.set_metric(m_runtime_metric);
    }
    else
    {
      bfknn.set_metric(m_compile_time_metric);
    }
    return bfknn.radius_neighbors(query, radius, indices, distances);
  }

  if (!m_kdtree)
  {
    return false;
  }

  indices.clear();
  distances.clear();

  // Prepare query point
  value_type query_pt[3] = {query.x, query.y, query.z};

  // For L2 metric, nanoflann expects squared radius
  distance_type search_radius = radius;
  if (std::is_same_v<Metric, toolbox::metrics::L2Metric<value_type>>)
  {
    search_radius = radius * radius;
  }

  // Search
  std::vector<nanoflann::ResultItem<std::size_t, distance_type>> matches;
  const std::size_t num_matches = m_kdtree->radiusSearch(
      &query_pt[0],
      search_radius,
      matches,
      nanoflann::SearchParameters()
  );

  // Extract results
  indices.reserve(num_matches);
  distances.reserve(num_matches);

  for (const auto& match : matches)
  {
    indices.push_back(match.first);
    distance_type dist = match.second;
    
    // nanoflann returns squared distances for L2
    if (std::is_same_v<Metric, toolbox::metrics::L2Metric<value_type>>)
    {
      dist = std::sqrt(dist);
    }
    
    distances.push_back(dist);
  }

  return true;
}

// Legacy KD-tree implementation
template<typename DataType>
std::size_t kdtree_t<DataType>::set_input_impl(const point_cloud& cloud)
{
  // Convert point cloud to vector of points
  std::vector<point_t<DataType>> points(cloud.points.begin(), cloud.points.end());
  
  // Only use kdtree for supported metrics
  if (m_metric == metric_type_t::euclidean)
  {
    return m_impl.set_input(points);
  }
  else
  {
    // Use brute-force fallback for unsupported metrics
    if (!m_bfknn_fallback)
    {
      m_bfknn_fallback = std::make_unique<bfknn_t<DataType>>();
    }
    m_use_bfknn_fallback = true;
    return m_bfknn_fallback->set_input(cloud);
  }
}

template<typename DataType>
std::size_t kdtree_t<DataType>::set_input_impl(const point_cloud_ptr& cloud)
{
  if (!cloud) return 0;
  return set_input_impl(*cloud);
}

template<typename DataType>
std::size_t kdtree_t<DataType>::set_metric_impl(metric_type_t metric)
{
  m_metric = metric;
  
  // KD-tree only supports Euclidean distance efficiently
  if (metric == metric_type_t::euclidean)
  {
    m_impl.set_metric(toolbox::metrics::L2Metric<DataType>{});
    m_use_bfknn_fallback = false;
  }
  else
  {
    // Create fallback for unsupported metrics
    if (!m_bfknn_fallback)
    {
      m_bfknn_fallback = std::make_unique<bfknn_t<DataType>>();
    }
    m_bfknn_fallback->set_metric(metric);
    m_use_bfknn_fallback = true;
  }
  
  return 0;
}

template<typename DataType>
bool kdtree_t<DataType>::kneighbors_impl(
    const point_t<data_type>& query_point,
    std::size_t num_neighbors,
    std::vector<std::size_t>& indices,
    std::vector<data_type>& distances)
{
  if (m_use_bfknn_fallback && m_bfknn_fallback)
  {
    return m_bfknn_fallback->kneighbors(query_point, num_neighbors, indices, distances);
  }
  return m_impl.kneighbors(query_point, num_neighbors, indices, distances);
}

template<typename DataType>
bool kdtree_t<DataType>::radius_neighbors_impl(
    const point_t<data_type>& query_point,
    data_type radius,
    std::vector<std::size_t>& indices,
    std::vector<data_type>& distances)
{
  if (m_use_bfknn_fallback && m_bfknn_fallback)
  {
    return m_bfknn_fallback->radius_neighbors(query_point, radius, indices, distances);
  }
  return m_impl.radius_neighbors(query_point, radius, indices, distances);
}

}  // namespace toolbox::pcl