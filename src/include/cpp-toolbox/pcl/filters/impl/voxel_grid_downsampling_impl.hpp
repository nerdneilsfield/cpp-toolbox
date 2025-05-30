#pragma once

#include <cmath>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <cpp-toolbox/concurrent/parallel.hpp>
#include <cpp-toolbox/pcl/filters/voxel_grid_downsampling.hpp>
#include <cpp-toolbox/types/minmax.hpp>

namespace toolbox::pcl
{

// 实现 voxel_data_soa_t 的方法

template<typename DataType>
std::size_t voxel_grid_downsampling_t<DataType>::voxel_data_soa_t::add_voxel()
{
  std::size_t idx = sum_x.size();
  sum_x.push_back(0);
  sum_y.push_back(0);
  sum_z.push_back(0);
  sum_nx.push_back(0);
  sum_ny.push_back(0);
  sum_nz.push_back(0);
  sum_r.push_back(0);
  sum_g.push_back(0);
  sum_b.push_back(0);
  counts.push_back(0);
  voxel_indices.push_back(idx);
  return idx;
}

template<typename DataType>
std::size_t voxel_grid_downsampling_t<DataType>::voxel_data_soa_t::size() const
{
  return sum_x.size();
}

template<typename DataType>
void voxel_grid_downsampling_t<DataType>::voxel_data_soa_t::reserve(
    std::size_t reserve_size)
{
  sum_x.reserve(reserve_size);
  sum_y.reserve(reserve_size);
  sum_z.reserve(reserve_size);
  sum_nx.reserve(reserve_size);
  sum_ny.reserve(reserve_size);
  sum_nz.reserve(reserve_size);
  sum_r.reserve(reserve_size);
  sum_g.reserve(reserve_size);
  sum_b.reserve(reserve_size);
  counts.reserve(reserve_size);
  voxel_indices.reserve(reserve_size);
}

template<typename DataType>
void voxel_grid_downsampling_t<DataType>::voxel_data_soa_t::clear()
{
  sum_x.clear();
  sum_y.clear();
  sum_z.clear();
  sum_nx.clear();
  sum_ny.clear();
  sum_nz.clear();
  sum_r.clear();
  sum_g.clear();
  sum_b.clear();
  counts.clear();
  voxel_indices.clear();
}

// 实现 key_hash 的哈希函数
template<typename DataType>
std::size_t voxel_grid_downsampling_t<DataType>::key_hash::operator()(
    const voxel_key_t& key) const noexcept
{
  // 对于整数键，直接使用标准哈希函数
  return std::hash<voxel_key_t> {}(key);
}

// 实现计算点云边界的方法
template<typename DataType>
void voxel_grid_downsampling_t<DataType>::compute_point_cloud_bounds()
{
  if (!m_cloud || m_cloud->empty()) {
    m_bounds_computed = false;
    return;
  }

  // 使用现有的 minmax 工具计算点云边界
  constexpr std::size_t k_parallel_threshold = 1024;

  // 计算点云边界
  auto bounds = m_enable_parallel && m_cloud->size() > k_parallel_threshold
      ? toolbox::types::calculate_minmax_parallel(*m_cloud)
      : toolbox::types::calculate_minmax(*m_cloud);

  // 计算体素索引范围
  m_min_ix = static_cast<int>(std::floor(bounds.min.x / m_voxel_size));
  m_min_iy = static_cast<int>(std::floor(bounds.min.y / m_voxel_size));
  m_min_iz = static_cast<int>(std::floor(bounds.min.z / m_voxel_size));

  m_max_ix = static_cast<int>(std::floor(bounds.max.x / m_voxel_size));
  m_max_iy = static_cast<int>(std::floor(bounds.max.y / m_voxel_size));
  m_max_iz = static_cast<int>(std::floor(bounds.max.z / m_voxel_size));

  // 计算跨度用于键值计算
  m_span_x = m_max_ix - m_min_ix + 1;
  m_span_y = m_max_iy - m_min_iy + 1;

  m_bounds_computed = true;
}

// 实现计算体素键值的方法
template<typename DataType>
typename voxel_grid_downsampling_t<DataType>::voxel_key_t
voxel_grid_downsampling_t<DataType>::compute_voxel_key(int ix,
                                                       int iy,
                                                       int iz) const
{
  // 确保边界已计算
  if (!m_bounds_computed) {
    // 如果边界未计算，使用简单的哈希方法
    // 这不是最优的，但可以作为后备方案
    constexpr std::uint64_t kBitMask = 0x1FFFFF;  // 21位掩码
    constexpr int kYShift = 21;
    constexpr int kZShift = 42;

    // 确保坐标在合理范围内
    const std::uint64_t x_bits =
        static_cast<std::uint64_t>(static_cast<std::uint32_t>(ix)) & kBitMask;
    const std::uint64_t y_bits =
        static_cast<std::uint64_t>(static_cast<std::uint32_t>(iy)) & kBitMask;
    const std::uint64_t z_bits =
        static_cast<std::uint64_t>(static_cast<std::uint32_t>(iz)) & kBitMask;

    return (z_bits << static_cast<unsigned>(kZShift))
        | (y_bits << static_cast<unsigned>(kYShift)) | x_bits;
  }

  // 使用点云边界计算相对坐标
  const int rel_x = ix - m_min_ix;
  const int rel_y = iy - m_min_iy;
  const int rel_z = iz - m_min_iz;

  // 计算一维索引
  return (static_cast<std::uint64_t>(rel_z)
          * static_cast<std::uint64_t>(m_span_y)
          * static_cast<std::uint64_t>(m_span_x))
      + (static_cast<std::uint64_t>(rel_y)
         * static_cast<std::uint64_t>(m_span_x))
      + static_cast<std::uint64_t>(rel_x);
}

// 实现估计体素数量的方法
template<typename DataType>
std::size_t voxel_grid_downsampling_t<DataType>::estimate_voxel_count() const
{
  if (!m_bounds_computed || !m_cloud || m_cloud->empty()) {
    // 如果边界未计算或点云为空，使用默认估计
    constexpr int kDefaultDivisor = 10;  // 假设平均每10个点一个体素
    return m_cloud ? m_cloud->size() / kDefaultDivisor : 0;
  }

  // 基于点云边界估计体素数量
  const std::size_t total_voxels = static_cast<std::size_t>(m_span_x)
      * static_cast<std::size_t>(m_span_y)
      * static_cast<std::size_t>(m_max_iz - m_min_iz + 1);

  // 假设点云不是均匀分布的，实际体素数量会小于总体素数量
  // 使用一个经验系数来估计
  const double fill_factor = 0.1;  // 假设只有10%的体素包含点

  return std::min(static_cast<std::size_t>(total_voxels * fill_factor),
                  m_cloud->size());
}

// 实现 process_point 方法
template<typename DataType>
void voxel_grid_downsampling_t<DataType>::process_point(
    std::size_t idx,
    std::unordered_map<voxel_key_t, std::size_t, key_hash>& voxel_map,
    voxel_data_soa_t& voxel_data)
{
  const auto& point = m_cloud->points[idx];
  int voxel_x = static_cast<int>(std::floor(point.x / m_voxel_size));
  int voxel_y = static_cast<int>(std::floor(point.y / m_voxel_size));
  int voxel_z = static_cast<int>(std::floor(point.z / m_voxel_size));
  voxel_key_t key = compute_voxel_key(voxel_x, voxel_y, voxel_z);

  // 查找或创建体素
  std::size_t voxel_idx = 0;
  auto iter = voxel_map.find(key);
  if (iter == voxel_map.end()) {
    voxel_idx = voxel_data.add_voxel();
    voxel_map[key] = voxel_idx;
  } else {
    voxel_idx = iter->second;
  }

  // 累加点数据
  voxel_data.sum_x[voxel_idx] += point.x;
  voxel_data.sum_y[voxel_idx] += point.y;
  voxel_data.sum_z[voxel_idx] += point.z;

  // 如果有法线，累加法线数据
  if (!m_cloud->normals.empty()) {
    const auto& normal = m_cloud->normals[idx];
    voxel_data.sum_nx[voxel_idx] += normal.x;
    voxel_data.sum_ny[voxel_idx] += normal.y;
    voxel_data.sum_nz[voxel_idx] += normal.z;
  }

  // 如果有颜色，累加颜色数据
  if (!m_cloud->colors.empty()) {
    const auto& color = m_cloud->colors[idx];
    voxel_data.sum_r[voxel_idx] += color.x;
    voxel_data.sum_g[voxel_idx] += color.y;
    voxel_data.sum_b[voxel_idx] += color.z;
  }

  // 增加计数
  voxel_data.counts[voxel_idx]++;
}

// 实现 merge_thread_data 方法
template<typename DataType>
void voxel_grid_downsampling_t<DataType>::merge_thread_data(
    const std::vector<std::unordered_map<voxel_key_t, std::size_t, key_hash>>&
        thread_maps,
    const std::vector<voxel_data_soa_t>& thread_data,
    std::unordered_map<voxel_key_t, std::size_t, key_hash>& merged_map,
    voxel_data_soa_t& merged_data)
{
  const bool has_normals = !m_cloud->normals.empty();
  const bool has_colors = !m_cloud->colors.empty();

  // 遍历所有线程的数据
  for (std::size_t thread_id = 0; thread_id < thread_maps.size(); ++thread_id) {
    const auto& thread_map = thread_maps[thread_id];
    const auto& thread_voxel_data = thread_data[thread_id];

    // 遍历该线程的所有体素
    for (const auto& [key, thread_voxel_idx] : thread_map) {
      std::size_t merged_voxel_idx = 0;
      auto iter = merged_map.find(key);

      if (iter == merged_map.end()) {
        // 创建新体素
        merged_voxel_idx = merged_data.add_voxel();
        merged_map[key] = merged_voxel_idx;

        // 复制数据
        merged_data.sum_x[merged_voxel_idx] =
            thread_voxel_data.sum_x[thread_voxel_idx];
        merged_data.sum_y[merged_voxel_idx] =
            thread_voxel_data.sum_y[thread_voxel_idx];
        merged_data.sum_z[merged_voxel_idx] =
            thread_voxel_data.sum_z[thread_voxel_idx];

        if (has_normals) {
          merged_data.sum_nx[merged_voxel_idx] =
              thread_voxel_data.sum_nx[thread_voxel_idx];
          merged_data.sum_ny[merged_voxel_idx] =
              thread_voxel_data.sum_ny[thread_voxel_idx];
          merged_data.sum_nz[merged_voxel_idx] =
              thread_voxel_data.sum_nz[thread_voxel_idx];
        }

        if (has_colors) {
          merged_data.sum_r[merged_voxel_idx] =
              thread_voxel_data.sum_r[thread_voxel_idx];
          merged_data.sum_g[merged_voxel_idx] =
              thread_voxel_data.sum_g[thread_voxel_idx];
          merged_data.sum_b[merged_voxel_idx] =
              thread_voxel_data.sum_b[thread_voxel_idx];
        }

        merged_data.counts[merged_voxel_idx] =
            thread_voxel_data.counts[thread_voxel_idx];
      } else {
        // 合并到现有体素
        merged_voxel_idx = iter->second;

        // 累加数据
        merged_data.sum_x[merged_voxel_idx] +=
            thread_voxel_data.sum_x[thread_voxel_idx];
        merged_data.sum_y[merged_voxel_idx] +=
            thread_voxel_data.sum_y[thread_voxel_idx];
        merged_data.sum_z[merged_voxel_idx] +=
            thread_voxel_data.sum_z[thread_voxel_idx];

        if (has_normals) {
          merged_data.sum_nx[merged_voxel_idx] +=
              thread_voxel_data.sum_nx[thread_voxel_idx];
          merged_data.sum_ny[merged_voxel_idx] +=
              thread_voxel_data.sum_ny[thread_voxel_idx];
          merged_data.sum_nz[merged_voxel_idx] +=
              thread_voxel_data.sum_nz[thread_voxel_idx];
        }

        if (has_colors) {
          merged_data.sum_r[merged_voxel_idx] +=
              thread_voxel_data.sum_r[thread_voxel_idx];
          merged_data.sum_g[merged_voxel_idx] +=
              thread_voxel_data.sum_g[thread_voxel_idx];
          merged_data.sum_b[merged_voxel_idx] +=
              thread_voxel_data.sum_b[thread_voxel_idx];
        }

        merged_data.counts[merged_voxel_idx] +=
            thread_voxel_data.counts[thread_voxel_idx];
      }
    }
  }
}

template<typename DataType>
std::size_t voxel_grid_downsampling_t<DataType>::set_input_impl(
    const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);

