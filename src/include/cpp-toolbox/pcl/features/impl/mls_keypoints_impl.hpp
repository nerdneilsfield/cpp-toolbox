#pragma once

#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <Eigen/Dense>
#include <Eigen/SVD>
#include <algorithm>
#include <cmath>
#include <future>
#include <vector>
#include <iostream>

namespace toolbox::pcl
{

template<typename DataType, typename KNN>
std::size_t mls_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t mls_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t mls_keypoint_extractor_t<DataType, KNN>::set_knn_impl(const knn_type& knn)
{
  m_knn = const_cast<knn_type*>(&knn);
  if (m_cloud) {
    m_knn->set_input(*m_cloud);
  }
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t mls_keypoint_extractor_t<DataType, KNN>::set_search_radius_impl(data_type radius)
{
  m_search_radius = radius;
  return 0;
}

template<typename DataType, typename KNN>
void mls_keypoint_extractor_t<DataType, KNN>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

template<typename DataType, typename KNN>
int mls_keypoint_extractor_t<DataType, KNN>::get_polynomial_coefficients_size() const
{
  switch (m_polynomial_order) {
    case polynomial_order_t::NONE:
      return 3;  // a0 + a1*x + a2*y (plane z = f(x,y))
    case polynomial_order_t::LINEAR:
      return 4;  // Above + a3*x*y
    case polynomial_order_t::QUADRATIC:
      return 6;  // Above + a4*x^2 + a5*y^2
    default:
      return 3;
  }
}

template<typename DataType, typename KNN>
void mls_keypoint_extractor_t<DataType, KNN>::compute_polynomial_coefficients(
    const std::vector<Eigen::Vector3f>& points,
    const std::vector<data_type>& weights,
    const Eigen::Vector3f& mean_point,
    Eigen::VectorXf& coefficients)
{
  const int n_coeffs = get_polynomial_coefficients_size();
  const int n_points = static_cast<int>(points.size());
  
  // Build the design matrix A and weight matrix W
  Eigen::MatrixXf A(n_points, n_coeffs);
  Eigen::VectorXf b(n_points);
  Eigen::VectorXf W(n_points);
  
  // Fill matrices
  for (int i = 0; i < n_points; ++i) {
    const Eigen::Vector3f p = points[i] - mean_point;
    const float x = p.x();
    const float y = p.y();
    const float z = p.z();
    
    // Basic terms (plane z = f(x,y))
    A(i, 0) = 1.0f;
    A(i, 1) = x;
    A(i, 2) = y;
    
    // First order terms
    if (m_polynomial_order >= polynomial_order_t::LINEAR && n_coeffs > 3) {
      A(i, 3) = x * y;
    }
    
    // Second order terms
    if (m_polynomial_order >= polynomial_order_t::QUADRATIC && n_coeffs > 4) {
      A(i, 4) = x * x;
      A(i, 5) = y * y;
    }
    
    b(i) = z;  // Fit to the actual z values
    W(i) = weights[i];
  }
  
  // Solve weighted least squares: (A^T * W * A) * coeffs = A^T * W * b
  Eigen::MatrixXf AtW = A.transpose() * W.asDiagonal();
  Eigen::MatrixXf AtWA = AtW * A;
  Eigen::VectorXf AtWb = AtW * b;
  
  // Use SVD for numerical stability
  Eigen::JacobiSVD<Eigen::MatrixXf> svd(AtWA, Eigen::ComputeFullU | Eigen::ComputeFullV);
  coefficients = svd.solve(AtWb);
}

template<typename DataType, typename KNN>
typename mls_keypoint_extractor_t<DataType, KNN>::data_type
mls_keypoint_extractor_t<DataType, KNN>::compute_surface_variation(
    const std::vector<Eigen::Vector3f>& points,
    const Eigen::Vector3f& mean_point,
    const Eigen::VectorXf& coefficients)
{
  data_type total_variation = 0;
  data_type total_weight = 0;
  
  const int n_coeffs = get_polynomial_coefficients_size();
  
  for (const auto& point : points) {
    const Eigen::Vector3f p = point - mean_point;
    const float x = p.x();
    const float y = p.y();
    const float z = p.z();
    
    // Evaluate polynomial at this point (only x and y terms, as we're fitting z = f(x,y))
    float poly_value = coefficients(0);
    poly_value += coefficients(1) * x + coefficients(2) * y;
    
    if (m_polynomial_order >= polynomial_order_t::LINEAR && n_coeffs > 3) {
      poly_value += coefficients(3) * x * y;
    }
    
    if (m_polynomial_order >= polynomial_order_t::QUADRATIC && n_coeffs > 4) {
      poly_value += coefficients(4) * x * x;
      poly_value += coefficients(5) * y * y;
    }
    
    // Compute residual (difference between actual z and predicted z)
    float residual = z - poly_value;
    
    // Accumulate squared residual
    total_variation += residual * residual;
    total_weight += 1.0;
  }
  
  return total_weight > 0 ? std::sqrt(total_variation / total_weight) : 0;
}

template<typename DataType, typename KNN>
typename mls_keypoint_extractor_t<DataType, KNN>::MLSResult
mls_keypoint_extractor_t<DataType, KNN>::compute_mls_surface(std::size_t point_idx)
{
  MLSResult result;
  
  if (!m_cloud || !m_knn || point_idx >= m_cloud->size()) {
    return result;
  }

  const auto& query_point = m_cloud->points[point_idx];
  std::vector<std::size_t> neighbor_indices;
  std::vector<data_type> neighbor_distances;
  
  // Find neighbors within search radius
  m_knn->radius_neighbors(query_point, m_search_radius, neighbor_indices, neighbor_distances);
  
  if (neighbor_indices.size() < m_min_neighbors) {
    return result;
  }
  
  // Convert points to Eigen format and compute mean
  std::vector<Eigen::Vector3f> eigen_points;
  eigen_points.reserve(neighbor_indices.size());
  Eigen::Vector3f mean_point = Eigen::Vector3f::Zero();
  
  for (const auto& idx : neighbor_indices) {
    const auto& pt = m_cloud->points[idx];
    Eigen::Vector3f eigen_pt(pt.x, pt.y, pt.z);
    eigen_points.push_back(eigen_pt);
    mean_point += eigen_pt;
  }
  mean_point /= static_cast<float>(neighbor_indices.size());
  
  // Compute weights using Gaussian kernel
  std::vector<data_type> weights;
  weights.reserve(neighbor_indices.size());
  
  const data_type sqr_gauss = (m_sqr_gauss_param > 0) ? m_sqr_gauss_param : (m_search_radius * m_search_radius);
  
  for (std::size_t i = 0; i < neighbor_distances.size(); ++i) {
    const data_type sqr_dist = neighbor_distances[i] * neighbor_distances[i];
    const data_type weight = std::exp(-sqr_dist / sqr_gauss);
    weights.push_back(weight);
  }
  
  // Check if we have a normal for the query point
  Eigen::Vector3f normal(0, 0, 1);  // Default to Z-up
  if (point_idx < m_cloud->normals.size()) {
    const auto& n = m_cloud->normals[point_idx];
    normal = Eigen::Vector3f(n.x, n.y, n.z);
    if (normal.norm() > 0.1f) {
      normal.normalize();
    } else {
      // Estimate normal using PCA if not provided or invalid
      Eigen::Matrix3f covariance = Eigen::Matrix3f::Zero();
      for (const auto& pt : eigen_points) {
        Eigen::Vector3f diff = pt - mean_point;
        covariance += diff * diff.transpose();
      }
      covariance /= static_cast<float>(eigen_points.size());
      
      Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> eigen_solver(covariance);
      normal = eigen_solver.eigenvectors().col(0);  // Smallest eigenvalue
    }
  }
  
  // Project points to local coordinate system
  // Create local coordinate frame with normal as Z axis
  Eigen::Vector3f v1, v2;
  if (std::abs(normal.dot(Eigen::Vector3f::UnitX())) < 0.9f) {
    v1 = normal.cross(Eigen::Vector3f::UnitX()).normalized();
  } else {
    v1 = normal.cross(Eigen::Vector3f::UnitY()).normalized();
  }
  v2 = normal.cross(v1).normalized();
  
  // Transform points to local coordinates
  std::vector<Eigen::Vector3f> local_points;
  local_points.reserve(eigen_points.size());
  
  for (const auto& pt : eigen_points) {
    Eigen::Vector3f diff = pt - mean_point;
    Eigen::Vector3f local_pt;
    local_pt.x() = diff.dot(v1);
    local_pt.y() = diff.dot(v2);
    local_pt.z() = diff.dot(normal);
    local_points.push_back(local_pt);
  }
  
  // Fit polynomial
  Eigen::VectorXf coefficients;
  compute_polynomial_coefficients(local_points, weights, Eigen::Vector3f::Zero(), coefficients);
  
  // Compute surface variation
  result.variation = compute_surface_variation(local_points, Eigen::Vector3f::Zero(), coefficients);
  
  // Compute curvature if requested
  if (m_compute_curvatures && m_polynomial_order >= polynomial_order_t::QUADRATIC) {
    // Mean curvature from second order coefficients
    // H = (fxx + fyy) / 2 for z = f(x,y)
    if (coefficients.size() >= 6) {
      const float fxx = 2.0f * coefficients(4);
      const float fyy = 2.0f * coefficients(5);
      result.curvature = std::abs((fxx + fyy) / 2.0f);
    }
  }
  
  // Store refined normal (gradient of the polynomial)
  if (coefficients.size() >= 3) {
    Eigen::Vector3f grad_local(coefficients(1), coefficients(2), -1.0f);
    grad_local.normalize();
    
    // Transform back to global coordinates
    result.normal = v1 * grad_local.x() + v2 * grad_local.y() + normal * grad_local.z();
    result.normal.normalize();
  } else {
    result.normal = normal;
  }
  
  result.valid = true;
  return result;
}

template<typename DataType, typename KNN>
void mls_keypoint_extractor_t<DataType, KNN>::compute_mls_range(
    std::vector<MLSResult>& mls_results,
    std::size_t start_idx, 
    std::size_t end_idx)
{
  for (std::size_t i = start_idx; i < end_idx; ++i) {
    mls_results[i] = compute_mls_surface(i);
  }
}

template<typename DataType, typename KNN>
std::vector<typename mls_keypoint_extractor_t<DataType, KNN>::MLSResult>
mls_keypoint_extractor_t<DataType, KNN>::compute_all_mls_surfaces()
{
  if (!m_cloud) {
    return {};
  }

  const std::size_t num_points = m_cloud->size();
  std::vector<MLSResult> mls_results(num_points);

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
            [this, &mls_results, start_idx, end_idx]() {
              compute_mls_range(mls_results, start_idx, end_idx);
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
    compute_mls_range(mls_results, 0, num_points);
  }

  return mls_results;
}

template<typename DataType, typename KNN>
typename mls_keypoint_extractor_t<DataType, KNN>::indices_vector
mls_keypoint_extractor_t<DataType, KNN>::apply_non_maxima_suppression(
    const std::vector<MLSResult>& mls_results)
{
  if (!m_cloud || mls_results.empty()) {
    return {};
  }

  indices_vector keypoints;
  const std::size_t num_points = m_cloud->size();

  // Debug: Count valid results
  std::size_t valid_count = 0;
  data_type max_variation = 0;
  data_type max_curvature = 0;
  
  for (const auto& result : mls_results) {
    if (result.valid) {
      valid_count++;
      max_variation = std::max(max_variation, result.variation);
      max_curvature = std::max(max_curvature, result.curvature);
    }
  }
  
  // Debug output (uncomment for debugging)
  // std::cout << "MLS Debug: valid=" << valid_count << "/" << num_points 
  //           << " max_var=" << max_variation << " max_curv=" << max_curvature
  //           << " thresh_var=" << m_variation_threshold << " thresh_curv=" << m_curvature_threshold << std::endl;

  for (std::size_t i = 0; i < num_points; ++i) {
    const auto& current_result = mls_results[i];
    
    // Skip invalid points
    if (!current_result.valid) {
      continue;
    }
    
    // Check thresholds
    bool is_keypoint = false;
    
    if (current_result.variation > m_variation_threshold) {
      is_keypoint = true;
    }
    
    if (m_compute_curvatures && current_result.curvature > m_curvature_threshold) {
      is_keypoint = true;
    }
    
    if (!is_keypoint) {
      continue;
    }

    // Find neighbors within non-maxima suppression radius
    const auto& query_point = m_cloud->points[i];
    std::vector<std::size_t> neighbor_indices;
    std::vector<data_type> neighbor_distances;
    
    m_knn->radius_neighbors(query_point, m_non_maxima_radius, neighbor_indices, neighbor_distances);

    // Check if current point is local maximum
    bool is_local_maximum = true;
    
    // Use combined score (variation + curvature)
    const data_type current_score = current_result.variation + 
                                   (m_compute_curvatures ? current_result.curvature : 0);
    
    for (const auto& neighbor_idx : neighbor_indices) {
      if (neighbor_idx != i && neighbor_idx < mls_results.size()) {
        const auto& neighbor_result = mls_results[neighbor_idx];
        if (neighbor_result.valid) {
          const data_type neighbor_score = neighbor_result.variation + 
                                         (m_compute_curvatures ? neighbor_result.curvature : 0);
          if (neighbor_score > current_score) {
            is_local_maximum = false;
            break;
          }
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
typename mls_keypoint_extractor_t<DataType, KNN>::indices_vector
mls_keypoint_extractor_t<DataType, KNN>::extract_impl()
{
  if (!m_cloud || !m_knn) {
    return {};
  }

  // Compute MLS surfaces for all points
  auto mls_results = compute_all_mls_surfaces();
  
  // Apply non-maxima suppression to find keypoints
  return apply_non_maxima_suppression(mls_results);
}

template<typename DataType, typename KNN>
void mls_keypoint_extractor_t<DataType, KNN>::extract_impl(indices_vector& keypoint_indices)
{
  keypoint_indices = extract_impl();
}

template<typename DataType, typename KNN>
typename mls_keypoint_extractor_t<DataType, KNN>::point_cloud
mls_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl()
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
void mls_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl(point_cloud_ptr output)
{
  auto keypoint_indices = extract_impl();
  
  output->points.clear();
  output->points.reserve(keypoint_indices.size());
  
  for (const auto& idx : keypoint_indices) {
    output->points.push_back(m_cloud->points[idx]);
  }
}

}  // namespace toolbox::pcl