#pragma once

#include <filesystem>
#include <optional>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/file/file.hpp>  ///< 用于列出目录下的文件/For listing files in a directory
#include <cpp-toolbox/io/dataset/dataset.hpp>
#include <cpp-toolbox/io/formats/kitti.hpp>  ///< 用于读取KITTI点云/For reading KITTI

namespace toolbox::io
{

/**
 * @brief KITTI点云数据集类/KITTI point cloud dataset class
 *
 * 用于加载和访问指定目录下的KITTI格式点云数据/Used to load and access KITTI
 * format point cloud data in a given directory
 *
 * @code
 * // 示例：加载KITTI点云数据集并访问点云
 * toolbox::io::kitti_pcd_dataset_t dataset("/path/to/velodyne");
 * std::size_t n = dataset.size(); // 获取点云数量/Get number of point clouds
 * auto cloud_opt = dataset.at(0); // 访问第一个点云/Access the first point
 * cloud if (cloud_opt && *cloud_opt) {
 *   // 使用点云/Use the point cloud
 * }
 * @endcode
 */
class CPP_TOOLBOX_EXPORT kitti_pcd_dataset_t
    : public dataset_t<kitti_pcd_dataset_t,
                       std::unique_ptr<toolbox::types::point_cloud_t<float>>>
{
public:
  /**
   * @brief 构造函数，加载指定目录下的所有KITTI点云/Constructor, loads all KITTI
   * point clouds in the given directory
   * @param velodye_dir KITTI点云文件所在目录/Directory containing KITTI point
   * cloud files
   *
   * @code
   * toolbox::io::kitti_pcd_dataset_t dataset("/path/to/velodyne");
   * @endcode
   */
  explicit kitti_pcd_dataset_t(const std::string& velodye_dir);

  /**
   * @brief 获取数据集中的点云数量/Get the number of point clouds in the dataset
   * @return 点云数量/Number of point clouds
   *
   * @code
   * std::size_t n = dataset.size_impl();
   * @endcode
   */
  [[nodiscard]] std::size_t size_impl() const;

  /**
   * @brief 按索引访问点云/Access a point cloud by index
   * @param index 点云索引/Index of the point cloud
   * @return 对应索引的点云的unique_ptr（可选）/Optional unique_ptr to the point
   * cloud at the given index
   *
   * @code
   * auto cloud_opt = dataset.at_impl(0);
   * if (cloud_opt && *cloud_opt) {
   *   // 处理点云/Process the point cloud
   * }
   * @endcode
   */
  [[nodiscard]] std::optional<
      std::unique_ptr<toolbox::types::point_cloud_t<float>>>
  at_impl(std::size_t index) const;

private:
  /**
   * @brief KITTI点云文件路径列表/List of KITTI point cloud file paths
   */
  std::vector<std::filesystem::path> m_binary_paths;
};

/**
 * @brief KITTI点云对数据集类/KITTI point cloud pair dataset class
 *
 * 用于按指定间隔成对加载KITTI点云数据/Used to load KITTI point cloud pairs with
 * a specified skip interval
 *
 * @code
 * // 示例：加载KITTI点云对数据集并访问点云对
 * toolbox::io::kitti_pcd_pair_t pair_dataset("/path/to/velodyne", 1);
 * std::size_t n = pair_dataset.size();
 * auto pair_opt = pair_dataset.at(0);
 * if (pair_opt && pair_opt->first && pair_opt->second) {
 *   // 处理点云对/Process the point cloud pair
 * }
 * @endcode
 */
class CPP_TOOLBOX_EXPORT kitti_pcd_pair_t
    : public dataset_t<
          kitti_pcd_pair_t,
          std::pair<std::unique_ptr<toolbox::types::point_cloud_t<float>>,
                    std::unique_ptr<toolbox::types::point_cloud_t<float>>>>
{
public:
  /**
   * @brief 构造函数，加载指定目录下的KITTI点云对/Constructor, loads KITTI point
   * cloud pairs from the given directory
   * @param velodye_dir KITTI点云文件所在目录/Directory containing KITTI point
   * cloud files
   * @param skip 点云对之间的间隔/Skip interval between point cloud pairs
   *
   * @code
   * toolbox::io::kitti_pcd_pair_t pair_dataset("/path/to/velodyne", 2);
   * @endcode
   */
  explicit kitti_pcd_pair_t(const std::string& velodye_dir,
                            std::size_t skip = 0);

  /**
   * @brief 获取点云对的数量/Get the number of point cloud pairs
   * @return 点云对数量/Number of point cloud pairs
   *
   * @code
   * std::size_t n = pair_dataset.size_impl();
   * @endcode
   */
  [[nodiscard]] std::size_t size_impl() const;

  /**
   * @brief 按索引访问点云对/Access a point cloud pair by index
   * @param index 点云对索引/Index of the point cloud pair
   * @return 对应索引的点云对的unique_ptr对（可选）/Optional pair of unique_ptrs
   * to the point clouds at the given index
   *
   * @code
   * auto pair_opt = pair_dataset.at_impl(0);
   * if (pair_opt && pair_opt->first && pair_opt->second) {
   *   // 处理点云对/Process the point cloud pair
   * }
   * @endcode
   */
  [[nodiscard]] std::optional<
      std::pair<std::unique_ptr<toolbox::types::point_cloud_t<float>>,
                std::unique_ptr<toolbox::types::point_cloud_t<float>>>>
  at_impl(std::size_t index) const;

private:
  /**
   * @brief KITTI点云文件路径列表/List of KITTI point cloud file paths
   */
  std::vector<std::filesystem::path> m_binary_paths;

  /**
   * @brief 点云对之间的间隔/Skip interval between point cloud pairs
   */
  std::size_t m_skip;
};

}  // namespace toolbox::io