  // 计算点云边界，用于优化体素索引计算
  m_bounds_computed = false;
  if (!m_cloud->empty()) {
    compute_point_cloud_bounds();
  }

  return m_cloud->size();
}

template<typename DataType>
std::size_t voxel_grid_downsampling_t<DataType>::set_input_impl(
    const point_cloud_ptr& cloud)
{
  m_cloud = cloud;

  // 计算点云边界，用于优化体素索引计算
  m_bounds_computed = false;
  if (m_cloud && !m_cloud->empty()) {
    compute_point_cloud_bounds();
  }

  return m_cloud ? m_cloud->size() : 0U;
}

template<typename DataType>
void voxel_grid_downsampling_t<DataType>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

template<typename DataType>
typename voxel_grid_downsampling_t<DataType>::point_cloud
voxel_grid_downsampling_t<DataType>::filter_impl()
{
  auto output = std::make_shared<point_cloud>();
  filter_impl(output);
  return *output;
}

template<typename DataType>
void voxel_grid_downsampling_t<DataType>::filter_impl(point_cloud_ptr output)
{
  if (!output) {
    return;
  }
  if (!m_cloud || m_cloud->empty()) {
    output->clear();
    return;
  }

  // 定义常量
  constexpr std::size_t k_parallel_threshold = 1024;
  constexpr std::size_t kMaxVoxelsPerThread = 1000;

  const std::size_t total_points = m_cloud->size();
  const bool has_normals = !m_cloud->normals.empty();
  const bool has_colors = !m_cloud->colors.empty();

  // 确定线程数量
  const std::size_t num_threads =
      m_enable_parallel && total_points > k_parallel_threshold
      ? std::thread::hardware_concurrency()
      : 1;

  // 预估每个线程的体素数量 - 假设点云均匀分布，每个体素包含多个点
  constexpr int kDefaultDivisor = 10;  // 假设平均每10个点一个体素
  const std::size_t estimated_voxels_per_thread =
      std::min(total_points / kDefaultDivisor, kMaxVoxelsPerThread);

  // 创建线程局部哈希表和数据结构
  std::vector<std::unordered_map<voxel_key_t, std::size_t, key_hash>>
      thread_voxel_maps(num_threads);
  std::vector<voxel_data_soa_t> thread_voxel_data(num_threads);

  // 为每个线程预分配内存
  for (auto& voxel_data : thread_voxel_data) {
    voxel_data.reserve(estimated_voxels_per_thread);
  }

  // 并行或串行处理点云
  if (m_enable_parallel && total_points > k_parallel_threshold) {
    // 计算每个线程处理的点数
    const std::size_t points_per_thread =
        (total_points + num_threads - 1) / num_threads;

    // 创建并行任务
    std::vector<std::future<void>> futures;
    futures.reserve(num_threads);

    for (std::size_t thread_id = 0; thread_id < num_threads; ++thread_id) {
      // 计算每个线程处理的点范围
      const std::size_t start_idx = thread_id * points_per_thread;
      const std::size_t end_idx =
          std::min(start_idx + points_per_thread, total_points);

      if (start_idx >= end_idx) {
        continue;
      }

      futures.emplace_back(toolbox::concurrent::default_pool().submit(
          [this,
           thread_id,
           start_idx,
           end_idx,
           has_normals,
           has_colors,
           &thread_voxel_maps,
           &thread_voxel_data]()
          {
            auto& voxel_map = thread_voxel_maps[thread_id];
            auto& voxel_data = thread_voxel_data[thread_id];

            // 处理该线程负责的点
            for (std::size_t i = start_idx; i < end_idx; ++i) {
              process_point(i, voxel_map, voxel_data);
            }
          }));
    }

    // 等待所有线程完成
    for (auto& future : futures) {
      future.get();
    }
  } else {
    // 单线程处理
    auto& voxel_map = thread_voxel_maps[0];
    auto& voxel_data = thread_voxel_data[0];

    for (std::size_t i = 0; i < total_points; ++i) {
      process_point(i, voxel_map, voxel_data);
    }
  }

  // 合并所有线程的结果
  std::unordered_map<voxel_key_t, std::size_t, key_hash> merged_voxel_map;
  voxel_data_soa_t merged_voxel_data;

  // 估计合并后的体素数量
  std::size_t total_voxels = 0;
  for (const auto& voxel_data : thread_voxel_data) {
    total_voxels += voxel_data.size();
  }
  merged_voxel_data.reserve(total_voxels);

  // 合并线程数据
  merge_thread_data(thread_voxel_maps,
                    thread_voxel_data,
                    merged_voxel_map,
                    merged_voxel_data);

  // 生成输出点云
  const std::size_t num_voxels = merged_voxel_data.size();
  output->points.resize(num_voxels);
  if (has_normals) {
    output->normals.resize(num_voxels);
  }
  if (has_colors) {
    output->colors.resize(num_voxels);
  }
  output->intensity = m_cloud->intensity;

  // 并行或串行计算质心
  if (m_enable_parallel && num_voxels > k_parallel_threshold) {
    std::vector<std::future<void>> futures;
    futures.reserve(num_threads);

    const std::size_t voxels_per_thread =
        (num_voxels + num_threads - 1) / num_threads;

    for (std::size_t thread_id = 0; thread_id < num_threads; ++thread_id) {
      const std::size_t start_idx = thread_id * voxels_per_thread;
      const std::size_t end_idx =
          std::min(start_idx + voxels_per_thread, num_voxels);

      if (start_idx >= end_idx) {
        continue;
      }

      futures.emplace_back(toolbox::concurrent::default_pool().submit(
          [start_idx,
           end_idx,
           has_normals,
           has_colors,
           &merged_voxel_data,
           &output]()
          {
            for (std::size_t i = start_idx; i < end_idx; ++i) {
              const DataType inv_count =
                  static_cast<DataType>(1.0) / merged_voxel_data.counts[i];

              // 计算质心
              output->points[i].x = merged_voxel_data.sum_x[i] * inv_count;
              output->points[i].y = merged_voxel_data.sum_y[i] * inv_count;
              output->points[i].z = merged_voxel_data.sum_z[i] * inv_count;

              // 计算法线
              if (has_normals) {
                output->normals[i].x = merged_voxel_data.sum_nx[i] * inv_count;
                output->normals[i].y = merged_voxel_data.sum_ny[i] * inv_count;
                output->normals[i].z = merged_voxel_data.sum_nz[i] * inv_count;
              }

              // 计算颜色
              if (has_colors) {
                output->colors[i].x = merged_voxel_data.sum_r[i] * inv_count;
                output->colors[i].y = merged_voxel_data.sum_g[i] * inv_count;
                output->colors[i].z = merged_voxel_data.sum_b[i] * inv_count;
              }
            }
          }));
    }

    // 等待所有线程完成
    for (auto& future : futures) {
      future.get();
    }
  } else {
    // 单线程计算质心
    for (std::size_t i = 0; i < num_voxels; ++i) {
      const DataType inv_count =
          static_cast<DataType>(1.0) / merged_voxel_data.counts[i];

      // 计算质心
      output->points[i].x = merged_voxel_data.sum_x[i] * inv_count;
      output->points[i].y = merged_voxel_data.sum_y[i] * inv_count;
      output->points[i].z = merged_voxel_data.sum_z[i] * inv_count;

      // 计算法线
      if (has_normals) {
        output->normals[i].x = merged_voxel_data.sum_nx[i] * inv_count;
        output->normals[i].y = merged_voxel_data.sum_ny[i] * inv_count;
        output->normals[i].z = merged_voxel_data.sum_nz[i] * inv_count;
      }

      // 计算颜色
      if (has_colors) {
        output->colors[i].x = merged_voxel_data.sum_r[i] * inv_count;
        output->colors[i].y = merged_voxel_data.sum_g[i] * inv_count;
        output->colors[i].z = merged_voxel_data.sum_b[i] * inv_count;
      }
    }
  }
}

}  // namespace toolbox::pcl
