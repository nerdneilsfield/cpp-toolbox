#pragma once

#include <algorithm>
#include <numeric>
#include <vector>

#include <cpp-toolbox/concurrent/parallel.hpp>

namespace toolbox::pcl
{

template<typename DataType>
std::size_t random_downsampling_t<DataType>::set_input_impl(
    const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType>
std::size_t random_downsampling_t<DataType>::set_input_impl(
    const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  return m_cloud ? m_cloud->size() : 0U;
}

template<typename DataType>
void random_downsampling_t<DataType>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

template<typename DataType>
typename random_downsampling_t<DataType>::point_cloud
random_downsampling_t<DataType>::filter_impl()
{
  auto output = std::make_shared<point_cloud>();
  filter_impl(output);
  return *output;
}

template<typename DataType>
void random_downsampling_t<DataType>::filter_impl(point_cloud_ptr output)
{
  if (!output)
    return;
  if (!m_cloud || m_cloud->empty()) {
    output->clear();
    return;
  }

  const std::size_t input_size = m_cloud->size();
  std::size_t sample_count =
      static_cast<std::size_t>(std::floor(input_size * m_ration));
  sample_count = std::min(sample_count, input_size);
  if (sample_count == 0) {
    output->clear();
    return;
  }

  // 使用蓄水池采样算法(Reservoir Sampling)代替全数组洗牌
  std::vector<std::size_t> indices;
  indices.reserve(sample_count);

  // 如果采样率很高或点云较小，使用传统方法可能更快
  if (sample_count > input_size / 2 || input_size < 10000) {
    // 传统方法：生成所有索引并洗牌
    indices.resize(input_size);
    std::iota(indices.begin(), indices.end(), 0);
    toolbox::utils::random_t::instance().shuffle(indices);
    indices.resize(sample_count);
  } else {
    // 蓄水池采样算法
    auto& rng = toolbox::utils::random_t::instance();

    // 第1步：填充初始蓄水池
    indices.resize(sample_count);
    for (std::size_t i = 0; i < sample_count; ++i) {
      indices[i] = i;
    }

    // 第2步：处理剩余元素
    for (std::size_t i = sample_count; i < input_size; ++i) {
      // 以 k/i 的概率选择第i个元素
      std::size_t j = rng.random_int<std::size_t>(0, i);
      if (j < sample_count) {
        indices[j] = i;
      }
    }

    // 可选：打乱结果以避免顺序偏差
    if (sample_count > 1) {
      rng.shuffle(indices);
    }
  }

  // 预分配输出点云内存
  output->points.resize(sample_count);
  if (!m_cloud->normals.empty()) {
    output->normals.resize(sample_count);
  }
  if (!m_cloud->colors.empty()) {
    output->colors.resize(sample_count);
  }
  output->intensity = m_cloud->intensity;

  // 并行处理逻辑保持不变
  if (m_enable_parallel && sample_count > 1024) {
    toolbox::concurrent::parallel_transform(indices.cbegin(),
                                            indices.cend(),
                                            output->points.begin(),
                                            [this](std::size_t idx)
                                            { return m_cloud->points[idx]; });
    if (!m_cloud->normals.empty()) {
      toolbox::concurrent::parallel_transform(
          indices.cbegin(),
          indices.cend(),
          output->normals.begin(),
          [this](std::size_t idx) { return m_cloud->normals[idx]; });
    }
    if (!m_cloud->colors.empty()) {
      toolbox::concurrent::parallel_transform(indices.cbegin(),
                                              indices.cend(),
                                              output->colors.begin(),
                                              [this](std::size_t idx)
                                              { return m_cloud->colors[idx]; });
    }
  } else {
    for (std::size_t i = 0; i < sample_count; ++i) {
      std::size_t idx = indices[i];
      output->points[i] = m_cloud->points[idx];
      if (!m_cloud->normals.empty()) {
        output->normals[i] = m_cloud->normals[idx];
      }
      if (!m_cloud->colors.empty()) {
        output->colors[i] = m_cloud->colors[idx];
      }
    }
  }
}

}  // namespace toolbox::pcl
