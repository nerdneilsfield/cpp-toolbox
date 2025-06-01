#pragma once

#include <array>
#include <vector>
#include <cmath>

#include <cpp-toolbox/pcl/descriptors/base_descriptor_extractor.hpp>
#include <cpp-toolbox/pcl/norm/pca_norm.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::pcl
{

template<typename DataType>
struct vfh_signature_t : public base_signature_t<vfh_signature_t<DataType>>
{
  static constexpr std::size_t HISTOGRAM_SIZE = 308;  // 4*45 + 128
  std::array<DataType, HISTOGRAM_SIZE> histogram{};
  
  DataType distance(const vfh_signature_t& other) const
  {
    DataType dist = 0;
    for (std::size_t i = 0; i < HISTOGRAM_SIZE; ++i)
    {
      DataType diff = histogram[i] - other.histogram[i];
      dist += diff * diff;
    }
    return std::sqrt(dist);
  }
  
  bool operator==(const vfh_signature_t& other) const
  {
    return histogram == other.histogram;
  }
};

template<typename DataType, typename KNN>
class vfh_extractor_t
    : public base_descriptor_extractor_t<vfh_extractor_t<DataType, KNN>,
                                        DataType, vfh_signature_t<DataType>>
{
public:
  vfh_extractor_t() = default;

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

  void enable_parallel_impl(bool enable)
  {
    enable_parallel_ = enable;
  }

  void compute_impl(const toolbox::types::point_cloud_t<DataType>& cloud,
                    const std::vector<std::size_t>& keypoint_indices,
                    std::vector<vfh_signature_t<DataType>>& descriptors) const
  {
    descriptors.clear();
    
    // VFH is a global descriptor - compute for the entire cloud
    // Ignore keypoint_indices and process all points
    
    // Compute centroid
    toolbox::types::point_t<DataType> centroid(0, 0, 0);
    for (const auto& point : cloud.points)
    {
      centroid.x += point.x;
      centroid.y += point.y;
      centroid.z += point.z;
    }
    centroid.x /= cloud.points.size();
    centroid.y /= cloud.points.size();
    centroid.z /= cloud.points.size();
    
    // Compute normals if not provided
    std::vector<toolbox::types::point_t<DataType>> normals;
    compute_normals(cloud, normals);
    
    // Compute viewpoint vector (from centroid to viewpoint)
    toolbox::types::point_t<DataType> viewpoint(0, 0, 100);  // Default viewpoint
    
    // Compute VFH
    vfh_signature_t<DataType> vfh;
    compute_vfh(cloud, normals, centroid, viewpoint, vfh);
    
    descriptors.push_back(vfh);
  }

  void compute_impl(const toolbox::types::point_cloud_t<DataType>& cloud,
                    const std::vector<std::size_t>& keypoint_indices,
                    std::unique_ptr<std::vector<vfh_signature_t<DataType>>> descriptors) const
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
  
  void compute_vfh(const toolbox::types::point_cloud_t<DataType>& cloud,
                   const std::vector<toolbox::types::point_t<DataType>>& normals,
                   const toolbox::types::point_t<DataType>& centroid,
                   const toolbox::types::point_t<DataType>& viewpoint,
                   vfh_signature_t<DataType>& vfh) const
  {
    const std::size_t nr_bins_f1 = 45;
    const std::size_t nr_bins_f2 = 45;
    const std::size_t nr_bins_f3 = 45;
    const std::size_t nr_bins_f4 = 45;
    const std::size_t nr_bins_vp = 128;
    
    // Initialize histogram
    std::fill(vfh.histogram.begin(), vfh.histogram.end(), 0);
    
    // Viewpoint direction from centroid
    auto vp_dir = viewpoint;
    vp_dir.x -= centroid.x;
    vp_dir.y -= centroid.y;
    vp_dir.z -= centroid.z;
    auto vp_norm = vp_dir.normalize();
    
    // Compute extended FPFH for all point pairs
    for (std::size_t i = 0; i < cloud.points.size(); ++i)
    {
      const auto& pi = cloud.points[i];
      const auto& ni = normals[i];
      
      // Compute angle between normal and viewpoint direction
      DataType vp_angle = std::acos(std::min<DataType>(1.0, 
                                    std::max<DataType>(-1.0,
                                    ni.x * vp_norm.x + ni.y * vp_norm.y + ni.z * vp_norm.z)));
      
      // Add to viewpoint component histogram
      std::size_t vp_bin = static_cast<std::size_t>(vp_angle / M_PI * nr_bins_vp);
      if (vp_bin >= nr_bins_vp) vp_bin = nr_bins_vp - 1;
      vfh.histogram[180 + vp_bin] += 1.0;
      
      // Compute FPFH features with other points
      for (std::size_t j = i + 1; j < cloud.points.size(); ++j)
      {
        const auto& pj = cloud.points[j];
        const auto& nj = normals[j];
        
        // Compute features between points i and j
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
        
        // Normalize angles
        f1 = (f1 + 1.0) * 0.5;  // [0, 1]
        f2 = (f2 + 1.0) * 0.5;  // [0, 1]
        f3 = (f3 + M_PI) / (2.0 * M_PI);  // [0, 1]
        f4 = (f4 + M_PI) / (2.0 * M_PI);  // [0, 1]
        
        // Compute bins
        std::size_t bin_f1 = static_cast<std::size_t>(f1 * nr_bins_f1);
        std::size_t bin_f2 = static_cast<std::size_t>(f2 * nr_bins_f2);
        std::size_t bin_f3 = static_cast<std::size_t>(f3 * nr_bins_f3);
        std::size_t bin_f4 = static_cast<std::size_t>(f4 * nr_bins_f4);
        
        if (bin_f1 >= nr_bins_f1) bin_f1 = nr_bins_f1 - 1;
        if (bin_f2 >= nr_bins_f2) bin_f2 = nr_bins_f2 - 1;
        if (bin_f3 >= nr_bins_f3) bin_f3 = nr_bins_f3 - 1;
        if (bin_f4 >= nr_bins_f4) bin_f4 = nr_bins_f4 - 1;
        
        // Update histogram
        vfh.histogram[bin_f1] += 1.0;
        vfh.histogram[45 + bin_f2] += 1.0;
        vfh.histogram[90 + bin_f3] += 1.0;
        vfh.histogram[135 + bin_f4] += 1.0;
      }
    }
    
    // Normalize histogram
    DataType sum = 0;
    for (auto& val : vfh.histogram)
    {
      sum += val;
    }
    if (sum > 0)
    {
      for (auto& val : vfh.histogram)
      {
        val /= sum;
      }
    }
  }

private:
  const toolbox::types::point_cloud_t<DataType>* cloud_ = nullptr;
  KNN* knn_ = nullptr;
  DataType search_radius_ = 0.1;
  std::size_t num_neighbors_ = 10;
  bool enable_parallel_ = true;
};

}  // namespace toolbox::pcl