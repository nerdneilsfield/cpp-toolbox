#pragma once

#include <array>
#include <vector>
#include <cmath>

#include <cpp-toolbox/pcl/descriptors/base_descriptor_extractor.hpp>
#include <cpp-toolbox/pcl/norm/pca_norm.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/concurrent/parallel.hpp>

namespace toolbox::pcl
{

template<typename DataType>
struct dsc3d_signature_t : public base_signature_t<dsc3d_signature_t<DataType>>
{
  static constexpr std::size_t HISTOGRAM_SIZE = 1980;  // 11 * 12 * 15
  std::array<DataType, HISTOGRAM_SIZE> histogram{};
  
  DataType distance(const dsc3d_signature_t& other) const
  {
    DataType dist = 0;
    for (std::size_t i = 0; i < HISTOGRAM_SIZE; ++i)
    {
      DataType diff = histogram[i] - other.histogram[i];
      dist += diff * diff;
    }
    return std::sqrt(dist);
  }
  
  bool operator==(const dsc3d_signature_t& other) const
  {
    return histogram == other.histogram;
  }
};

template<typename DataType, typename KNN>
class dsc3d_extractor_t
    : public base_descriptor_extractor_t<dsc3d_extractor_t<DataType, KNN>,
                                        DataType, dsc3d_signature_t<DataType>>
{
public:
  dsc3d_extractor_t() = default;

  void set_input(const toolbox::types::point_cloud_t<DataType>& cloud)
  {
    cloud_ = &cloud;
  }

  void set_knn(KNN& knn)
  {
    knn_ = &knn;
  }

  void set_search_radius(DataType radius)
  {
    search_radius_ = radius;
  }

  void set_num_neighbors(std::size_t num_neighbors)
  {
    num_neighbors_ = num_neighbors;
  }

  void set_minimal_radius(DataType radius)
  {
    minimal_radius_ = radius;
  }

  void set_point_density_radius(DataType radius)
  {
    point_density_radius_ = radius;
  }

  void enable_parallel_impl(bool enable)
  {
    enable_parallel_ = enable;
  }

  void compute_impl(const toolbox::types::point_cloud_t<DataType>& cloud,
                    const std::vector<std::size_t>& keypoint_indices,
                    std::vector<dsc3d_signature_t<DataType>>& descriptors) const
  {
    descriptors.clear();
    descriptors.resize(keypoint_indices.size());
    
    if (!cloud_ || !knn_) return;
    
    // Compute normals if not provided
    std::vector<toolbox::types::point_t<DataType>> normals;
    compute_normals(cloud, normals);
    
    auto compute_descriptor = [&](std::size_t idx) {
      const auto keypoint_idx = keypoint_indices[idx];
      compute_3dsc(cloud, normals, keypoint_idx, descriptors[idx]);
    };
    
    if (enable_parallel_)
    {
      toolbox::concurrent::parallel_for_each(
        keypoint_indices.size(), compute_descriptor);
    }
    else
    {
      for (std::size_t i = 0; i < keypoint_indices.size(); ++i)
      {
        compute_descriptor(i);
      }
    }
  }

  void compute_impl(const toolbox::types::point_cloud_t<DataType>& cloud,
                    const std::vector<std::size_t>& keypoint_indices,
                    std::unique_ptr<std::vector<dsc3d_signature_t<DataType>>> descriptors) const
  {
    descriptors->clear();
    compute_impl(cloud, keypoint_indices, *descriptors);
  }

private:
  void compute_normals(const toolbox::types::point_cloud_t<DataType>& cloud,
                       std::vector<toolbox::types::point_t<DataType>>& normals) const
  {
    if (!knn_) return;
    
    pca_norm_t<DataType, KNN> normal_estimator;
    normal_estimator.set_input(cloud);
    normal_estimator.set_knn(*knn_);
    normal_estimator.set_search_radius(search_radius_);
    normal_estimator.set_num_neighbors(num_neighbors_);
    normal_estimator.enable_parallel(enable_parallel_);
    
    normal_estimator.compute(cloud, normals);
  }
  
  void compute_3dsc(const toolbox::types::point_cloud_t<DataType>& cloud,
                    const std::vector<toolbox::types::point_t<DataType>>& normals,
                    std::size_t keypoint_idx,
                    dsc3d_signature_t<DataType>& descriptor) const
  {
    // Initialize histogram
    std::fill(descriptor.histogram.begin(), descriptor.histogram.end(), 0);
    
    const auto& keypoint = cloud.points[keypoint_idx];
    const auto& normal = normals[keypoint_idx];
    
    // Find neighbors
    std::vector<std::size_t> neighbors;
    std::vector<DataType> distances;
    knn_->radius_neighbors(keypoint, search_radius_, neighbors, distances);
    
    if (neighbors.size() < 3) return;
    
    // Create local reference frame
    auto lrf = compute_local_reference_frame(cloud, keypoint, normal, neighbors);
    
    // Compute shape context
    const std::size_t nr_bins_r = 11;    // Radial bins
    const std::size_t nr_bins_theta = 12; // Azimuth bins
    const std::size_t nr_bins_phi = 15;   // Elevation bins
    
    DataType min_radius = minimal_radius_;
    DataType max_radius = search_radius_;
    DataType log_factor = (std::log(max_radius) - std::log(min_radius)) / nr_bins_r;
    
    for (const auto& neighbor_idx : neighbors)
    {
      if (neighbor_idx == keypoint_idx) continue;
      
      const auto& neighbor = cloud.points[neighbor_idx];
      
      // Transform to local frame
      auto local_pt = transform_to_local_frame(neighbor, keypoint, lrf);
      
      // Convert to spherical coordinates
      DataType r = std::sqrt(local_pt.x * local_pt.x + 
                            local_pt.y * local_pt.y + 
                            local_pt.z * local_pt.z);
      
      if (r < min_radius) continue;
      
      DataType theta = std::atan2(local_pt.y, local_pt.x) + M_PI;  // [0, 2*pi]
      DataType phi = std::acos(std::min<DataType>(1.0, 
                               std::max<DataType>(-1.0, local_pt.z / r)));  // [0, pi]
      
      // Compute bins
      std::size_t bin_r = static_cast<std::size_t>(
        (std::log(r) - std::log(min_radius)) / log_factor);
      std::size_t bin_theta = static_cast<std::size_t>(
        theta / (2.0 * M_PI) * nr_bins_theta);
      std::size_t bin_phi = static_cast<std::size_t>(
        phi / M_PI * nr_bins_phi);
      
      // Clamp bins
      if (bin_r >= nr_bins_r) bin_r = nr_bins_r - 1;
      if (bin_theta >= nr_bins_theta) bin_theta = nr_bins_theta - 1;
      if (bin_phi >= nr_bins_phi) bin_phi = nr_bins_phi - 1;
      
      // Update histogram
      std::size_t bin_idx = bin_r * nr_bins_theta * nr_bins_phi + 
                           bin_theta * nr_bins_phi + bin_phi;
      descriptor.histogram[bin_idx] += 1.0;
    }
    
    // Normalize by point density
    DataType density_weight = compute_point_density(cloud, keypoint);
    
    // Normalize histogram
    DataType sum = 0;
    for (auto& val : descriptor.histogram)
    {
      sum += val;
    }
    if (sum > 0)
    {
      for (auto& val : descriptor.histogram)
      {
        val = (val / sum) * density_weight;
      }
    }
  }
  
  struct LocalReferenceFrame
  {
    toolbox::types::point_t<DataType> x_axis;
    toolbox::types::point_t<DataType> y_axis;
    toolbox::types::point_t<DataType> z_axis;
  };
  
  LocalReferenceFrame compute_local_reference_frame(
      const toolbox::types::point_cloud_t<DataType>& cloud,
      const toolbox::types::point_t<DataType>& keypoint,
      const toolbox::types::point_t<DataType>& normal,
      const std::vector<std::size_t>& neighbors) const
  {
    LocalReferenceFrame lrf;
    
    // Z-axis is the normal
    lrf.z_axis = normal.normalize();
    
    // Find the point with maximum angle from normal
    DataType max_angle = 0;
    toolbox::types::point_t<DataType> max_point;
    
    for (const auto& idx : neighbors)
    {
      if (idx == keypoint.index) continue;
      
      auto diff = cloud.points[idx];
      diff.x -= keypoint.x;
      diff.y -= keypoint.y;
      diff.z -= keypoint.z;
      
      auto d_norm = diff.normalize();
      DataType angle = std::acos(std::abs(
        d_norm.x * lrf.z_axis.x + 
        d_norm.y * lrf.z_axis.y + 
        d_norm.z * lrf.z_axis.z));
      
      if (angle > max_angle)
      {
        max_angle = angle;
        max_point = d_norm;
      }
    }
    
    // X-axis perpendicular to Z
    lrf.x_axis = max_point;
    lrf.x_axis.x -= (lrf.x_axis.x * lrf.z_axis.x + 
                     lrf.x_axis.y * lrf.z_axis.y + 
                     lrf.x_axis.z * lrf.z_axis.z) * lrf.z_axis.x;
    lrf.x_axis.y -= (lrf.x_axis.x * lrf.z_axis.x + 
                     lrf.x_axis.y * lrf.z_axis.y + 
                     lrf.x_axis.z * lrf.z_axis.z) * lrf.z_axis.y;
    lrf.x_axis.z -= (lrf.x_axis.x * lrf.z_axis.x + 
                     lrf.x_axis.y * lrf.z_axis.y + 
                     lrf.x_axis.z * lrf.z_axis.z) * lrf.z_axis.z;
    lrf.x_axis = lrf.x_axis.normalize();
    
    // Y-axis = Z x X
    lrf.y_axis.x = lrf.z_axis.y * lrf.x_axis.z - lrf.z_axis.z * lrf.x_axis.y;
    lrf.y_axis.y = lrf.z_axis.z * lrf.x_axis.x - lrf.z_axis.x * lrf.x_axis.z;
    lrf.y_axis.z = lrf.z_axis.x * lrf.x_axis.y - lrf.z_axis.y * lrf.x_axis.x;
    
    return lrf;
  }
  
  toolbox::types::point_t<DataType> transform_to_local_frame(
      const toolbox::types::point_t<DataType>& point,
      const toolbox::types::point_t<DataType>& origin,
      const LocalReferenceFrame& lrf) const
  {
    // Translate
    auto p = point;
    p.x -= origin.x;
    p.y -= origin.y;
    p.z -= origin.z;
    
    // Rotate
    toolbox::types::point_t<DataType> result;
    result.x = p.x * lrf.x_axis.x + p.y * lrf.x_axis.y + p.z * lrf.x_axis.z;
    result.y = p.x * lrf.y_axis.x + p.y * lrf.y_axis.y + p.z * lrf.y_axis.z;
    result.z = p.x * lrf.z_axis.x + p.y * lrf.z_axis.y + p.z * lrf.z_axis.z;
    
    return result;
  }
  
  DataType compute_point_density(const toolbox::types::point_cloud_t<DataType>& cloud,
                                const toolbox::types::point_t<DataType>& point) const
  {
    std::vector<std::size_t> neighbors;
    std::vector<DataType> distances;
    knn_->radius_neighbors(point, point_density_radius_, neighbors, distances);
    
    DataType volume = (4.0 / 3.0) * M_PI * 
                     point_density_radius_ * point_density_radius_ * point_density_radius_;
    
    return static_cast<DataType>(neighbors.size()) / volume;
  }

private:
  const toolbox::types::point_cloud_t<DataType>* cloud_ = nullptr;
  KNN* knn_ = nullptr;
  DataType search_radius_ = 0.5;
  DataType minimal_radius_ = 0.01;
  DataType point_density_radius_ = 0.05;
  std::size_t num_neighbors_ = 10;
  bool enable_parallel_ = true;
};

}  // namespace toolbox::pcl