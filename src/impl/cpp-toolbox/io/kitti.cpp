#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <cpp-toolbox/container/string.hpp>  // ends_with
#include <cpp-toolbox/file/memory_mapped_file.hpp>
#include <cpp-toolbox/io/formats/kitti.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

namespace toolbox::io::formats
{

using toolbox::file::memory_mapped_file_t;
using toolbox::types::point_cloud_t;
using toolbox::types::point_t;

// --- kitti_format_t Public Member Functions Implementation ---

bool kitti_format_t::can_read(const std::string& path) const
{
  // 基本检查：仅检查扩展名/Basic check: just the extension
  return toolbox::container::string::ends_with(path, ".bin");
}

std::vector<std::string> kitti_format_t::get_supported_extensions() const
{
  return {".bin"};
}

bool kitti_format_t::read(const std::string& path,
                          std::unique_ptr<base_file_data_t>& data)
{
  LOG_INFO_S << "kitti_format_t: Attempting to read KITTI file: " << path;

  // 默认读取为float类型/Default to reading as float
  auto cloud = std::make_unique<point_cloud_t<float>>();
  cloud->clear();  // 确保为空/Ensure it's empty

  // KITTI格式没有头部，直接读取二进制数据/KITTI format has no header, directly read binary data
  bool success = read_binary_data(path, *cloud);

  if (success) {
    LOG_INFO_S << "kitti_format_t: Finished reading KITTI file: " << path
               << ", read " << cloud->size() << " points.";
    data = std::move(cloud);  // 转移所有权到输出参数/Transfer ownership to output parameter
    return true;
  }
  
  LOG_ERROR_S << "kitti_format_t: Failed to read data from: " << path;
  return false;
}

bool kitti_format_t::write(const std::string& path,
                           const std::unique_ptr<base_file_data_t>& data,
                           bool binary) const
{
  // KITTI格式总是二进制，忽略binary参数/KITTI format is always binary, ignore binary parameter
  if (!data) {
    LOG_ERROR_S << "kitti_format_t: Cannot write null data.";
    return false;
  }

  // 尝试转换为支持的点云类型/Try casting to supported point cloud types
  if (const auto* cloud_f = dynamic_cast<const point_cloud_t<float>*>(data.get())) {
    LOG_INFO_S << "kitti_format_t: Writing point_cloud_t<float> to " << path;
    return write_internal(path, *cloud_f, true);  // KITTI总是二进制/KITTI is always binary
  }

  if (const auto* cloud_d = dynamic_cast<const point_cloud_t<double>*>(data.get())) {
    LOG_INFO_S << "kitti_format_t: Writing point_cloud_t<double> to " << path;
    return write_internal(path, *cloud_d, true);  // KITTI总是二进制/KITTI is always binary
  }
  
  LOG_ERROR_S << "kitti_format_t: Input data is not a supported point_cloud_t "
                 "type (float or double).";
  return false;
}

// 显式实例化模板函数/Explicit instantiations for template functions
template bool kitti_format_t::read_binary_data(const std::string& path,
                                             point_cloud_t<float>& cloud);

template bool kitti_format_t::read_binary_data(const std::string& path,
                                             point_cloud_t<double>& cloud);

template bool kitti_format_t::write_internal(const std::string& path,
                                           const point_cloud_t<float>& cloud,
                                           bool binary) const;

template bool kitti_format_t::write_internal(const std::string& path,
                                           const point_cloud_t<double>& cloud,
                                           bool binary) const;

// 显式实例化独立辅助函数/Explicit instantiations for standalone helper functions
template CPP_TOOLBOX_EXPORT
    std::unique_ptr<toolbox::types::point_cloud_t<float>>
    read_kitti_bin<float>(const std::string& path);

template CPP_TOOLBOX_EXPORT
    std::unique_ptr<toolbox::types::point_cloud_t<double>>
    read_kitti_bin<double>(const std::string& path);

template CPP_TOOLBOX_EXPORT bool write_kitti_bin<float>(
    const std::string& path,
    const toolbox::types::point_cloud_t<float>& cloud);

template CPP_TOOLBOX_EXPORT bool write_kitti_bin<double>(
    const std::string& path,
    const toolbox::types::point_cloud_t<double>& cloud);

}  // namespace toolbox::io::formats
