#pragma once

/**
 * @file pcd_impl.hpp
 * @brief 此文件包含 pcd_format_t 的模板实现/This file contains the template
 * implementations for pcd_format_t
 * @details 该文件应该只被 pcd.hpp 包含/It should only be included by pcd.hpp
 */

#include <cpp-toolbox/io/formats/pcd.hpp>  // Include the header declaring the class

// Add necessary includes for the implementations below
#include <algorithm>  // std::clamp
#include <cmath>  // std::is_floating_point_v, std::is_integral_v
#include <cstdint>  // INT64_C
#include <cstring>  // memcpy
#include <fstream>  // ofstream (for writing), istream
#include <iomanip>  // setprecision, fixed
#include <limits>  // numeric_limits
#include <map>
#include <memory>
#include <sstream>  // stringstream
#include <stdexcept>  // exceptions for parsing
#include <type_traits>  // is_same_v, decay_t
#include <variant>  // variant for ascii parsing
#include <vector>

#include <cpp-toolbox/file/memory_mapped_file.hpp>  // For reading binary
#include <cpp-toolbox/logger/thread_logger.hpp>  // For LOG_*
// #include <cpp-toolbox/types/point.hpp> // Should be included via pcd.hpp

namespace toolbox::io::formats
{

// Assuming pcd.hpp includes this file *after* the full class definition.

/**
 * @brief 将点云数据写入PCD文件的内部实现/Internal implementation for writing
 * point cloud data to a PCD file
 *
 * @tparam T 点云数据的数值类型/The numeric type of the point cloud data
 * @param path 输出文件路径/Output file path
 * @param cloud 要写入的点云数据/The point cloud to write
 * @param binary 是否以二进制格式写入/Whether to write in binary format
 * @return bool 写入操作是否成功/Whether the write operation was successful
 *
 * @code
 * // 写入浮点型点云到ASCII格式PCD文件
 * point_cloud_t<float> cloud;
 * // ... 填充点云数据 ...
 * pcd_format_t pcd;
 * bool success = pcd.write_internal("output.pcd", cloud, false); // ASCII格式
 *
 * // 写入双精度点云到二进制格式PCD文件
 * point_cloud_t<double> cloud_double;
 * // ... 填充点云数据 ...
 * bool binary_success = pcd.write_internal("output_binary.pcd", cloud_double,
 * true); // 二进制格式
 * @endcode
 */
template<typename T>
bool pcd_format_t::write_internal(const std::string& path,
                                  const point_cloud_t<T>& cloud,
                                  bool binary) const
{
  LOG_DEBUG_S << "pcd_format_t::write_internal processing " << cloud.size()
              << " points for " << path;

  std::ofstream file;
  std::ios_base::openmode mode = std::ios::trunc;
  if (binary) {
    mode |= std::ios::binary;
  }
  file.open(path, mode);

  if (!file.is_open()) {
    LOG_ERROR_S << "pcd_format_t: Failed to open file for writing: " << path;
    return false;
  }

  // Determine fields, types, sizes, counts based on T and cloud content
  std::vector<std::string> fields;
  std::vector<char> types;
  std::vector<size_t> sizes;
  std::vector<size_t> counts;

  // XYZ are always present if cloud is not empty (XYZ在非空点云中始终存在/XYZ
  // are always present if cloud is not empty)
  fields.push_back("x");
  fields.push_back("y");
  fields.push_back("z");
  char point_field_type = 'F';
  size_t point_field_size = 4;
  if constexpr (std::is_same_v<T, double>) {
    point_field_size = 8;
  } else if constexpr (!std::is_same_v<T, float>) {
    LOG_WARN_S << "pcd_format_t: Writing non-float/double type "
               << typeid(T).name()
               << " might require specific TYPE/SIZE adjustments.";
  }
  for (int i = 0; i < 3; ++i) {
    types.push_back(point_field_type);
    sizes.push_back(point_field_size);
    counts.push_back(1);
  }

  bool has_normals =
      !cloud.normals.empty() && cloud.normals.size() == cloud.points.size();
  if (has_normals) {
    fields.push_back("normal_x");
    fields.push_back("normal_y");
    fields.push_back("normal_z");
    for (int i = 0; i < 3; ++i) {
      types.push_back(
          point_field_type);  // Assume same type as points
                              // (假设与点相同类型/Assume same type as points)
      sizes.push_back(point_field_size);
      counts.push_back(1);
    }
  }

  bool has_colors =
      !cloud.colors.empty() && cloud.colors.size() == cloud.points.size();
  if (has_colors) {
    fields.push_back("r");
    fields.push_back("g");
    fields.push_back("b");
    for (int i = 0; i < 3; ++i) {
      types.push_back('U');
      sizes.push_back(1);
      counts.push_back(1);  // Standard uint8 for colors
                            // (颜色使用标准uint8/Standard uint8 for colors)
    }
  }

  // Write Header (写入头部/Write Header)
  file << "VERSION .7\n";
  file << "FIELDS";
  for (const auto& f : fields) {
    file << " " << f;
  }
  file << "\n";
  file << "SIZE";
  for (auto s : sizes) {
    file << " " << s;
  }
  file << "\n";
  file << "TYPE";
  for (auto t : types) {
    file << " " << t;
  }
  file << "\n";
  file << "COUNT";
  for (auto c : counts) {
    file << " " << c;
  }
  file << "\n";
  file << "WIDTH " << cloud.size() << "\n";
  file << "HEIGHT 1\n";  // Assume unorganized cloud (假设为非组织化点云/Assume
                         // unorganized cloud)
  file << "VIEWPOINT 0 0 0 1 0 0 0\n";  // Default viewpoint (默认视点/Default
                                        // viewpoint)
  file << "POINTS " << cloud.size() << "\n";
  file << "DATA " << (binary ? "binary" : "ascii") << "\n";

  // Write Data (写入数据/Write Data)
  if (cloud.empty()) {
    LOG_WARN_S << "pcd_format_t: Writing empty point cloud header to " << path;
    return file.good();  // Wrote header successfully (成功写入头部/Wrote header
                         // successfully)
  }

  if (binary) {
    size_t point_step = 0;
    for (size_t i = 0; i < fields.size(); ++i) {
      point_step += sizes[i] * counts[i];
    }
    std::vector<unsigned char> point_buffer(point_step);

    for (size_t i = 0; i < cloud.size(); ++i) {
      unsigned char* current_ptr = point_buffer.data();
      // Write XYZ (写入XYZ坐标/Write XYZ)
      std::memcpy(current_ptr, &cloud.points[i].x, point_field_size);
      current_ptr += point_field_size;
      std::memcpy(current_ptr, &cloud.points[i].y, point_field_size);
      current_ptr += point_field_size;
      std::memcpy(current_ptr, &cloud.points[i].z, point_field_size);
      current_ptr += point_field_size;
      // Write Normals (写入法线/Write Normals)
      if (has_normals) {
        std::memcpy(current_ptr, &cloud.normals[i].x, point_field_size);
        current_ptr += point_field_size;
        std::memcpy(current_ptr, &cloud.normals[i].y, point_field_size);
        current_ptr += point_field_size;
        std::memcpy(current_ptr, &cloud.normals[i].z, point_field_size);
        current_ptr += point_field_size;
      }
      // Write Colors (convert to uint8) (写入颜色（转换为uint8）/Write Colors
      // (convert to uint8))
      if (has_colors) {
        uint8_t r_val = 0, g_val = 0, b_val = 0;
        // Clamp and convert T -> uint8 (限制范围并转换T到uint8/Clamp and
        // convert T -> uint8)
        if constexpr (std::is_floating_point_v<T>) {
          r_val = static_cast<uint8_t>(
              std::clamp(cloud.colors[i].x * static_cast<T>(255.0),
                         static_cast<T>(0.0),
                         static_cast<T>(255.0)));
          g_val = static_cast<uint8_t>(
              std::clamp(cloud.colors[i].y * static_cast<T>(255.0),
                         static_cast<T>(0.0),
                         static_cast<T>(255.0)));
          b_val = static_cast<uint8_t>(
              std::clamp(cloud.colors[i].z * static_cast<T>(255.0),
                         static_cast<T>(0.0),
                         static_cast<T>(255.0)));
        } else {  // Assume integer type already in range or needs clamping
                  // (假设整数类型已在范围内或需要限制/Assume integer type
                  // already in range or needs clamping)
          r_val = static_cast<uint8_t>(
              std::clamp(static_cast<int64_t>(cloud.colors[i].x),
                         INT64_C(0),
                         INT64_C(255)));
          g_val = static_cast<uint8_t>(
              std::clamp(static_cast<int64_t>(cloud.colors[i].y),
                         INT64_C(0),
                         INT64_C(255)));
          b_val = static_cast<uint8_t>(
              std::clamp(static_cast<int64_t>(cloud.colors[i].z),
                         INT64_C(0),
                         INT64_C(255)));
        }
        std::memcpy(current_ptr, &r_val, 1);
        current_ptr += 1;
        std::memcpy(current_ptr, &g_val, 1);
        current_ptr += 1;
        std::memcpy(current_ptr, &b_val, 1);
        current_ptr += 1;
      }
      file.write(reinterpret_cast<const char*>(point_buffer.data()),
                 point_buffer.size());
      if (!file) {
        LOG_ERROR_S << "pcd_format_t: Error writing binary data point " << i
                    << " to " << path;
        return false;
      }
    }
  } else {  // ASCII
    file << std::fixed
         << std::setprecision(std::numeric_limits<T>::max_digits10);
    for (size_t i = 0; i < cloud.size(); ++i) {
      file << cloud.points[i].x << " " << cloud.points[i].y << " "
           << cloud.points[i].z;
      if (has_normals) {
        file << " " << cloud.normals[i].x << " " << cloud.normals[i].y << " "
             << cloud.normals[i].z;
      }
      if (has_colors) {
        uint8_t r_val = 0, g_val = 0, b_val = 0;
        if constexpr (std::is_floating_point_v<T>) {
          r_val = static_cast<uint8_t>(
              std::clamp(cloud.colors[i].x * static_cast<T>(255.0),
                         static_cast<T>(0.0),
                         static_cast<T>(255.0)));
          g_val = static_cast<uint8_t>(
              std::clamp(cloud.colors[i].y * static_cast<T>(255.0),
                         static_cast<T>(0.0),
                         static_cast<T>(255.0)));
          b_val = static_cast<uint8_t>(
              std::clamp(cloud.colors[i].z * static_cast<T>(255.0),
                         static_cast<T>(0.0),
                         static_cast<T>(255.0)));
        } else {
          r_val = static_cast<uint8_t>(
              std::clamp(static_cast<int64_t>(cloud.colors[i].x),
                         INT64_C(0),
                         INT64_C(255)));
          g_val = static_cast<uint8_t>(
              std::clamp(static_cast<int64_t>(cloud.colors[i].y),
                         INT64_C(0),
                         INT64_C(255)));
          b_val = static_cast<uint8_t>(
              std::clamp(static_cast<int64_t>(cloud.colors[i].z),
                         INT64_C(0),
                         INT64_C(255)));
        }
        file << " " << static_cast<int>(r_val) << " " << static_cast<int>(g_val)
             << " " << static_cast<int>(b_val);
      }
      file << "\n";
      if (!file) {
        LOG_ERROR_S << "pcd_format_t: Error writing ASCII data point " << i
                    << " to " << path;
        return false;
      }
    }
  }
  return file.good();
}

/**
 * @brief 从ASCII格式的PCD文件读取点云数据/Read point cloud data from ASCII
 * format PCD file
 *
 * @tparam T 点云数据的数值类型/The numeric type of the point cloud data
 * @param stream 输入流/Input stream
 * @param header PCD文件头信息/PCD file header information
 * @param cloud 输出点云数据/Output point cloud
 * @return bool 读取操作是否成功/Whether the read operation was successful
 *
 * @code
 * // 从ASCII格式PCD文件读取点云
 * std::ifstream file("input.pcd");
 * pcd_header_t header;
 * // ... 解析头部 ...
 * point_cloud_t<float> cloud;
 * pcd_format_t pcd;
 * bool success = pcd.read_ascii_data(file, header, cloud);
 * @endcode
 */
template<typename T>
bool pcd_format_t::read_ascii_data(std::istream& stream,
                                   const pcd_header_t& header,
                                   point_cloud_t<T>& cloud)
{
  cloud.points.reserve(header.points);
  bool has_normals = header.m_field_indices.count("normal_x")
      && header.m_field_indices.count("normal_y")
      && header.m_field_indices.count("normal_z");
  bool has_colors = header.m_field_indices.count("rgb")
      || header.m_field_indices.count("rgba")
      || (header.m_field_indices.count("r") && header.m_field_indices.count("g")
          && header.m_field_indices.count("b"));

  if (has_normals)
    cloud.normals.reserve(header.points);
  if (has_colors)
    cloud.colors.reserve(header.points);

  std::string line;
  size_t points_read = 0;
  while (points_read < header.points && std::getline(stream, line)) {
    if (line.empty() || line[0] == '#')
      continue;  // Skip empty/comment lines (跳过空行和注释行/Skip
                 // empty/comment lines)

    point_t<T> current_point;
    point_t<T> current_normal;
    point_t<T> current_color;
    point_t<T>* normal_ptr = has_normals ? &current_normal : nullptr;
    point_t<T>* color_ptr = has_colors ? &current_color : nullptr;

    if (parse_ascii_point_line(
            line, header, current_point, normal_ptr, color_ptr))
    {
      cloud.points.push_back(current_point);
      if (has_normals)
        cloud.normals.push_back(current_normal);
      if (has_colors)
        cloud.colors.push_back(current_color);
      points_read++;
    } else {
      LOG_WARN_S << "pcd_format_t: Skipping invalid ASCII line ("
                 << points_read + 1 << ") in file.";
    }
  }
  if (points_read != header.points) {
    LOG_WARN_S << "pcd_format_t: Read " << points_read
               << " points, but header expected " << header.points
               << " (ASCII).";
    return points_read
        == header.points;  // Require full read (要求完整读取/Require full read)
  }
  return true;
}

/**
 * @brief 从二进制格式的PCD文件读取点云数据/Read point cloud data from binary
 * format PCD file
 *
 * @tparam T 点云数据的数值类型/The numeric type of the point cloud data
 * @param path 输入文件路径/Input file path
 * @param header PCD文件头信息/PCD file header information
 * @param cloud 输出点云数据/Output point cloud
 * @return bool 读取操作是否成功/Whether the read operation was successful
 *
 * @code
 * // 从二进制格式PCD文件读取点云
 * pcd_header_t header;
 * // ... 解析头部 ...
 * point_cloud_t<float> cloud;
 * pcd_format_t pcd;
 * bool success = pcd.read_binary_data("input_binary.pcd", header, cloud);
 * @endcode
 */
template<typename T>
bool pcd_format_t::read_binary_data(const std::string& path,
                                    const pcd_header_t& header,
                                    point_cloud_t<T>& cloud)
{
  toolbox::file::memory_mapped_file_t mapped_file;
  if (!mapped_file.open(path)) {
    LOG_ERROR_S << "pcd_format_t: Failed to memory map file: " << path;
    return false;
  }

  const unsigned char* buffer_start = mapped_file.data();
  if (!buffer_start) {
    LOG_ERROR_S << "pcd_format_t: Memory mapped buffer is null for: " << path;
    return false;
  }
  const unsigned char* data_ptr = buffer_start + header.header_length;
  const unsigned char* buffer_end = buffer_start + mapped_file.size();

  // Pre-fetch field info (预先获取字段信息/Pre-fetch field info)
  auto x_info = header.get_field_info("x");
  auto y_info = header.get_field_info("y");
  auto z_info = header.get_field_info("z");
  if (!x_info || !y_info || !z_info) {
    LOG_ERROR_S
        << "pcd_format_t: Essential fields (x, y, z) info not found in header.";
    return false;
  }

  bool has_normals = false;
  std::optional<pcd_header_t::field_info_t> nx_info, ny_info, nz_info;
  if (header.m_field_indices.count("normal_x")
      && header.m_field_indices.count("normal_y")
      && header.m_field_indices.count("normal_z"))
  {
    nx_info = header.get_field_info("normal_x");
    ny_info = header.get_field_info("normal_y");
    nz_info = header.get_field_info("normal_z");
    has_normals = nx_info && ny_info && nz_info;
    if (!has_normals)
      LOG_WARN_S << "pcd_format_t: Normals expected but info couldn't be "
                    "retrieved. Skipping normals.";
  }

  bool has_colors = false;
  std::optional<pcd_header_t::field_info_t> r_info, g_info, b_info;
  if (header.m_field_indices.count("r") && header.m_field_indices.count("g")
      && header.m_field_indices.count("b"))
  {
    r_info = header.get_field_info("r");
    g_info = header.get_field_info("g");
    b_info = header.get_field_info("b");
    has_colors = r_info && g_info && b_info;
    if (!has_colors)
      LOG_WARN_S << "pcd_format_t: R,G,B expected but info couldn't be "
                    "retrieved. Skipping colors.";
    else if (!(r_info->type == 'U' && r_info->size == 1 && g_info->type == 'U'
               && g_info->size == 1 && b_info->type == 'U'
               && b_info->size == 1))
    {
      LOG_WARN_S << "pcd_format_t: R,G,B fields found but are not TYPE=U, "
                    "SIZE=1. Skipping colors.";
      has_colors = false;
    }
  }

  cloud.points.reserve(header.points);
  if (has_normals)
    cloud.normals.reserve(header.points);
  if (has_colors)
    cloud.colors.reserve(header.points);

  auto read_typed_field = [&](const pcd_header_t::field_info_t& info,
                              const unsigned char* point_ptr,
                              T& out_val) -> bool
  {
#define READ_BINARY_CASE(PCDType) \
  case sizeof(PCDType): \
    if (!read_binary_field_value<T, PCDType>(point_ptr, info.offset, out_val)) \
    { \
      LOG_ERROR_S << "pcd_format_t: Failed binary read (offset " \
                  << info.offset << ", size " << sizeof(PCDType) << ")"; \
      return false; \
    } \
    break
    switch (info.type) {
      case 'F':
        switch (info.size) {
          READ_BINARY_CASE(float);
          READ_BINARY_CASE(double);
          default:
            LOG_ERROR_S << "Unsupported float size " << info.size;
            return false;
        }
        break;
      case 'I':
        switch (info.size) {
          READ_BINARY_CASE(int8_t);
          READ_BINARY_CASE(int16_t);
          READ_BINARY_CASE(int32_t);
          READ_BINARY_CASE(int64_t);
          default:
            LOG_ERROR_S << "Unsupported int size " << info.size;
            return false;
        }
        break;
      case 'U':
        switch (info.size) {
          READ_BINARY_CASE(uint8_t);
          READ_BINARY_CASE(uint16_t);
          READ_BINARY_CASE(uint32_t);
          READ_BINARY_CASE(uint64_t);
          default:
            LOG_ERROR_S << "Unsupported uint size " << info.size;
            return false;
        }
        break;
      default:
        LOG_ERROR_S << "Unsupported field type " << info.type;
        return false;
    }
    return true;
#undef READ_BINARY_CASE
  };

  auto read_color_comp = [&](const pcd_header_t::field_info_t& info,
                             const unsigned char* point_ptr,
                             T& out_val) -> bool
  {
    uint8_t pcd_val;
    if (!read_binary_field_value<uint8_t, uint8_t>(
            point_ptr, info.offset, pcd_val))
    {
      LOG_ERROR_S << "pcd_format_t: Failed binary color read (offset "
                  << info.offset << ")";
      return false;
    }
    if constexpr (std::is_floating_point_v<T>) {
      out_val = static_cast<T>(pcd_val) / static_cast<T>(255.0);
    } else {
      out_val = static_cast<T>(pcd_val);
    }
    return true;
  };

  for (size_t i = 0; i < header.points; ++i) {
    const unsigned char* current_point_ptr = data_ptr + i * header.point_step;
    if (current_point_ptr < data_ptr
        || current_point_ptr + header.point_step > buffer_end)
    {
      LOG_ERROR_S << "pcd_format_t: Attempt to read past mapped buffer "
                     "boundary at point index "
                  << i;
      return false;
    }

    point_t<T> current_point;
    if (!read_typed_field(*x_info, current_point_ptr, current_point.x)
        || !read_typed_field(*y_info, current_point_ptr, current_point.y)
        || !read_typed_field(*z_info, current_point_ptr, current_point.z))
    {
      LOG_ERROR_S << "pcd_format_t: Failed reading XYZ for point " << i;
      return false;
    }
    cloud.points.push_back(current_point);

    if (has_normals) {
      point_t<T> current_normal;
      if (!read_typed_field(*nx_info, current_point_ptr, current_normal.x)
          || !read_typed_field(*ny_info, current_point_ptr, current_normal.y)
          || !read_typed_field(*nz_info, current_point_ptr, current_normal.z))
      {
        LOG_ERROR_S << "pcd_format_t: Failed reading normal for point " << i;
        return false;
      }
      cloud.normals.push_back(current_normal);
    }

    if (has_colors) {
      point_t<T> current_color;
      if (!read_color_comp(*r_info, current_point_ptr, current_color.x)
          || !read_color_comp(*g_info, current_point_ptr, current_color.y)
          || !read_color_comp(*b_info, current_point_ptr, current_color.z))
      {
        LOG_ERROR_S << "pcd_format_t: Failed reading color for point " << i;
        return false;
      }
      cloud.colors.push_back(current_color);
    }
  }
  return true;
}

/**
 * @brief 解析ASCII格式PCD文件中的一行点数据/Parse a line of point data from
 * ASCII format PCD file
 *
 * @tparam PointStorageT 点云数据的存储类型/Storage type for point cloud data
 * @param line 要解析的文本行/Text line to parse
 * @param header PCD文件头信息/PCD file header information
 * @param point 输出点坐标/Output point coordinates
 * @param normal 输出法线（如果有）/Output normal (if available)
 * @param color 输出颜色（如果有）/Output color (if available)
 * @return bool 解析是否成功/Whether parsing was successful
 *
 * @code
 * // 解析ASCII格式的点数据行
 * std::string line = "1.0 2.0 3.0 0.1 0.2 0.3 255 128 64";
 * pcd_header_t header;
 * // ... 设置header字段信息 ...
 * point_t<float> point, normal, color;
 * pcd_format_t pcd;
 * bool success = pcd.parse_ascii_point_line(line, header, point, &normal,
 * &color);
 * @endcode
 */
template<typename PointStorageT>
bool pcd_format_t::parse_ascii_point_line(const std::string& line,
                                          const pcd_header_t& header,
                                          point_t<PointStorageT>& point,
                                          point_t<PointStorageT>* normal,
                                          point_t<PointStorageT>* color)
{
  std::stringstream ss(line);
  std::vector<std::string> tokens;
  std::string token;
  while (ss >> token) {
    tokens.push_back(token);
  }

  if (tokens.size() < header.fields.size()) {
    LOG_WARN_S << "pcd_format_t: ASCII line has fewer values (" << tokens.size()
               << ") than expected fields (" << header.fields.size()
               << "). Line: " << line;
    return false;
  }
  if (tokens.size() > header.fields.size()) {
    LOG_WARN_S << "pcd_format_t: ASCII line has more values (" << tokens.size()
               << ") than expected fields (" << header.fields.size()
               << "). Ignoring extra values. Line: " << line;
  }

  std::map<std::string,
           std::variant<float,
                        double,
                        int8_t,
                        uint8_t,
                        int16_t,
                        uint16_t,
                        int32_t,
                        uint32_t>>
      field_values;

  for (size_t i = 0; i < header.fields.size(); ++i) {
    const std::string& field_name = header.fields[i];
    char pcd_type = header.types[i];
    size_t pcd_size = header.sizes[i];
    const std::string& val_str = tokens[i];

    try {
      if (pcd_type == 'F') {
        if (pcd_size == 4)
          field_values[field_name] = std::stof(val_str);
        else if (pcd_size == 8)
          field_values[field_name] = std::stod(val_str);
        else {
          LOG_ERROR_S << "Unsupported float size " << pcd_size;
          return false;
        }
      } else if (pcd_type == 'I') {
        long long val = std::stoll(val_str);
        if (pcd_size == 1)
          field_values[field_name] = static_cast<int8_t>(val);
        else if (pcd_size == 2)
          field_values[field_name] = static_cast<int16_t>(val);
        else if (pcd_size == 4)
          field_values[field_name] = static_cast<int32_t>(val);
        else {
          LOG_ERROR_S << "Unsupported int size " << pcd_size;
          return false;
        }
      } else if (pcd_type == 'U') {
        unsigned long long val = std::stoull(val_str);
        if (pcd_size == 1)
          field_values[field_name] = static_cast<uint8_t>(val);
        else if (pcd_size == 2)
          field_values[field_name] = static_cast<uint16_t>(val);
        else if (pcd_size == 4)
          field_values[field_name] = static_cast<uint32_t>(val);
        else {
          LOG_ERROR_S << "Unsupported uint size " << pcd_size;
          return false;
        }
      } else {
        LOG_ERROR_S << "Unsupported type '" << pcd_type << "'";
        return false;
      }
    } catch (const std::exception& e) {
      LOG_ERROR_S << "Error parsing ASCII value '" << val_str << "' for field '"
                  << field_name << "': " << e.what();
      return false;
    }
  }

  auto get_val = [&](const std::string& name) -> std::optional<PointStorageT>
  {
    if (field_values.count(name)) {
      try {
        return std::visit([](auto&& arg) -> PointStorageT
                          { return static_cast<PointStorageT>(arg); },
                          field_values.at(name));
      } catch (...) {
        LOG_ERROR_S << "Exception casting value for field '" << name << "'";
        return std::nullopt;
      }
    }
    return std::nullopt;
  };

  auto get_color_comp =
      [&](const std::string& name) -> std::optional<PointStorageT>
  {
    if (field_values.count(name)) {
      try {
        uint8_t val = std::visit(
            [](auto&& arg) -> uint8_t
            {
              using TArg = std::decay_t<decltype(arg)>;
              if constexpr (std::is_integral_v<TArg>) {
                return static_cast<uint8_t>(std::clamp(
                    static_cast<int64_t>(arg), INT64_C(0), INT64_C(255)));
              } else if constexpr (std::is_floating_point_v<TArg>) {
                return static_cast<uint8_t>(
                    std::clamp(arg * 255.0, 0.0, 255.0));
              } else {
                return 0;
              }
            },
            field_values.at(name));
        if constexpr (std::is_floating_point_v<PointStorageT>) {
          return static_cast<PointStorageT>(val)
              / static_cast<PointStorageT>(255.0);
        } else {
          return static_cast<PointStorageT>(val);
        }
      } catch (...) {
        LOG_ERROR_S << "Exception casting color value for field '" << name
                    << "'";
        return std::nullopt;
      }
    }
    return std::nullopt;
  };

  auto x_opt = get_val("x");
  auto y_opt = get_val("y");
  auto z_opt = get_val("z");
  if (!x_opt || !y_opt || !z_opt) {
    LOG_ERROR_S << "Missing x, y, or z in ASCII line.";
    return false;
  }
  point.x = *x_opt;
  point.y = *y_opt;
  point.z = *z_opt;

  if (normal) {
    auto nx_opt = get_val("normal_x");
    auto ny_opt = get_val("normal_y");
    auto nz_opt = get_val("normal_z");
    if (nx_opt && ny_opt && nz_opt) {
      normal->x = *nx_opt;
      normal->y = *ny_opt;
      normal->z = *nz_opt;
    }
  }
  if (color) {
    auto r_opt = get_color_comp("r");
    auto g_opt = get_color_comp("g");
    auto b_opt = get_color_comp("b");
    if (r_opt && g_opt && b_opt) {
      color->x = *r_opt;
      color->y = *g_opt;
      color->z = *b_opt;
    }
  }
  return true;
}

template<typename DestT, typename PCDT>
bool pcd_format_t::read_binary_field_value(const unsigned char* point_ptr,
                                           size_t offset,
                                           DestT& out_val)
{
  if (!point_ptr)
    return false;
  PCDT pcd_val;
  std::memcpy(&pcd_val, point_ptr + offset, sizeof(PCDT));
  out_val = static_cast<DestT>(pcd_val);
  return true;
}

template<typename T>
CPP_TOOLBOX_EXPORT std::unique_ptr<toolbox::types::point_cloud_t<T>> read_pcd(
    const std::string& path)
{
  auto pcd = std::make_unique<toolbox::io::formats::pcd_format_t>();
  std::unique_ptr<base_file_data_t> base_data =
      nullptr;  // Create ptr of the type expected by read()

  // Call read, passing the base class unique_ptr reference
  bool success = pcd->read(path, base_data);

  if (success && base_data) {
    // Read succeeded, now check if the actual data type matches T
    auto* typed_ptr = dynamic_cast<point_cloud_t<T>*>(base_data.get());
    if (typed_ptr) {
      // Type matches T, transfer ownership to a new
      // unique_ptr<point_cloud_t<T>>
      base_data.release();  // Prevent base_data from deleting the object
      return std::unique_ptr<point_cloud_t<T>>(typed_ptr);
    } else {
      // Read succeeded, but the resulting type is not point_cloud_t<T>
      // (e.g., read always returns float, but T was double)
      LOG_WARN_S << "read_pcd: read data type does not match requested type T="
                 << typeid(T).name() << ". Path: " << path;
      // Return nullptr as the requested type couldn't be provided.
      // The base_data unique_ptr will delete the object when it goes out of
      // scope.
      return nullptr;
    }
  }

  // Read failed or returned null data
  return nullptr;
}

template<typename T>
CPP_TOOLBOX_EXPORT bool write_pcd(const std::string& path,
                                  const toolbox::types::point_cloud_t<T>& cloud,
                                  bool binary)
{
  auto pcd = std::make_unique<toolbox::io::formats::pcd_format_t>();

  // Create a copy of the input cloud to satisfy the unique_ptr interface
  auto cloud_copy_ptr =
      std::make_unique<toolbox::types::point_cloud_t<T>>(cloud);

  // Cast the unique_ptr holding the copy to the base class type unique_ptr
  std::unique_ptr<base_file_data_t> base_data_ptr = std::move(cloud_copy_ptr);

  // Pass the unique_ptr (as const reference) to the write method
  return pcd->write(path, base_data_ptr, binary);
}

}  // namespace toolbox::io::formats
