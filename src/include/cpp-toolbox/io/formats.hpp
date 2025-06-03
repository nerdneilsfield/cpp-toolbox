#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/file/file.hpp>
#include <cpp-toolbox/io/formats/base.hpp>
#include <cpp-toolbox/io/formats/kitti.hpp>
#include <cpp-toolbox/io/formats/pcd.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

namespace toolbox::io
{
template<typename T>
CPP_TOOLBOX_EXPORT std::unique_ptr<toolbox::types::point_cloud_t<T>>
read_point_cloud(const std::string& path)
{
  auto extension = toolbox::file::get_file_extension(path);
  LOG_INFO_S << "Reading point cloud from " << path << " with extension "
             << extension;
  if (extension == ".pcd") {
    return read_pcd<T>(path);
  }

  if (extension == ".bin") {
    return read_kitti_bin<T>(path);
  }

  LOG_ERROR_S << "Unsupported file extension: " << extension;
  return nullptr;
}

template<typename T>
CPP_TOOLBOX_EXPORT bool write_point_cloud(
    const std::string& path, const toolbox::types::point_cloud_t<T>& cloud)
{
  auto extension = toolbox::file::get_file_extension(path);
  if (extension == "pcd") {
    return write_pcd(path, cloud);
  }

  if (extension == "bin") {
    return write_kitti_bin(path, cloud);
  }

  LOG_ERROR_S << "Unsupported file extension: " << extension;
  return false;
}
}  // namespace toolbox::io