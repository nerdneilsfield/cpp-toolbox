#pragma once

#include <cpp-toolbox/pcl/descriptors/shot_extractor.hpp>
#include <numeric>
#include <cpp-toolbox/concurrent/parallel.hpp>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

namespace toolbox::pcl
{

template<typename DataType, typename KNN>
std::size_t shot_extractor_t<DataType, KNN>::set_input(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t shot_extractor_t<DataType, KNN>::set_input(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t shot_extractor_t<DataType, KNN>::set_knn(const knn_type& knn)
{
  m_knn = const_cast<knn_type*>(&knn);
  if (m_cloud && m_knn)
  {
    // Convert point cloud to vector of points for new KNN API
    std::vector<point_t<DataType>> points(m_cloud->points.begin(), m_cloud->points.end());
    m_knn->set_input(points);
  }
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t shot_extractor_t<DataType, KNN>::set_search_radius(data_type radius)
{
  m_search_radius = radius;
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t shot_extractor_t<DataType, KNN>::set_num_neighbors(std::size_t num_neighbors)
{
  m_num_neighbors = num_neighbors;
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
void shot_extractor_t<DataType, KNN>::set_normals(const point_cloud_ptr& normals)
{
  m_normals = normals;
}

template<typename DataType, typename KNN>
void shot_extractor_t<DataType, KNN>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

template<typename DataType, typename KNN>
void shot_extractor_t<DataType, KNN>::compute_impl(
    const point_cloud& cloud,
    const std::vector<std::size_t>& keypoint_indices,
    std::vector<signature_type>& descriptors) const
{
  if (!m_knn || keypoint_indices.empty())
  {
    descriptors.clear();
    return;
  }

  // Compute normals if not provided
  point_cloud_ptr normals = m_normals;
  if (!normals)
  {
    normals = std::make_shared<point_cloud>();
    normals->points.resize(cloud.size());
    pca_norm_extractor_t<data_type, knn_type> norm_extractor;
    norm_extractor.set_input(cloud);
    norm_extractor.set_knn(*m_knn);
    norm_extractor.set_num_neighbors(m_num_neighbors);
    norm_extractor.enable_parallel(m_enable_parallel);
    norm_extractor.extract(normals);
  }

  descriptors.resize(keypoint_indices.size());

  auto compute_shot_range = [&](std::size_t start, std::size_t end) {
    for (std::size_t i = start; i < end; ++i)
    {
      std::size_t keypoint_idx = keypoint_indices[i];
      
      // Get neighbors
      std::vector<std::size_t> neighbor_indices;
      std::vector<data_type> neighbor_distances;
      m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius, neighbor_indices, neighbor_distances);
      
      // Limit to m_num_neighbors
      if (neighbor_indices.size() > m_num_neighbors)
      {
        neighbor_indices.resize(m_num_neighbors);
        neighbor_distances.resize(m_num_neighbors);
      }
      
      if (neighbor_indices.size() < 5)  // Need enough neighbors for robust LRF
      {
        // Set empty descriptor
        std::fill(descriptors[i].histogram.begin(), descriptors[i].histogram.end(), DataType(0));
        continue;
      }

      // Compute local reference frame
      local_rf_t lrf;
      compute_local_reference_frame(cloud, *normals, keypoint_idx, neighbor_indices, lrf);

      // Compute SHOT descriptor
      compute_shot_feature(cloud, *normals, keypoint_idx, neighbor_indices, lrf, descriptors[i]);
    }
  };

  if (m_enable_parallel)
  {
    // Create index range for keypoints
    std::vector<std::size_t> indices(keypoint_indices.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    toolbox::concurrent::parallel_for_each(indices.begin(), indices.end(),
                                          [&](std::size_t i) {
      std::size_t keypoint_idx = keypoint_indices[i];
      
      // Get neighbors
      std::vector<std::size_t> neighbor_indices;
      std::vector<data_type> neighbor_distances;
      m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius, neighbor_indices, neighbor_distances);
      
      // Limit to m_num_neighbors
      if (neighbor_indices.size() > m_num_neighbors)
      {
        neighbor_indices.resize(m_num_neighbors);
        neighbor_distances.resize(m_num_neighbors);
      }
      
      if (neighbor_indices.size() < 5)  // Need enough neighbors for robust LRF
      {
        // Set empty descriptor
        std::fill(descriptors[i].histogram.begin(), descriptors[i].histogram.end(), DataType(0));
        return;
      }

      // Compute local reference frame
      local_rf_t lrf;
      compute_local_reference_frame(cloud, *normals, keypoint_idx, neighbor_indices, lrf);

      // Compute SHOT descriptor
      compute_shot_feature(cloud, *normals, keypoint_idx, neighbor_indices, lrf, descriptors[i]);
    });
  }
  else
  {
    compute_shot_range(0, keypoint_indices.size());
  }
}

template<typename DataType, typename KNN>
void shot_extractor_t<DataType, KNN>::compute_impl(
    const point_cloud& cloud,
    const std::vector<std::size_t>& keypoint_indices,
    std::unique_ptr<std::vector<signature_type>>& descriptors) const
{
  descriptors = std::make_unique<std::vector<signature_type>>();
  compute_impl(cloud, keypoint_indices, *descriptors);
}

template<typename DataType, typename KNN>
void shot_extractor_t<DataType, KNN>::compute_local_reference_frame(
    const point_cloud& cloud,
    const point_cloud& normals,
    std::size_t index,
    const std::vector<std::size_t>& neighbor_indices,
    local_rf_t& lrf) const
{
  const auto& center = cloud.points[index];
  const auto& normal = normals.points[index];

  // Compute weighted covariance matrix
  std::vector<data_type> weights(neighbor_indices.size());
  data_type total_weight = 0;
  
  for (std::size_t i = 0; i < neighbor_indices.size(); ++i)
  {
    point_t<data_type> diff;
    diff.x = cloud.points[neighbor_indices[i]].x - center.x;
    diff.y = cloud.points[neighbor_indices[i]].y - center.y;
    diff.z = cloud.points[neighbor_indices[i]].z - center.z;
    data_type dist = diff.norm();
    weights[i] = DataType(1) / (dist + DataType(1e-6));
    total_weight += weights[i];
  }

  // Normalize weights
  for (auto& w : weights)
  {
    w /= total_weight;
  }

  // Compute weighted covariance
  data_type cov[9];
  compute_weighted_covariance(cloud, index, neighbor_indices, weights, cov);

  // Eigen decomposition
  Eigen::Matrix<DataType, 3, 3> cov_matrix;
  cov_matrix << cov[0], cov[1], cov[2],
                cov[3], cov[4], cov[5],
                cov[6], cov[7], cov[8];

  Eigen::SelfAdjointEigenSolver<Eigen::Matrix<DataType, 3, 3>> solver(cov_matrix);
  auto eigenvectors = solver.eigenvectors();

  // Set z-axis as the normal
  lrf.z_axis = normal;

  // x-axis is the eigenvector with largest eigenvalue orthogonal to z
  lrf.x_axis = point_t<data_type>(eigenvectors(0, 2), eigenvectors(1, 2), eigenvectors(2, 2));
  
  // Make x-axis orthogonal to z-axis
  data_type dot_product = lrf.x_axis.dot(lrf.z_axis);
  lrf.x_axis.x = lrf.x_axis.x - lrf.z_axis.x * dot_product;
  lrf.x_axis.y = lrf.x_axis.y - lrf.z_axis.y * dot_product;
  lrf.x_axis.z = lrf.x_axis.z - lrf.z_axis.z * dot_product;
  auto x_normalized = lrf.x_axis.normalize();
  lrf.x_axis.x = static_cast<data_type>(x_normalized.x);
  lrf.x_axis.y = static_cast<data_type>(x_normalized.y);
  lrf.x_axis.z = static_cast<data_type>(x_normalized.z);

  // y-axis = z × x
  lrf.y_axis = lrf.z_axis.cross(lrf.x_axis);
  auto y_normalized = lrf.y_axis.normalize();
  lrf.y_axis.x = static_cast<data_type>(y_normalized.x);
  lrf.y_axis.y = static_cast<data_type>(y_normalized.y);
  lrf.y_axis.z = static_cast<data_type>(y_normalized.z);

  // Disambiguate the sign of axes
  data_type positive_count = 0;
  for (std::size_t neighbor_idx : neighbor_indices)
  {
    point_t<data_type> diff;
    diff.x = cloud.points[neighbor_idx].x - center.x;
    diff.y = cloud.points[neighbor_idx].y - center.y;
    diff.z = cloud.points[neighbor_idx].z - center.z;
    if (diff.dot(lrf.x_axis) > 0)
      positive_count++;
  }

  if (positive_count < neighbor_indices.size() / 2)
  {
    lrf.x_axis.x = -lrf.x_axis.x;
    lrf.x_axis.y = -lrf.x_axis.y;
    lrf.x_axis.z = -lrf.x_axis.z;
    lrf.y_axis.x = -lrf.y_axis.x;
    lrf.y_axis.y = -lrf.y_axis.y;
    lrf.y_axis.z = -lrf.y_axis.z;
  }
}

template<typename DataType, typename KNN>
void shot_extractor_t<DataType, KNN>::compute_shot_feature(
    const point_cloud& cloud,
    const point_cloud& normals,
    std::size_t index,
    const std::vector<std::size_t>& neighbor_indices,
    const local_rf_t& lrf,
    signature_type& shot) const
{
  // Initialize histogram
  std::fill(shot.histogram.begin(), shot.histogram.end(), DataType(0));

  const auto& center = cloud.points[index];
  constexpr std::size_t num_spatial_bins = 32;
  constexpr std::size_t num_angular_bins = 11;

  // Process each neighbor
  for (std::size_t neighbor_idx : neighbor_indices)
  {
    if (neighbor_idx == index) continue;

    const auto& neighbor_point = cloud.points[neighbor_idx];
    const auto& neighbor_normal = normals.points[neighbor_idx];

    // Compute spatial bin
    std::size_t spatial_bin = compute_spatial_bin(neighbor_point, center, lrf, m_search_radius);
    if (spatial_bin >= num_spatial_bins) continue;

    // Compute angular bin
    std::size_t angular_bin = compute_angular_bin(neighbor_normal, lrf);
    if (angular_bin >= num_angular_bins) continue;

    // Add to histogram
    std::size_t hist_idx = spatial_bin * num_angular_bins + angular_bin;
    point_t<data_type> diff;
    diff.x = neighbor_point.x - center.x;
    diff.y = neighbor_point.y - center.y;
    diff.z = neighbor_point.z - center.z;
    data_type distance = diff.norm();
    data_type weight = DataType(1) - (distance / m_search_radius);
    shot.histogram[hist_idx] += weight;
  }

  // Normalize histogram
  data_type norm = 0;
  for (std::size_t i = 0; i < signature_type::HISTOGRAM_SIZE; ++i)
  {
    norm += shot.histogram[i] * shot.histogram[i];
  }

  if (norm > DataType(1e-6))
  {
    norm = DataType(1) / std::sqrt(norm);
    for (std::size_t i = 0; i < signature_type::HISTOGRAM_SIZE; ++i)
    {
      shot.histogram[i] *= norm;
    }
  }
}

template<typename DataType, typename KNN>
void shot_extractor_t<DataType, KNN>::compute_weighted_covariance(
    const point_cloud& cloud,
    std::size_t center_idx,
    const std::vector<std::size_t>& indices,
    const std::vector<data_type>& weights,
    data_type cov[9]) const
{
  const auto& center = cloud.points[center_idx];

  // Initialize covariance matrix
  std::fill(cov, cov + 9, DataType(0));

  // Compute weighted mean
  point_t<data_type> mean(0, 0, 0);
  for (std::size_t i = 0; i < indices.size(); ++i)
  {
    point_t<data_type> diff;
    diff.x = cloud.points[indices[i]].x - center.x;
    diff.y = cloud.points[indices[i]].y - center.y;
    diff.z = cloud.points[indices[i]].z - center.z;
    mean.x += weights[i] * diff.x;
    mean.y += weights[i] * diff.y;
    mean.z += weights[i] * diff.z;
  }

  // Compute covariance
  for (std::size_t i = 0; i < indices.size(); ++i)
  {
    point_t<data_type> diff;
    diff.x = cloud.points[indices[i]].x - center.x - mean.x;
    diff.y = cloud.points[indices[i]].y - center.y - mean.y;
    diff.z = cloud.points[indices[i]].z - center.z - mean.z;
    data_type w = weights[i];

    cov[0] += w * diff.x * diff.x;
    cov[1] += w * diff.x * diff.y;
    cov[2] += w * diff.x * diff.z;
    cov[3] += w * diff.y * diff.x;
    cov[4] += w * diff.y * diff.y;
    cov[5] += w * diff.y * diff.z;
    cov[6] += w * diff.z * diff.x;
    cov[7] += w * diff.z * diff.y;
    cov[8] += w * diff.z * diff.z;
  }
}

template<typename DataType, typename KNN>
std::size_t shot_extractor_t<DataType, KNN>::compute_spatial_bin(
    const point_t<data_type>& point,
    const point_t<data_type>& center,
    const local_rf_t& lrf,
    data_type radius) const
{
  point_t<data_type> local_point;
  local_point.x = point.x - center.x;
  local_point.y = point.y - center.y;
  local_point.z = point.z - center.z;
  
  // Transform to local reference frame
  data_type x = local_point.dot(lrf.x_axis);
  data_type y = local_point.dot(lrf.y_axis);
  data_type z = local_point.dot(lrf.z_axis);

  // Compute spherical coordinates
  data_type r = std::sqrt(x*x + y*y + z*z);
  data_type theta = std::atan2(y, x);  // Azimuth angle
  data_type phi = std::acos(z / (r + DataType(1e-6)));  // Elevation angle

  // Normalize coordinates
  r = r / radius;
  theta = (theta + DataType(M_PI)) / (DataType(2) * DataType(M_PI));
  phi = phi / DataType(M_PI);

  // Determine spatial bin (2 radial × 4 azimuth × 4 elevation = 32 bins)
  std::size_t r_bin = (r < DataType(0.5)) ? 0 : 1;
  std::size_t theta_bin = static_cast<std::size_t>(theta * 4);
  std::size_t phi_bin = static_cast<std::size_t>(phi * 4);

  // Clamp bins
  theta_bin = std::min(theta_bin, std::size_t(3));
  phi_bin = std::min(phi_bin, std::size_t(3));

  return r_bin * 16 + theta_bin * 4 + phi_bin;
}

template<typename DataType, typename KNN>
std::size_t shot_extractor_t<DataType, KNN>::compute_angular_bin(
    const point_t<data_type>& normal,
    const local_rf_t& lrf) const
{
  // Project normal onto local reference frame
  data_type cos_angle = normal.dot(lrf.z_axis);
  
  // Clamp to [-1, 1]
  cos_angle = std::max(DataType(-1), std::min(DataType(1), cos_angle));
  
  // Convert to bin index (11 bins from -1 to 1)
  data_type normalized = (cos_angle + DataType(1)) / DataType(2);
  std::size_t bin = static_cast<std::size_t>(normalized * 11);
  
  return std::min(bin, std::size_t(10));
}

}  // namespace toolbox::pcl