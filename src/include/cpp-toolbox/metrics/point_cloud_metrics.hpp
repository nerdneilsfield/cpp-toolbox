#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <unordered_map>
#include <vector>

#include <Eigen/Core>

#include <cpp-toolbox/metrics/base_metric.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::metrics
{

/**
 * @brief Hausdorff distance between two point clouds
 * 
 * The Hausdorff distance is the maximum distance from a point in one cloud
 * to the nearest point in the other cloud, considering both directions.
 */
template<typename T>
class HausdorffMetric : public base_metric_t<HausdorffMetric<T>, T>
{
public:
  using element_type = T;
  using point_type = toolbox::types::point_t<T>;
  using cloud_type = toolbox::types::point_cloud_t<T>;

  // For compatibility with base_metric_t, we use pointers to clouds
  constexpr T distance_impl(const T* cloud_a, const T* cloud_b, std::size_t size) const
  {
    // This overload is not used for point clouds
    throw std::runtime_error("HausdorffMetric requires point cloud objects, not raw arrays");
  }

  T distance(const cloud_type& cloud_a, const cloud_type& cloud_b) const
  {
    if (cloud_a.empty() || cloud_b.empty()) {
      return std::numeric_limits<T>::infinity();
    }

    // Compute directed Hausdorff distance from A to B
    T max_dist_a_to_b = 0;
    for (const auto& point_a : cloud_a.points) {
      T min_dist = std::numeric_limits<T>::max();
      for (const auto& point_b : cloud_b.points) {
        T dist = point_a.distance(point_b);
        min_dist = std::min(min_dist, dist);
      }
      max_dist_a_to_b = std::max(max_dist_a_to_b, min_dist);
    }

    // Compute directed Hausdorff distance from B to A
    T max_dist_b_to_a = 0;
    for (const auto& point_b : cloud_b.points) {
      T min_dist = std::numeric_limits<T>::max();
      for (const auto& point_a : cloud_a.points) {
        T dist = point_b.distance(point_a);
        min_dist = std::min(min_dist, dist);
      }
      max_dist_b_to_a = std::max(max_dist_b_to_a, min_dist);
    }

    // Return the maximum of both directions
    return std::max(max_dist_a_to_b, max_dist_b_to_a);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

/**
 * @brief Modified Hausdorff distance (average of k-th smallest distances)
 * 
 * More robust to outliers than standard Hausdorff distance
 */
template<typename T>
class ModifiedHausdorffMetric : public base_metric_t<ModifiedHausdorffMetric<T>, T>
{
public:
  using element_type = T;
  using point_type = toolbox::types::point_t<T>;
  using cloud_type = toolbox::types::point_cloud_t<T>;

  explicit ModifiedHausdorffMetric(std::size_t k = 1) : k_(k) {}

  constexpr T distance_impl(const T* cloud_a, const T* cloud_b, std::size_t size) const
  {
    throw std::runtime_error("ModifiedHausdorffMetric requires point cloud objects");
  }

  T distance(const cloud_type& cloud_a, const cloud_type& cloud_b) const
  {
    if (cloud_a.empty() || cloud_b.empty()) {
      return std::numeric_limits<T>::infinity();
    }

    // Ensure k is valid
    std::size_t k_a = std::min(k_, cloud_a.size());
    std::size_t k_b = std::min(k_, cloud_b.size());

    // Compute distances from each point in A to all points in B
    std::vector<T> min_dists_a;
    min_dists_a.reserve(cloud_a.size());
    
    for (const auto& point_a : cloud_a.points) {
      T min_dist = std::numeric_limits<T>::max();
      for (const auto& point_b : cloud_b.points) {
        T dist = point_a.distance(point_b);
        min_dist = std::min(min_dist, dist);
      }
      min_dists_a.push_back(min_dist);
    }

    // Sort and take average of k smallest
    std::partial_sort(min_dists_a.begin(), min_dists_a.begin() + k_a, min_dists_a.end());
    T avg_a_to_b = std::accumulate(min_dists_a.begin(), min_dists_a.begin() + k_a, T(0)) / k_a;

    // Compute distances from each point in B to all points in A
    std::vector<T> min_dists_b;
    min_dists_b.reserve(cloud_b.size());
    
    for (const auto& point_b : cloud_b.points) {
      T min_dist = std::numeric_limits<T>::max();
      for (const auto& point_a : cloud_a.points) {
        T dist = point_b.distance(point_a);
        min_dist = std::min(min_dist, dist);
      }
      min_dists_b.push_back(min_dist);
    }

    // Sort and take average of k smallest
    std::partial_sort(min_dists_b.begin(), min_dists_b.begin() + k_b, min_dists_b.end());
    T avg_b_to_a = std::accumulate(min_dists_b.begin(), min_dists_b.begin() + k_b, T(0)) / k_b;

    return std::max(avg_a_to_b, avg_b_to_a);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }

private:
  std::size_t k_;
};

/**
 * @brief Chamfer distance between two point clouds
 * 
 * Average of nearest neighbor distances in both directions
 */
template<typename T>
class ChamferMetric : public base_metric_t<ChamferMetric<T>, T>
{
public:
  using element_type = T;
  using point_type = toolbox::types::point_t<T>;
  using cloud_type = toolbox::types::point_cloud_t<T>;

  constexpr T distance_impl(const T* cloud_a, const T* cloud_b, std::size_t size) const
  {
    throw std::runtime_error("ChamferMetric requires point cloud objects");
  }

  T distance(const cloud_type& cloud_a, const cloud_type& cloud_b) const
  {
    if (cloud_a.empty() || cloud_b.empty()) {
      return std::numeric_limits<T>::infinity();
    }

    // Compute average distance from A to B
    T sum_a_to_b = 0;
    for (const auto& point_a : cloud_a.points) {
      T min_dist = std::numeric_limits<T>::max();
      for (const auto& point_b : cloud_b.points) {
        T dist = point_a.distance(point_b);
        min_dist = std::min(min_dist, dist);
      }
      sum_a_to_b += min_dist;
    }
    T avg_a_to_b = sum_a_to_b / cloud_a.size();

    // Compute average distance from B to A
    T sum_b_to_a = 0;
    for (const auto& point_b : cloud_b.points) {
      T min_dist = std::numeric_limits<T>::max();
      for (const auto& point_a : cloud_a.points) {
        T dist = point_b.distance(point_a);
        min_dist = std::min(min_dist, dist);
      }
      sum_b_to_a += min_dist;
    }
    T avg_b_to_a = sum_b_to_a / cloud_b.size();

    // Return average of both directions
    return (avg_a_to_b + avg_b_to_a) / 2;
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

/**
 * @brief Earth Mover's Distance (EMD) for point clouds
 * 
 * Simplified version that assumes equal weight for each point
 */
template<typename T>
class PointCloudEMDMetric : public base_metric_t<PointCloudEMDMetric<T>, T>
{
public:
  using element_type = T;
  using point_type = toolbox::types::point_t<T>;
  using cloud_type = toolbox::types::point_cloud_t<T>;

  constexpr T distance_impl(const T* cloud_a, const T* cloud_b, std::size_t size) const
  {
    throw std::runtime_error("PointCloudEMDMetric requires point cloud objects");
  }

  T distance(const cloud_type& cloud_a, const cloud_type& cloud_b) const
  {
    if (cloud_a.empty() || cloud_b.empty()) {
      return std::numeric_limits<T>::infinity();
    }

    // For equal-sized clouds, compute minimum matching cost
    if (cloud_a.size() == cloud_b.size()) {
      // Simple greedy matching (not optimal but fast)
      std::vector<bool> matched_b(cloud_b.size(), false);
      T total_cost = 0;

      for (const auto& point_a : cloud_a.points) {
        T min_dist = std::numeric_limits<T>::max();
        std::size_t best_match = 0;
        
        for (std::size_t j = 0; j < cloud_b.size(); ++j) {
          if (!matched_b[j]) {
            T dist = point_a.distance(cloud_b.points[j]);
            if (dist < min_dist) {
              min_dist = dist;
              best_match = j;
            }
          }
        }
        
        matched_b[best_match] = true;
        total_cost += min_dist;
      }

      return total_cost / cloud_a.size();
    }
    else {
      // For different-sized clouds, use Chamfer distance as approximation
      ChamferMetric<T> chamfer;
      return chamfer.distance(cloud_a, cloud_b);
    }
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

/**
 * @brief Centroid distance between two point clouds
 * 
 * Distance between the centroids (centers of mass) of two point clouds
 */
template<typename T>
class CentroidMetric : public base_metric_t<CentroidMetric<T>, T>
{
public:
  using element_type = T;
  using point_type = toolbox::types::point_t<T>;
  using cloud_type = toolbox::types::point_cloud_t<T>;

  constexpr T distance_impl(const T* cloud_a, const T* cloud_b, std::size_t size) const
  {
    throw std::runtime_error("CentroidMetric requires point cloud objects");
  }

  T distance(const cloud_type& cloud_a, const cloud_type& cloud_b) const
  {
    if (cloud_a.empty() || cloud_b.empty()) {
      return std::numeric_limits<T>::infinity();
    }

    // Compute centroid of cloud A
    point_type centroid_a(0, 0, 0);
    for (const auto& point : cloud_a.points) {
      centroid_a.x += point.x;
      centroid_a.y += point.y;
      centroid_a.z += point.z;
    }
    centroid_a.x /= cloud_a.size();
    centroid_a.y /= cloud_a.size();
    centroid_a.z /= cloud_a.size();

    // Compute centroid of cloud B
    point_type centroid_b(0, 0, 0);
    for (const auto& point : cloud_b.points) {
      centroid_b.x += point.x;
      centroid_b.y += point.y;
      centroid_b.z += point.z;
    }
    centroid_b.x /= cloud_b.size();
    centroid_b.y /= cloud_b.size();
    centroid_b.z /= cloud_b.size();

    return centroid_a.distance(centroid_b);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

/**
 * @brief Bounding box distance between two point clouds
 * 
 * Distance based on the bounding boxes of the point clouds
 */
template<typename T>
class BoundingBoxMetric : public base_metric_t<BoundingBoxMetric<T>, T>
{
public:
  using element_type = T;
  using point_type = toolbox::types::point_t<T>;
  using cloud_type = toolbox::types::point_cloud_t<T>;

  enum class Mode {
    CENTER_DISTANCE,    // Distance between bounding box centers
    MIN_DISTANCE,       // Minimum distance between bounding boxes
    IOU_DISTANCE        // 1 - IoU (Intersection over Union)
  };

  explicit BoundingBoxMetric(Mode mode = Mode::CENTER_DISTANCE) : mode_(mode) {}

  constexpr T distance_impl(const T* cloud_a, const T* cloud_b, std::size_t size) const
  {
    throw std::runtime_error("BoundingBoxMetric requires point cloud objects");
  }

  T distance(const cloud_type& cloud_a, const cloud_type& cloud_b) const
  {
    if (cloud_a.empty() || cloud_b.empty()) {
      return std::numeric_limits<T>::infinity();
    }

    // Compute bounding box for cloud A
    auto [min_a, max_a] = compute_bounding_box(cloud_a);
    
    // Compute bounding box for cloud B
    auto [min_b, max_b] = compute_bounding_box(cloud_b);

    switch (mode_) {
      case Mode::CENTER_DISTANCE: {
        point_type center_a((min_a.x + max_a.x) / 2, 
                           (min_a.y + max_a.y) / 2,
                           (min_a.z + max_a.z) / 2);
        point_type center_b((min_b.x + max_b.x) / 2,
                           (min_b.y + max_b.y) / 2,
                           (min_b.z + max_b.z) / 2);
        return center_a.distance(center_b);
      }
      
      case Mode::MIN_DISTANCE: {
        // Check if boxes overlap
        bool overlap_x = !(max_a.x < min_b.x || max_b.x < min_a.x);
        bool overlap_y = !(max_a.y < min_b.y || max_b.y < min_a.y);
        bool overlap_z = !(max_a.z < min_b.z || max_b.z < min_a.z);
        
        if (overlap_x && overlap_y && overlap_z) {
          return T(0);  // Boxes overlap
        }
        
        // Compute minimum distance between boxes
        T dx = std::max(T(0), std::max(min_a.x - max_b.x, min_b.x - max_a.x));
        T dy = std::max(T(0), std::max(min_a.y - max_b.y, min_b.y - max_a.y));
        T dz = std::max(T(0), std::max(min_a.z - max_b.z, min_b.z - max_a.z));
        
        return std::sqrt(dx * dx + dy * dy + dz * dz);
      }
      
      case Mode::IOU_DISTANCE: {
        // Compute intersection
        T inter_x = std::max(T(0), std::min(max_a.x, max_b.x) - std::max(min_a.x, min_b.x));
        T inter_y = std::max(T(0), std::min(max_a.y, max_b.y) - std::max(min_a.y, min_b.y));
        T inter_z = std::max(T(0), std::min(max_a.z, max_b.z) - std::max(min_a.z, min_b.z));
        T intersection_volume = inter_x * inter_y * inter_z;
        
        // Compute volumes
        T volume_a = (max_a.x - min_a.x) * (max_a.y - min_a.y) * (max_a.z - min_a.z);
        T volume_b = (max_b.x - min_b.x) * (max_b.y - min_b.y) * (max_b.z - min_b.z);
        T union_volume = volume_a + volume_b - intersection_volume;
        
        if (union_volume < std::numeric_limits<T>::epsilon()) {
          return T(1);  // Both boxes are degenerate
        }
        
        T iou = intersection_volume / union_volume;
        return T(1) - iou;
      }
    }
    
    return T(0);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }

private:
  Mode mode_;

  std::pair<point_type, point_type> compute_bounding_box(const cloud_type& cloud) const
  {
    point_type min_pt = cloud.points[0];
    point_type max_pt = cloud.points[0];
    
    for (const auto& point : cloud.points) {
      min_pt.x = std::min(min_pt.x, point.x);
      min_pt.y = std::min(min_pt.y, point.y);
      min_pt.z = std::min(min_pt.z, point.z);
      
      max_pt.x = std::max(max_pt.x, point.x);
      max_pt.y = std::max(max_pt.y, point.y);
      max_pt.z = std::max(max_pt.z, point.z);
    }
    
    return {min_pt, max_pt};
  }
};

/**
 * @brief LCP (Largest Common Pointset) metric for evaluating point cloud registration
 * 
 * Computes the average distance between transformed source points and their nearest
 * neighbors in the target cloud, considering only points within a threshold.
 * Lower scores indicate better alignment.
 * 
 * @tparam T Data type (float or double)
 */
template<typename T>
class LCPMetric : public base_metric_t<LCPMetric<T>, T>
{
public:
  using element_type = T;
  using point_type = toolbox::types::point_t<T>;
  using cloud_type = toolbox::types::point_cloud_t<T>;
  using transformation_type = Eigen::Matrix<T, 4, 4>;

  /**
   * @brief Constructor
   * @param inlier_threshold Distance threshold for considering a point as an inlier
   */
  explicit LCPMetric(T inlier_threshold = T(1.0)) 
      : inlier_threshold_(inlier_threshold) {}

  constexpr T distance_impl(const T* cloud_a, const T* cloud_b, std::size_t size) const
  {
    throw std::runtime_error("LCPMetric requires point cloud objects and transformation");
  }

  /**
   * @brief Compute LCP score between two point clouds
   * @param source Source point cloud
   * @param target Target point cloud
   * @param transform Transformation to apply to source points
   * @param[out] inliers Optional output vector of inlier indices
   * @return Average distance of inliers (lower is better)
   */
  T compute_lcp_score(const cloud_type& source, 
                      const cloud_type& target,
                      const transformation_type& transform,
                      std::vector<std::size_t>* inliers = nullptr) const
  {
    if (source.empty() || target.empty()) {
      return std::numeric_limits<T>::max();
    }

    // Clear inliers vector if provided
    if (inliers) {
      inliers->clear();
      inliers->reserve(source.size());
    }

    T total_distance = 0;
    std::size_t inlier_count = 0;
    const T threshold_squared = inlier_threshold_ * inlier_threshold_;

    // Extract rotation and translation from transformation
    Eigen::Matrix<T, 3, 3> rotation = transform.template block<3, 3>(0, 0);
    Eigen::Matrix<T, 3, 1> translation = transform.template block<3, 1>(0, 3);

    // For each source point
    for (std::size_t i = 0; i < source.size(); ++i) {
      const auto& src_pt = source.points[i];
      
      // Apply transformation
      Eigen::Matrix<T, 3, 1> src_vec(src_pt.x, src_pt.y, src_pt.z);
      Eigen::Matrix<T, 3, 1> transformed = rotation * src_vec + translation;
      point_type transformed_pt(transformed[0], transformed[1], transformed[2]);

      // Find nearest neighbor in target cloud
      T min_dist_squared = std::numeric_limits<T>::max();
      for (const auto& tgt_pt : target.points) {
        T dx = transformed_pt.x - tgt_pt.x;
        T dy = transformed_pt.y - tgt_pt.y;
        T dz = transformed_pt.z - tgt_pt.z;
        T dist_squared = dx * dx + dy * dy + dz * dz;
        min_dist_squared = std::min(min_dist_squared, dist_squared);
      }

      // Check if within threshold
      if (min_dist_squared <= threshold_squared) {
        total_distance += std::sqrt(min_dist_squared);
        inlier_count++;
        if (inliers) {
          inliers->push_back(i);
        }
      }
    }

    // Return average distance of inliers
    if (inlier_count == 0) {
      return std::numeric_limits<T>::max();
    }
    return total_distance / inlier_count;
  }


  /**
   * @brief Set inlier threshold
   * @param threshold New threshold value
   */
  void set_inlier_threshold(T threshold) { inlier_threshold_ = threshold; }

  /**
   * @brief Get inlier threshold
   * @return Current threshold value
   */
  [[nodiscard]] T get_inlier_threshold() const { return inlier_threshold_; }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }

private:
  T inlier_threshold_;
};

}  // namespace toolbox::metrics