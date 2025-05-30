#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>

namespace toolbox::pcl
{

template<typename DataType>
std::size_t bfknn_t<DataType>::set_input_impl(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType>
std::size_t bfknn_t<DataType>::set_input_impl(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType>
std::size_t bfknn_t<DataType>::set_metric_impl(metric_type_t metric)
{
  m_metric = metric;
  return 0;
}

template<typename DataType>
DataType bfknn_t<DataType>::compute_distance(
    const point_t<data_type>& p1,
    const point_t<data_type>& p2,
    metric_type_t metric) const
{
  switch (metric)
  {
    case metric_type_t::euclidean:
    {
      data_type dx = p1.x - p2.x;
      data_type dy = p1.y - p2.y;
      data_type dz = p1.z - p2.z;
      return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
    case metric_type_t::manhattan:
    {
      return std::abs(p1.x - p2.x) + std::abs(p1.y - p2.y) + std::abs(p1.z - p2.z);
    }
    case metric_type_t::chebyshev:
    {
      return std::max({std::abs(p1.x - p2.x), std::abs(p1.y - p2.y), std::abs(p1.z - p2.z)});
    }
    case metric_type_t::minkowski:
    {
      data_type p = 3.0;  // Default p value for Minkowski
      data_type sum = std::pow(std::abs(p1.x - p2.x), p) +
                      std::pow(std::abs(p1.y - p2.y), p) +
                      std::pow(std::abs(p1.z - p2.z), p);
      return std::pow(sum, 1.0 / p);
    }
    default:
      return compute_distance(p1, p2, metric_type_t::euclidean);
  }
}

template<typename DataType>
bool bfknn_t<DataType>::kneighbors_impl(
    const point_t<data_type>& query_point,
    std::size_t num_neighbors,
    std::vector<std::size_t>& indices,
    std::vector<data_type>& distances)
{
  if (!m_cloud || m_cloud->empty())
  {
    return false;
  }

  const std::size_t cloud_size = m_cloud->size();
  num_neighbors = std::min(num_neighbors, cloud_size);

  // Compute all distances
  std::vector<std::pair<data_type, std::size_t>> distance_index_pairs;
  distance_index_pairs.reserve(cloud_size);

  for (std::size_t i = 0; i < cloud_size; ++i)
  {
    data_type dist = compute_distance(query_point, m_cloud->points[i], m_metric);
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

template<typename DataType>
bool bfknn_t<DataType>::radius_neighbors_impl(
    const point_t<data_type>& query_point,
    data_type radius,
    std::vector<std::size_t>& indices,
    std::vector<data_type>& distances)
{
  if (!m_cloud || m_cloud->empty() || radius <= 0)
  {
    return false;
  }

  indices.clear();
  distances.clear();

  const std::size_t cloud_size = m_cloud->size();
  std::vector<std::pair<data_type, std::size_t>> distance_index_pairs;

  for (std::size_t i = 0; i < cloud_size; ++i)
  {
    data_type dist = compute_distance(query_point, m_cloud->points[i], m_metric);
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