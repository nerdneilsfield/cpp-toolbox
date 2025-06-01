#pragma once

#include <algorithm>
#include <array>
#include <vector>

#include <cpp-toolbox/concurrent/parallel.hpp>
#include <cpp-toolbox/pcl/descriptors/base_descriptor_extractor.hpp>
#include <cpp-toolbox/types/point.hpp>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>

namespace toolbox::pcl
{

template<typename DataType>
struct rops_signature_t : public base_signature_t<rops_signature_t<DataType>>
{
  static constexpr std::size_t HISTOGRAM_SIZE = 135;  // 5 * 3 * 3 * 3
  std::array<DataType, HISTOGRAM_SIZE> histogram {};

  DataType distance_impl(const rops_signature_t& other) const
  {
    DataType dist = 0;
    for (std::size_t i = 0; i < HISTOGRAM_SIZE; ++i) {
      DataType diff = histogram[i] - other.histogram[i];
      dist += diff * diff;
    }
    return std::sqrt(dist);
  }
};

template<typename DataType, typename KNN>
class rops_extractor_t
    : public base_descriptor_extractor_t<rops_extractor_t<DataType, KNN>,
                                         DataType,
                                         rops_signature_t<DataType>>
{
public:
  rops_extractor_t() = default;

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

  std::size_t set_num_partitions_x(std::size_t partitions)
  {
    num_partitions_x_ = partitions;
    return cloud_ ? cloud_->size() : 0;
  }

  std::size_t set_num_partitions_y(std::size_t partitions)
  {
    num_partitions_y_ = partitions;
    return cloud_ ? cloud_->size() : 0;
  }

  std::size_t set_num_partitions_z(std::size_t partitions)
  {
    num_partitions_z_ = partitions;
    return cloud_ ? cloud_->size() : 0;
  }

  std::size_t set_num_rotations(std::size_t rotations)
  {
    num_rotations_ = rotations;
    return cloud_ ? cloud_->size() : 0;
  }

  void enable_parallel_impl(bool enable) { enable_parallel_ = enable; }

  void compute_impl(const toolbox::types::point_cloud_t<DataType>& cloud,
                    const std::vector<std::size_t>& keypoint_indices,
                    std::vector<rops_signature_t<DataType>>& descriptors) const
  {
    descriptors.clear();
    descriptors.resize(keypoint_indices.size());

    if (!cloud_ || !knn_)
      return;

    auto compute_descriptor = [&](std::size_t idx)
    {
      const auto keypoint_idx = keypoint_indices[idx];
      compute_rops(cloud, keypoint_idx, descriptors[idx]);
    };

    if (enable_parallel_) {
#pragma omp parallel for
      for (int i = 0; i < static_cast<int>(keypoint_indices.size()); ++i) {
        compute_descriptor(static_cast<std::size_t>(i));
      }
    } else {
      for (std::size_t i = 0; i < keypoint_indices.size(); ++i) {
        compute_descriptor(i);
      }
    }
  }

  void compute_impl(const toolbox::types::point_cloud_t<DataType>& cloud,
                    const std::vector<std::size_t>& keypoint_indices,
                    std::unique_ptr<std::vector<rops_signature_t<DataType>>>
                        descriptors) const
  {
    descriptors->clear();
    compute_impl(cloud, keypoint_indices, *descriptors);
  }

private:
  void compute_rops(const toolbox::types::point_cloud_t<DataType>& cloud,
                    std::size_t keypoint_idx,
                    rops_signature_t<DataType>& descriptor) const
  {
    // Initialize
    std::fill(descriptor.histogram.begin(), descriptor.histogram.end(), 0);

    const auto& keypoint = cloud.points[keypoint_idx];

    // Find neighbors
    std::vector<std::size_t> neighbors;
    std::vector<DataType> distances;
    knn_->radius_neighbors(keypoint, search_radius_, neighbors, distances);

    if (neighbors.size() < 3)
      return;

    // Transform to local coordinates
    std::vector<Eigen::Vector3f> local_points;
    local_points.reserve(neighbors.size());

    for (const auto& idx : neighbors) {
      const auto& pt = cloud.points[idx];
      local_points.emplace_back(
          pt.x - keypoint.x, pt.y - keypoint.y, pt.z - keypoint.z);
    }

    // Compute local reference frame
    auto lrf = compute_lrf(local_points);

    // Transform points to LRF
    for (auto& pt : local_points) {
      pt = lrf.transpose() * pt;
    }

    // Generate rotation matrices
    std::vector<Eigen::Matrix3f> rotations;
    generate_rotation_matrices(rotations);

    // Compute statistics for each rotation
    std::vector<std::vector<DataType>> rotation_stats;
    rotation_stats.resize(num_rotations_);

    for (std::size_t r = 0; r < num_rotations_; ++r) {
      // Rotate points
      std::vector<Eigen::Vector3f> rotated_points;
      rotated_points.reserve(local_points.size());

      for (const auto& pt : local_points) {
        rotated_points.push_back(rotations[r] * pt);
      }

      // Compute bounding box
      Eigen::Vector3f bbox_min(std::numeric_limits<float>::max(),
                               std::numeric_limits<float>::max(),
                               std::numeric_limits<float>::max());
      Eigen::Vector3f bbox_max(std::numeric_limits<float>::lowest(),
                               std::numeric_limits<float>::lowest(),
                               std::numeric_limits<float>::lowest());

      for (const auto& pt : rotated_points) {
        bbox_min = bbox_min.cwiseMin(pt);
        bbox_max = bbox_max.cwiseMax(pt);
      }

      Eigen::Vector3f bbox_size = bbox_max - bbox_min;

      // Create partitions
      std::vector<std::vector<std::vector<std::vector<float>>>> partitions;
      partitions.resize(num_partitions_x_);
      for (auto& x : partitions) {
        x.resize(num_partitions_y_);
        for (auto& y : x) {
          y.resize(num_partitions_z_);
        }
      }

      // Assign points to partitions
      for (const auto& pt : rotated_points) {
        Eigen::Vector3f normalized = (pt - bbox_min).cwiseQuotient(bbox_size);

        std::size_t x_idx =
            static_cast<std::size_t>(normalized(0) * num_partitions_x_);
        std::size_t y_idx =
            static_cast<std::size_t>(normalized(1) * num_partitions_y_);
        std::size_t z_idx =
            static_cast<std::size_t>(normalized(2) * num_partitions_z_);

        // Clamp indices
        x_idx = std::min(x_idx, num_partitions_x_ - 1);
        y_idx = std::min(y_idx, num_partitions_y_ - 1);
        z_idx = std::min(z_idx, num_partitions_z_ - 1);

        // Add point distance to partition
        float dist = pt.norm();
        partitions[x_idx][y_idx][z_idx].push_back(dist);
      }

      // Compute statistics for each partition
      std::vector<DataType>& stats = rotation_stats[r];
      stats.reserve(num_partitions_x_ * num_partitions_y_ * num_partitions_z_);

      for (std::size_t x = 0; x < num_partitions_x_; ++x) {
        for (std::size_t y = 0; y < num_partitions_y_; ++y) {
          for (std::size_t z = 0; z < num_partitions_z_; ++z) {
            const auto& partition = partitions[x][y][z];

            if (partition.empty()) {
              stats.push_back(0);
            } else {
              // Compute mean distance
              DataType sum = 0;
              for (float d : partition) {
                sum += d;
              }
              stats.push_back(sum / partition.size());
            }
          }
        }
      }
    }

    // Build final histogram from all rotation statistics
    std::size_t hist_idx = 0;
    for (std::size_t r = 0; r < num_rotations_; ++r) {
      const auto& stats = rotation_stats[r];
      for (std::size_t i = 0; i < stats.size(); ++i) {
        descriptor.histogram[hist_idx++] = stats[i];
      }
    }

    // Normalize
    DataType sum = 0;
    for (auto& val : descriptor.histogram) {
      sum += val * val;
    }
    if (sum > 0) {
      sum = std::sqrt(sum);
      for (auto& val : descriptor.histogram) {
        val /= sum;
      }
    }
  }

  Eigen::Matrix3f compute_lrf(const std::vector<Eigen::Vector3f>& points) const
  {
    // Compute covariance matrix
    Eigen::Matrix3f covariance = Eigen::Matrix3f::Zero();

    for (const auto& pt : points) {
      covariance += pt * pt.transpose();
    }
    covariance /= points.size();

    // Eigenvalue decomposition
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(covariance);

    // Return eigenvectors as local reference frame
    return solver.eigenvectors();
  }

  void generate_rotation_matrices(std::vector<Eigen::Matrix3f>& rotations) const
  {
    rotations.clear();
    rotations.resize(num_rotations_);

    // Generate evenly spaced rotations around axes
    for (std::size_t i = 0; i < num_rotations_; ++i) {
      DataType angle = 2.0 * M_PI * i / num_rotations_;

      // Simple rotation around Z-axis for now
      Eigen::Matrix3f rot;
      rot << std::cos(angle), -std::sin(angle), 0, std::sin(angle),
          std::cos(angle), 0, 0, 0, 1;

      rotations[i] = rot;
    }
  }

private:
  const toolbox::types::point_cloud_t<DataType>* cloud_ = nullptr;
  KNN* knn_ = nullptr;
  DataType search_radius_ = 0.2;
  std::size_t num_neighbors_ = 50;
  std::size_t num_partitions_x_ = 3;
  std::size_t num_partitions_y_ = 3;
  std::size_t num_partitions_z_ = 3;
  std::size_t num_rotations_ = 5;
  bool enable_parallel_ = true;
};

}  // namespace toolbox::pcl