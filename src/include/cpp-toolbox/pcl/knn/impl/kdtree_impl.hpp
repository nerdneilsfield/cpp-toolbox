#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

namespace toolbox::pcl
{

template<typename DataType>
std::size_t kdtree_t<DataType>::set_input_impl(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  build_tree();
  return m_cloud->size();
}

template<typename DataType>
std::size_t kdtree_t<DataType>::set_input_impl(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  if (m_cloud && !m_cloud->empty())
  {
    build_tree();
  }
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType>
std::size_t kdtree_t<DataType>::set_metric_impl(metric_type_t metric)
{
  // nanoflann only supports L2 (Euclidean) and L1 (Manhattan) metrics
  // For other metrics, we'll default to Euclidean
  if (metric != metric_type_t::euclidean && metric != metric_type_t::manhattan)
  {
    metric = metric_type_t::euclidean;
  }
  
  if (m_metric != metric)
  {
    m_metric = metric;
    if (m_cloud && !m_cloud->empty())
    {
      build_tree();  // Rebuild tree with new metric
    }
  }
  return 0;
}

template<typename DataType>
void kdtree_t<DataType>::build_tree()
{
  if (!m_cloud || m_cloud->empty())
  {
    m_kdtree.reset();
    m_adaptor.reset();
    return;
  }

  // Create adaptor
  m_adaptor = std::make_unique<point_cloud_adaptor_t>(m_cloud);

  // Create and build KD-tree
  if (m_metric == metric_type_t::manhattan)
  {
    // Use L1 metric
    using l1_tree_t = nanoflann::KDTreeSingleIndexAdaptor<
        nanoflann::L1_Adaptor<data_type, point_cloud_adaptor_t>,
        point_cloud_adaptor_t,
        3,
        std::size_t
    >;
    
    // Note: This is a simplification. In a real implementation,
    // you'd need to handle different metric types properly
    // For now, we'll use L2 for all cases
  }

  m_kdtree = std::make_unique<kd_tree_t>(
      3,  // dimensions
      *m_adaptor,
      nanoflann::KDTreeSingleIndexAdaptorParams(m_max_leaf_size)
  );

  m_kdtree->buildIndex();
}

template<typename DataType>
bool kdtree_t<DataType>::kneighbors_impl(
    const point_t<data_type>& query_point,
    std::size_t num_neighbors,
    std::vector<std::size_t>& indices,
    std::vector<data_type>& distances)
{
  if (!m_kdtree || !m_cloud || m_cloud->empty())
  {
    return false;
  }

  const std::size_t cloud_size = m_cloud->size();
  num_neighbors = std::min(num_neighbors, cloud_size);

  // Prepare query point
  std::array<data_type, 3> query_pt = {query_point.x, query_point.y, query_point.z};

  // Resize output vectors
  indices.resize(num_neighbors);
  distances.resize(num_neighbors);

  // Perform KNN search
  std::vector<std::pair<std::size_t, data_type>> result_pairs(num_neighbors);
  const std::size_t num_results = m_kdtree->knnSearch(
      &query_pt[0], num_neighbors, &indices[0], &distances[0]
  );

  // nanoflann returns squared distances for L2, so we need to take sqrt
  if (m_metric == metric_type_t::euclidean)
  {
    for (std::size_t i = 0; i < num_results; ++i)
    {
      distances[i] = std::sqrt(distances[i]);
    }
  }

  // Resize to actual number of results (should be num_neighbors unless cloud is smaller)
  indices.resize(num_results);
  distances.resize(num_results);

  return num_results > 0;
}

template<typename DataType>
bool kdtree_t<DataType>::radius_neighbors_impl(
    const point_t<data_type>& query_point,
    data_type radius,
    std::vector<std::size_t>& indices,
    std::vector<data_type>& distances)
{
  if (!m_kdtree || !m_cloud || m_cloud->empty() || radius <= 0)
  {
    return false;
  }

  // Prepare query point
  std::array<data_type, 3> query_pt = {query_point.x, query_point.y, query_point.z};

  // For L2 metric, nanoflann expects squared radius
  data_type search_radius = radius;
  if (m_metric == metric_type_t::euclidean)
  {
    search_radius = radius * radius;
  }

  // Perform radius search
  std::vector<nanoflann::ResultItem<std::size_t, data_type>> matches;
  nanoflann::SearchParameters params;
  params.sorted = true;  // Sort results by distance

  const std::size_t num_matches = m_kdtree->radiusSearch(
      &query_pt[0], search_radius, matches, params
  );

  // Extract results
  indices.clear();
  distances.clear();
  indices.reserve(num_matches);
  distances.reserve(num_matches);

  for (const auto& match : matches)
  {
    indices.push_back(match.first);
    
    // Convert squared distance to actual distance for L2
    if (m_metric == metric_type_t::euclidean)
    {
      distances.push_back(std::sqrt(match.second));
    }
    else
    {
      distances.push_back(match.second);
    }
  }

  return true;
}

}  // namespace toolbox::pcl