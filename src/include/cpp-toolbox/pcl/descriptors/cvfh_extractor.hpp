#pragma once

#include <array>
#include <vector>
#include <algorithm>

#include <cpp-toolbox/pcl/descriptors/base_descriptor_extractor.hpp>
#include <cpp-toolbox/pcl/norm/pca_norm.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/concurrent/parallel.hpp>

namespace toolbox::pcl
{

template<typename DataType>
struct cvfh_signature_t : public base_signature_t<cvfh_signature_t<DataType>>
{
  static constexpr std::size_t HISTOGRAM_SIZE = 308;  // Same as VFH
  std::array<DataType, HISTOGRAM_SIZE> histogram{};
  
  DataType distance(const cvfh_signature_t& other) const
  {
    DataType dist = 0;
    for (std::size_t i = 0; i < HISTOGRAM_SIZE; ++i)
    {
      DataType diff = histogram[i] - other.histogram[i];
      dist += diff * diff;
    }
    return std::sqrt(dist);
  }
  
  bool operator==(const cvfh_signature_t& other) const
  {
    return histogram == other.histogram;
  }
};

template<typename DataType, typename KNN>
class cvfh_extractor_t
    : public base_descriptor_extractor_t<cvfh_extractor_t<DataType, KNN>,
                                        DataType, cvfh_signature_t<DataType>>
{
public:
  cvfh_extractor_t() = default;

  std::size_t set_input(const toolbox::types::point_cloud_t<DataType>& cloud)
  {
    cloud_ = &cloud;
    return cloud.size();
  }

  std::size_t set_knn(KNN& knn)
  {
    knn_ = &knn;
    return cloud_ ? cloud_->size() : 0;
  }

  std::size_t set_search_radius(DataType radius)
  {
    search_radius_ = radius;
    return cloud_ ? cloud_->size() : 0;
  }

  std::size_t set_num_neighbors(std::size_t num_neighbors)
  {
    num_neighbors_ = num_neighbors;
    return cloud_ ? cloud_->size() : 0;
  }

  std::size_t set_cluster_tolerance(DataType tolerance)
  {
    cluster_tolerance_ = tolerance;
    return cloud_ ? cloud_->size() : 0;
  }

  std::size_t set_eps_angle_threshold(DataType threshold)
  {
    eps_angle_threshold_ = threshold;
    return cloud_ ? cloud_->size() : 0;
  }

  std::size_t set_curvature_threshold(DataType threshold)
  {
    curvature_threshold_ = threshold;
    return cloud_ ? cloud_->size() : 0;
  }

  void enable_parallel_impl(bool enable)
  {
    enable_parallel_ = enable;
  }

  void compute_impl(const toolbox::types::point_cloud_t<DataType>& cloud,
                    const std::vector<std::size_t>& keypoint_indices,
                    std::vector<cvfh_signature_t<DataType>>& descriptors) const
  {
    descriptors.clear();
    
    if (!cloud_ || !knn_) return;
    
    // Compute normals
    std::vector<toolbox::types::point_t<DataType>> normals;
    compute_normals(cloud, normals);
    
    // Segment cloud into smooth clusters
    std::vector<std::vector<std::size_t>> clusters;
    segment_smooth_clusters(cloud, normals, clusters);
    
    // Compute CVFH for each cluster
    for (const auto& cluster : clusters)
    {
      if (cluster.size() < 3) continue;
      
      cvfh_signature_t<DataType> cvfh;
      compute_cluster_vfh(cloud, normals, cluster, cvfh);
      descriptors.push_back(cvfh);
    }
  }

  void compute_impl(const toolbox::types::point_cloud_t<DataType>& cloud,
                    const std::vector<std::size_t>& keypoint_indices,
                    std::unique_ptr<std::vector<cvfh_signature_t<DataType>>> descriptors) const
  {
    descriptors->clear();
    compute_impl(cloud, keypoint_indices, *descriptors);
  }

private:
  void compute_normals(const toolbox::types::point_cloud_t<DataType>& cloud,
                       std::vector<toolbox::types::point_t<DataType>>& normals) const
  {
    if (!knn_) return;
    
    pca_norm_extractor_t<DataType, KNN> normal_estimator;
    normal_estimator.set_input(cloud);
    normal_estimator.set_knn(*knn_);
    normal_estimator.set_num_neighbors(num_neighbors_);
    normal_estimator.enable_parallel(enable_parallel_);
    
    auto normal_cloud = normal_estimator.extract();
    normals.clear();
    normals.reserve(normal_cloud.size());
    for (const auto& pt : normal_cloud.points)
    {
      normals.push_back(pt);
    }
  }
  
  void segment_smooth_clusters(const toolbox::types::point_cloud_t<DataType>& cloud,
                               const std::vector<toolbox::types::point_t<DataType>>& normals,
                               std::vector<std::vector<std::size_t>>& clusters) const
  {
    std::vector<bool> processed(cloud.points.size(), false);
    
    for (std::size_t i = 0; i < cloud.points.size(); ++i)
    {
      if (processed[i]) continue;
      
      std::vector<std::size_t> cluster;
      std::vector<std::size_t> seeds;
      seeds.push_back(i);
      processed[i] = true;
      
      while (!seeds.empty())
      {
        std::size_t current = seeds.back();
        seeds.pop_back();
        cluster.push_back(current);
        
        // Find neighbors
        std::vector<std::size_t> neighbors;
        std::vector<DataType> distances;
        knn_->radius_neighbors(cloud.points[current], cluster_tolerance_, 
                              neighbors, distances);
        
        for (const auto& neighbor_idx : neighbors)
        {
          if (processed[neighbor_idx]) continue;
          
          // Check normal similarity
          const auto& n1 = normals[current];
          const auto& n2 = normals[neighbor_idx];
          
          DataType dot = n1.x * n2.x + n1.y * n2.y + n1.z * n2.z;
          DataType angle = std::acos(std::min<DataType>(1.0, 
                                    std::max<DataType>(-1.0, dot)));
          
          if (angle < eps_angle_threshold_)
          {
            seeds.push_back(neighbor_idx);
            processed[neighbor_idx] = true;
          }
        }
      }
      
      if (cluster.size() >= 3)
      {
        clusters.push_back(cluster);
      }
    }
  }
  
  void compute_cluster_vfh(const toolbox::types::point_cloud_t<DataType>& cloud,
                          const std::vector<toolbox::types::point_t<DataType>>& normals,
                          const std::vector<std::size_t>& cluster,
                          cvfh_signature_t<DataType>& cvfh) const
  {
    // Compute cluster centroid
    toolbox::types::point_t<DataType> centroid(0, 0, 0);
    for (const auto& idx : cluster)
    {
      centroid.x += cloud.points[idx].x;
      centroid.y += cloud.points[idx].y;
      centroid.z += cloud.points[idx].z;
    }
    centroid.x /= cluster.size();
    centroid.y /= cluster.size();
    centroid.z /= cluster.size();
    
    // Compute viewpoint
    toolbox::types::point_t<DataType> viewpoint(0, 0, 100);
    
    // Initialize histogram
    std::fill(cvfh.histogram.begin(), cvfh.histogram.end(), 0);
    
    const std::size_t nr_bins_f1 = 45;
    const std::size_t nr_bins_f2 = 45;
    const std::size_t nr_bins_f3 = 45;
    const std::size_t nr_bins_f4 = 45;
    const std::size_t nr_bins_vp = 128;
    
    // Viewpoint direction
    auto vp_dir = viewpoint;
    vp_dir.x -= centroid.x;
    vp_dir.y -= centroid.y;
    vp_dir.z -= centroid.z;
    auto vp_norm = vp_dir.normalize();
    
    // Compute shape distribution
    for (std::size_t i = 0; i < cluster.size(); ++i)
    {
      const auto& idx_i = cluster[i];
      const auto& pi = cloud.points[idx_i];
      const auto& ni = normals[idx_i];
      
      // Viewpoint angle
      DataType vp_angle = std::acos(std::min<DataType>(1.0, 
                                    std::max<DataType>(-1.0,
                                    ni.x * vp_norm.x + ni.y * vp_norm.y + ni.z * vp_norm.z)));
      
      std::size_t vp_bin = static_cast<std::size_t>(vp_angle / M_PI * nr_bins_vp);
      if (vp_bin >= nr_bins_vp) vp_bin = nr_bins_vp - 1;
      cvfh.histogram[180 + vp_bin] += 1.0;
      
      // Compute pairwise features
      for (std::size_t j = i + 1; j < cluster.size(); ++j)
      {
        const auto& idx_j = cluster[j];
        const auto& pj = cloud.points[idx_j];
        const auto& nj = normals[idx_j];
        
        // Compute features
        auto d = pj;
        d.x -= pi.x;
        d.y -= pi.y;
        d.z -= pi.z;
        DataType dist = std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
        
        if (dist < 1e-8) continue;
        
        d.x /= dist;
        d.y /= dist;
        d.z /= dist;
        
        // Compute angles
        DataType f1 = ni.x * d.x + ni.y * d.y + ni.z * d.z;
        DataType f2 = (d.x * nj.x + d.y * nj.y + d.z * nj.z) - f1;
        DataType f3 = std::atan2(
          ni.y * d.z - ni.z * d.y,
          ni.x * d.x + ni.y * d.y + ni.z * d.z
        );
        DataType f4 = std::atan2(
          nj.y * d.z - nj.z * d.y,
          nj.x * d.x + nj.y * d.y + nj.z * d.z
        ) - f3;
        
        // Normalize
        f1 = (f1 + 1.0) * 0.5;
        f2 = (f2 + 1.0) * 0.5;
        f3 = (f3 + M_PI) / (2.0 * M_PI);
        f4 = (f4 + M_PI) / (2.0 * M_PI);
        
        // Bins
        std::size_t bin_f1 = static_cast<std::size_t>(f1 * nr_bins_f1);
        std::size_t bin_f2 = static_cast<std::size_t>(f2 * nr_bins_f2);
        std::size_t bin_f3 = static_cast<std::size_t>(f3 * nr_bins_f3);
        std::size_t bin_f4 = static_cast<std::size_t>(f4 * nr_bins_f4);
        
        if (bin_f1 >= nr_bins_f1) bin_f1 = nr_bins_f1 - 1;
        if (bin_f2 >= nr_bins_f2) bin_f2 = nr_bins_f2 - 1;
        if (bin_f3 >= nr_bins_f3) bin_f3 = nr_bins_f3 - 1;
        if (bin_f4 >= nr_bins_f4) bin_f4 = nr_bins_f4 - 1;
        
        // Update histogram with distance weighting
        DataType weight = 1.0 / (1.0 + dist);
        cvfh.histogram[bin_f1] += weight;
        cvfh.histogram[45 + bin_f2] += weight;
        cvfh.histogram[90 + bin_f3] += weight;
        cvfh.histogram[135 + bin_f4] += weight;
      }
    }
    
    // Normalize
    DataType sum = 0;
    for (auto& val : cvfh.histogram)
    {
      sum += val;
    }
    if (sum > 0)
    {
      for (auto& val : cvfh.histogram)
      {
        val /= sum;
      }
    }
  }

private:
  const toolbox::types::point_cloud_t<DataType>* cloud_ = nullptr;
  KNN* knn_ = nullptr;
  DataType search_radius_ = 0.1;
  DataType cluster_tolerance_ = 0.05;
  DataType eps_angle_threshold_ = 0.08;  // ~5 degrees
  DataType curvature_threshold_ = 0.1;
  std::size_t num_neighbors_ = 10;
  bool enable_parallel_ = true;
};

}  // namespace toolbox::pcl