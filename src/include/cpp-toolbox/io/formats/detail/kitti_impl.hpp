#pragma once

/**
 * @file kitti_impl.hpp
 * @brief 此文件包含 kitti_format_t 的模板实现/This file contains the template
 * implementations for kitti_format_t
 * @details 该文件应该只被 kitti.hpp 包含/It should only be included by kitti.hpp
 */

#include <cpp-toolbox/io/formats/kitti.hpp>  // Include the header declaring the class

// Add necessary includes for the implementations below
#include <algorithm>  // std::clamp
#include <cmath>  // std::is_floating_point_v, std::is_integral_v
#include <cstdint>  // INT64_C
#include <cstring>  // memcpy
#include <fstream>  // ofstream (for writing), istream
#include <iomanip>  // setprecision, fixed
#include <limits>  // numeric_limits
#include <memory>
#include <stdexcept>  // exceptions for parsing
#include <type_traits>  // is_same_v, decay_t
#include <vector>

#include <cpp-toolbox/file/memory_mapped_file.hpp>  // For reading binary
#include <cpp-toolbox/logger/thread_logger.hpp>  // For LOG_*
// #include <cpp-toolbox/types/point.hpp> // Should be included via kitti.hpp

namespace toolbox::io::formats
{

// Assuming kitti.hpp includes this file *after* the full class definition.

/**
 * @brief 将点云数据写入KITTI文件的内部实现/Internal implementation for writing
 * point cloud data to a KITTI file
 *
 * @tparam T 点云数据的数值类型/The numeric type of the point cloud data
 * @param path 输出文件路径/Output file path
 * @param cloud 要写入的点云数据/The point cloud to write
 * @param binary 是否以二进制格式写入（KITTI格式总是二进制）/Whether to write in binary format (KITTI format is always binary)
 * @return bool 写入操作是否成功/Whether the write operation was successful
 *
 * @code
 * // 写入浮点型点云到KITTI格式文件
 * point_cloud_t<float> cloud;
 * // ... 填充点云数据 ...
 * kitti_format_t kitti;
 * bool success = kitti.write_internal("output.bin", cloud, true); // 二进制格式
 *
 * // 写入双精度点云到KITTI格式文件
 * point_cloud_t<double> cloud_double;
 * // ... 填充点云数据 ...
 * bool binary_success = kitti.write_internal("output.bin", cloud_double, true); // 二进制格式
 * @endcode
 */
template<typename T>
bool kitti_format_t::write_internal(const std::string& path,
                                  const point_cloud_t<T>& cloud,
                                  bool binary) const
{
  // KITTI格式总是二进制，忽略binary参数
  // KITTI format is always binary, ignore binary parameter
  LOG_DEBUG_S << "kitti_format_t::write_internal processing " << cloud.size()
              << " points for " << path;

  std::ofstream file;
  std::ios_base::openmode mode = std::ios::trunc | std::ios::binary;
  file.open(path, mode);

  if (!file.is_open()) {
    LOG_ERROR_S << "kitti_format_t: Failed to open file for writing: " << path;
    return false;
  }

  // 检查点云是否为空
  // Check if point cloud is empty
  if (cloud.empty()) {
    LOG_WARN_S << "kitti_format_t: Writing empty point cloud to " << path;
    return file.good();  // 成功写入空文件/Successfully wrote empty file
  }

  // KITTI格式：每个点由4个float组成 (x, y, z, intensity)
  // KITTI format: each point consists of 4 floats (x, y, z, intensity)
  const size_t point_step = 4 * sizeof(float);  // 每个点16字节/16 bytes per point
  std::vector<unsigned char> point_buffer(point_step);

  for (size_t i = 0; i < cloud.size(); ++i) {
    unsigned char* current_ptr = point_buffer.data();

    // 写入XYZ坐标/Write XYZ coordinates
    float x_val = static_cast<float>(cloud.points[i].x);
    float y_val = static_cast<float>(cloud.points[i].y);
    float z_val = static_cast<float>(cloud.points[i].z);

    // 写入强度值（如果没有，使用默认值0.0f）
    // Write intensity value (use default 0.0f if not available)
    float intensity_val = 0.0f;

    std::memcpy(current_ptr, &x_val, sizeof(float));
    current_ptr += sizeof(float);
    std::memcpy(current_ptr, &y_val, sizeof(float));
    current_ptr += sizeof(float);
    std::memcpy(current_ptr, &z_val, sizeof(float));
    current_ptr += sizeof(float);
    std::memcpy(current_ptr, &intensity_val, sizeof(float));

    file.write(reinterpret_cast<const char*>(point_buffer.data()),
               point_buffer.size());
    if (!file) {
      LOG_ERROR_S << "kitti_format_t: Error writing binary data point " << i
                  << " to " << path;
      return false;
    }
  }

  return file.good();
}

/**
 * @brief 从KITTI二进制文件读取点云数据/Read point cloud data from KITTI binary file
 *
 * @tparam T 点云数据的数值类型/The numeric type of the point cloud data
 * @param path 输入文件路径/Input file path
 * @param cloud 输出点云数据/Output point cloud
 * @return bool 读取操作是否成功/Whether the read operation was successful
 *
 * @code
 * // 从KITTI二进制文件读取点云
 * point_cloud_t<float> cloud;
 * kitti_format_t kitti;
 * bool success = kitti.read_binary_data("input.bin", cloud);
 * @endcode
 */
template<typename T>
bool kitti_format_t::read_binary_data(const std::string& path,
                                    point_cloud_t<T>& cloud)
{
  toolbox::file::memory_mapped_file_t mapped_file;
  if (!mapped_file.open(path)) {
    LOG_ERROR_S << "kitti_format_t: Failed to memory map file: " << path;
    return false;
  }

  const unsigned char* buffer_start = mapped_file.data();
  if (!buffer_start) {
    LOG_ERROR_S << "kitti_format_t: Memory mapped buffer is null for: " << path;
    return false;
  }

  const unsigned char* buffer_end = buffer_start + mapped_file.size();

  // 计算点的数量：文件大小除以每个点的大小（4个float = 16字节）
  // Calculate number of points: file size divided by size per point (4 floats = 16 bytes)
  const size_t point_step = 4 * sizeof(float);  // 每个点16字节/16 bytes per point
  const size_t num_points = mapped_file.size() / point_step;

  if (mapped_file.size() % point_step != 0) {
    LOG_WARN_S << "kitti_format_t: File size " << mapped_file.size()
               << " is not a multiple of point size " << point_step
               << ". Some data may be truncated.";
  }

  if (num_points == 0) {
    LOG_WARN_S << "kitti_format_t: No points found in file: " << path;
    return true;  // 成功读取了空文件/Successfully read empty file
  }

  // 预分配内存
  // Pre-allocate memory
  cloud.points.reserve(num_points);

  // 读取每个点的数据
  // Read data for each point
  for (size_t i = 0; i < num_points; ++i) {
    const unsigned char* current_point_ptr = buffer_start + i * point_step;

    // 确保不会读取超出缓冲区的数据
    // Ensure we don't read past the buffer boundary
    if (current_point_ptr + point_step > buffer_end) {
      LOG_ERROR_S << "kitti_format_t: Attempt to read past mapped buffer "
                  << "boundary at point index " << i;
      return false;
    }

    // 读取XYZ和强度值
    // Read XYZ and intensity values
    float x_val, y_val, z_val, intensity_val;
    std::memcpy(&x_val, current_point_ptr, sizeof(float));
    std::memcpy(&y_val, current_point_ptr + sizeof(float), sizeof(float));
    std::memcpy(&z_val, current_point_ptr + 2 * sizeof(float), sizeof(float));
    std::memcpy(&intensity_val, current_point_ptr + 3 * sizeof(float), sizeof(float));

    // 添加点到点云
    // Add point to point cloud
    point_t<T> current_point;
    current_point.x = static_cast<T>(x_val);
    current_point.y = static_cast<T>(y_val);
    current_point.z = static_cast<T>(z_val);
    cloud.points.push_back(current_point);

    // 设置强度值（如果需要）
    // Set intensity value (if needed)
    cloud.intensity = static_cast<T>(intensity_val);
  }

  LOG_DEBUG_S << "kitti_format_t: Successfully read " << cloud.size()
              << " points from " << path;

  return true;
}

template<typename T>
CPP_TOOLBOX_EXPORT std::unique_ptr<toolbox::types::point_cloud_t<T>> read_kitti_bin(
    const std::string& path)
{
  auto kitti = std::make_unique<toolbox::io::formats::kitti_format_t>();
  std::unique_ptr<base_file_data_t> base_data = nullptr;  // Create ptr of the type expected by read()

  // Call read, passing the base class unique_ptr reference
  bool success = kitti->read(path, base_data);

  if (success && base_data) {
    // Read succeeded, now check if the actual data type matches T
    auto* typed_ptr = dynamic_cast<point_cloud_t<T>*>(base_data.get());
    if (typed_ptr) {
      // Type matches T, transfer ownership to a new unique_ptr<point_cloud_t<T>>
      base_data.release();  // Prevent base_data from deleting the object
      return std::unique_ptr<point_cloud_t<T>>(typed_ptr);
    } else {
      // Read succeeded, but the resulting type is not point_cloud_t<T>
      // (e.g., read always returns float, but T was double)
      LOG_WARN_S << "read_kitti_bin: read data type does not match requested type T="
                 << typeid(T).name() << ". Path: " << path;
      // Return nullptr as the requested type couldn't be provided.
      // The base_data unique_ptr will delete the object when it goes out of scope.
      return nullptr;
    }
  }

  // Read failed or returned null data
  return nullptr;
}

template<typename T>
CPP_TOOLBOX_EXPORT bool write_kitti_bin(const std::string& path,
                                      const toolbox::types::point_cloud_t<T>& cloud)
{
  auto kitti = std::make_unique<toolbox::io::formats::kitti_format_t>();

  // Create a copy of the input cloud to satisfy the unique_ptr interface
  auto cloud_copy_ptr = std::make_unique<toolbox::types::point_cloud_t<T>>(cloud);

  // Cast the unique_ptr holding the copy to the base class type unique_ptr
  std::unique_ptr<base_file_data_t> base_data_ptr = std::move(cloud_copy_ptr);

  // Pass the unique_ptr (as const reference) to the write method
  // KITTI格式总是二进制/KITTI format is always binary
  return kitti->write(path, base_data_ptr, true);
}

}  // namespace toolbox::io::formats
