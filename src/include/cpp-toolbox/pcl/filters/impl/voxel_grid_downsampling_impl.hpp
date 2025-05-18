#pragma once

#include <cmath>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <cpp-toolbox/concurrent/parallel.hpp>

namespace toolbox::pcl
{

template<typename DataType>
std::size_t voxel_grid_downsampling_t<DataType>::set_input_impl(
    const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType>
std::size_t voxel_grid_downsampling_t<DataType>::set_input_impl(
    const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
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
  if (!output)
    return;
  if (!m_cloud || m_cloud->empty()) {
    output->clear();
    return;
  }

  struct voxel_data_t
  {
    toolbox::types::point_t<DataType> sum_point {0, 0, 0};
    toolbox::types::point_t<DataType> sum_normal {0, 0, 0};
    toolbox::types::point_t<DataType> sum_color {0, 0, 0};
    std::size_t count {0};
  };

  using key_t = std::tuple<int, int, int>;
  struct key_hash
  {
    std::size_t operator()(const key_t& k) const noexcept
    {
      std::size_t h1 = std::hash<int> {}(std::get<0>(k));
      std::size_t h2 = std::hash<int> {}(std::get<1>(k));
      std::size_t h3 = std::hash<int> {}(std::get<2>(k));
      return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
  };

  std::unordered_map<key_t, voxel_data_t, key_hash> voxel_map;
  std::mutex map_mutex;
  auto process_point = [&](std::size_t idx)
  {
    const auto& pt = m_cloud->points[idx];
    int ix = static_cast<int>(std::floor(pt.x / m_voxel_size));
    int iy = static_cast<int>(std::floor(pt.y / m_voxel_size));
    int iz = static_cast<int>(std::floor(pt.z / m_voxel_size));
    key_t key(ix, iy, iz);
    std::lock_guard<std::mutex> lock(map_mutex);
    auto& v = voxel_map[key];
    v.sum_point += pt;
    if (!m_cloud->normals.empty()) {
      v.sum_normal += m_cloud->normals[idx];
    }
    if (!m_cloud->colors.empty()) {
      v.sum_color += m_cloud->colors[idx];
    }
    ++v.count;
  };

  const std::size_t total = m_cloud->size();
  if (m_enable_parallel && total > 1024) {
    std::vector<std::size_t> indices(total);
    std::iota(indices.begin(), indices.end(), 0);
    toolbox::concurrent::parallel_for_each(
        indices.begin(), indices.end(), process_point);
  } else {
    for (std::size_t i = 0; i < total; ++i) {
      process_point(i);
    }
  }

  const bool has_normals = !m_cloud->normals.empty();
  const bool has_colors = !m_cloud->colors.empty();
  output->points.reserve(voxel_map.size());
  if (has_normals) {
    output->normals.reserve(voxel_map.size());
  }
  if (has_colors) {
    output->colors.reserve(voxel_map.size());
  }
  output->intensity = m_cloud->intensity;

  for (auto& [key, v] : voxel_map) {
    auto centroid = v.sum_point;
    centroid /= static_cast<DataType>(v.count);
    output->points.push_back(centroid);
    if (has_normals) {
      auto n = v.sum_normal;
      n /= static_cast<DataType>(v.count);
      output->normals.push_back(n);
    }
    if (has_colors) {
      auto c = v.sum_color;
      c /= static_cast<DataType>(v.count);
      output->colors.push_back(c);
    }
  }
}

}  // namespace toolbox::pcl
