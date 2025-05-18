#include <algorithm>

#include <cpp-toolbox/io/dataset/kitti_pcd.hpp>
#include <cpp-toolbox/io/formats/kitti.hpp>  // For read_kitti_bin
#include <cpp-toolbox/logger/thread_logger.hpp>

namespace toolbox::io
{

kitti_pcd_dataset_t::kitti_pcd_dataset_t(const std::string& velodye_dir)
{
  m_binary_paths = toolbox::file::list_files_in_directory(velodye_dir, ".bin");
  std::sort(m_binary_paths.begin(), m_binary_paths.end());
  LOG_INFO_S << "kitti_pcd_dataset_t: Found " << m_binary_paths.size()
             << " KITTI point cloud files in " << velodye_dir;
}

std::size_t kitti_pcd_dataset_t::size_impl() const
{
  return m_binary_paths.size();
}

std::optional<std::unique_ptr<toolbox::types::point_cloud_t<float>>>
kitti_pcd_dataset_t::at_impl(std::size_t index) const
{
  if (index >= m_binary_paths.size()) {
    LOG_WARN_S << "kitti_pcd_dataset_t: Index out of range. Which index: "
               << index << " Size: " << m_binary_paths.size() << "\n";
    return std::nullopt;
  }

  auto cloud = read_kitti_bin<float>(
      toolbox::file::path_to_string(m_binary_paths[index]));
  return cloud;
}

kitti_pcd_pair_t::kitti_pcd_pair_t(const std::string& velodye_dir,
                                   std::size_t skip)
    : m_skip(skip)
{
  m_binary_paths = toolbox::file::list_files_in_directory(velodye_dir, ".bin");
  std::sort(m_binary_paths.begin(), m_binary_paths.end());
  LOG_INFO_S << "kitti_pcd_pair_t: Found " << m_binary_paths.size()
             << " KITTI point cloud files in " << velodye_dir;
}

std::size_t kitti_pcd_pair_t::size_impl() const
{
  return m_binary_paths.size() - m_skip;
}

std::optional<std::pair<std::unique_ptr<toolbox::types::point_cloud_t<float>>,
                        std::unique_ptr<toolbox::types::point_cloud_t<float>>>>
kitti_pcd_pair_t::at_impl(std::size_t index) const
{
  if (index >= m_binary_paths.size() - m_skip) {
    LOG_WARN_S << "kitti_pcd_pair_t: Index out of range. Which index: " << index
               << " Size: " << m_binary_paths.size() << " Skip: " << m_skip
               << "\n";
    return std::nullopt;
  }

  auto cloud1 =
      read_kitti_bin<float>(toolbox::file::path_to_string(m_binary_paths[index]));
  auto cloud2 = read_kitti_bin<float>(
      toolbox::file::path_to_string(m_binary_paths[index + m_skip]));
  return std::make_pair(std::move(cloud1), std::move(cloud2));
}

}  // namespace toolbox::io
