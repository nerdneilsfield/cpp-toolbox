#pragma once

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <future>
#include <memory>

namespace toolbox::pcl
{

template<typename DataType, typename KNN>
std::size_t pca_norm_extractor_t<DataType, KNN>::set_input_impl(
    const point_cloud& cloud)
{
  auto cloud_ptr = std::make_shared<point_cloud>(cloud);
  return set_input_impl(cloud_ptr);
}

template<typename DataType, typename KNN>
std::size_t pca_norm_extractor_t<DataType, KNN>::set_input_impl(
    const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  if (m_knn) {
    m_knn->set_input(cloud);
  }
  return cloud->size();
}

template<typename DataType, typename KNN>
std::size_t pca_norm_extractor_t<DataType, KNN>::set_knn_impl(
    const knn_type& knn)
{
  m_knn = const_cast<knn_type*>(&knn);
  if (m_cloud) {
    m_knn->set_input(m_cloud);
  }
  return 0;
}

template<typename DataType, typename KNN>
std::size_t pca_norm_extractor_t<DataType, KNN>::set_num_neighbors_impl(
    std::size_t num_neighbors)
{
  m_num_neighbors = num_neighbors;
  return m_num_neighbors;
}

template<typename DataType, typename KNN>
typename pca_norm_extractor_t<DataType, KNN>::point_cloud
pca_norm_extractor_t<DataType, KNN>::extract_impl()
{
  auto output = std::make_shared<point_cloud>();
  extract_impl(output);
  return *output;
}

template<typename DataType, typename KNN>
void pca_norm_extractor_t<DataType, KNN>::extract_impl(point_cloud_ptr output)
{
  if (!m_cloud || !m_knn || m_num_neighbors == 0) {
    return;
  }

  const std::size_t num_points = m_cloud->size();
  output->points.clear();
  output->normals.clear();
  output->normals.resize(num_points);

  if (m_enable_parallel) {
    // Parallel processing
    auto& thread_pool = toolbox::base::thread_pool_singleton_t::instance();
    const std::size_t num_threads = thread_pool.get_thread_count();
    const std::size_t chunk_size = (num_points + num_threads - 1) / num_threads;

    std::vector<std::future<void>> futures;
    futures.reserve(num_threads);

    for (std::size_t t = 0; t < num_threads; ++t) {
      const std::size_t start_idx = t * chunk_size;
      const std::size_t end_idx = std::min(start_idx + chunk_size, num_points);

      if (start_idx >= end_idx) break;

      auto future = thread_pool.submit([this, output, start_idx, end_idx]() {
        this->compute_normals_range(output, start_idx, end_idx);
      });

      futures.push_back(std::move(future));
    }

    // Wait for all tasks to complete
    for (auto& future : futures) {
      future.get();
    }
  } else {
    // Sequential processing
    compute_normals_range(output, 0, num_points);
  }

  // Copy points from input cloud
  output->points = m_cloud->points;
}

template<typename DataType, typename KNN>
void pca_norm_extractor_t<DataType, KNN>::compute_normals_range(
    point_cloud_ptr output, std::size_t start_idx, std::size_t end_idx)
{
  std::vector<std::size_t> indices;
  std::vector<data_type> distances;

  for (std::size_t i = start_idx; i < end_idx; ++i) {
    const auto& query_point = m_cloud->points[i];
    
    // Find k-nearest neighbors
    indices.clear();
    distances.clear();
    
    if (!m_knn->kneighbors(query_point, m_num_neighbors, indices, distances)) {
      // If KNN search fails, use default normal (0, 0, 1)
      output->normals[i] = point_t<data_type>(0, 0, 1);
      continue;
    }

    // Compute normal using PCA
    output->normals[i] = compute_pca_normal(indices);
  }
}

template<typename DataType, typename KNN>
point_t<typename pca_norm_extractor_t<DataType, KNN>::data_type>
pca_norm_extractor_t<DataType, KNN>::compute_pca_normal(
    const std::vector<std::size_t>& indices)
{
  if (indices.size() < 3) {
    // Need at least 3 points for PCA
    return point_t<data_type>(0, 0, 1);
  }

  // Compute centroid
  Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
  for (const auto& idx : indices) {
    const auto& pt = m_cloud->points[idx];
    centroid += Eigen::Vector3d(static_cast<double>(pt.x), 
                               static_cast<double>(pt.y), 
                               static_cast<double>(pt.z));
  }
  centroid /= static_cast<double>(indices.size());

  // Compute covariance matrix
  Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
  for (const auto& idx : indices) {
    const auto& pt = m_cloud->points[idx];
    Eigen::Vector3d point_vec(static_cast<double>(pt.x), 
                             static_cast<double>(pt.y), 
                             static_cast<double>(pt.z));
    Eigen::Vector3d centered = point_vec - centroid;
    covariance += centered * centered.transpose();
  }
  covariance /= static_cast<double>(indices.size() - 1);

  // Compute eigenvalues and eigenvectors
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eigen_solver(covariance);
  if (eigen_solver.info() != Eigen::Success) {
    // If eigenvalue computation fails, use default normal
    return point_t<data_type>(0, 0, 1);
  }

  // The normal is the eigenvector corresponding to the smallest eigenvalue
  const Eigen::Vector3d& normal_vec = eigen_solver.eigenvectors().col(0);

  // Convert back to data_type and create point_t
  return point_t<data_type>(static_cast<data_type>(normal_vec.x()),
                           static_cast<data_type>(normal_vec.y()),
                           static_cast<data_type>(normal_vec.z()));
}

}  // namespace toolbox::pcl