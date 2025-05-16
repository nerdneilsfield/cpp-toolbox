/**
 * @file kitti_pcd_dataset_example.cpp
 * @brief KITTI点云数据集命令行示例程序/KITTI point cloud dataset CLI example
 * @details 展示如何通过命令行应用加载和遍历KITTI点云数据集/Demonstrates how to load and iterate KITTI point cloud dataset via CLI
 */

#include <algorithm>
#include <exception>
#include <optional>
#include <string>

#include <cpp-toolbox/io/dataset/kitti_pcd.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>  ///< 用于日志输出/For logger output
#include <cpp-toolbox/utils/click.hpp>

/**
 * @brief 使用命名空间以简化代码/Use namespaces for brevity
 */
using namespace toolbox::utils;
using namespace toolbox::logger;
using namespace toolbox::io;

/**
 * @brief 主函数/Entry point of the program
 * @param argc 命令行参数数量/Number of command line arguments
 * @param argv 命令行参数数组/Array of command line arguments
 * @return 程序退出码/Program exit code
 *
 * @code
 * // 运行示例/Run example:
 * ./kitti_pcd_dataset_example -d /path/to/velodyne
 * @endcode
 */
int main(int argc, char** argv)
{
  /**
   * @brief 获取线程安全日志实例/Obtain thread-safe logger instance
   */
  auto& logger = thread_logger_t::instance();
  logger.set_level(thread_logger_t::Level::INFO);

  /**
   * @brief 创建命令行应用/Create command line application
   */
  CommandLineApp app(
      "kitti_pcd_dataset_example",
      "An example CLI application demonstrating KITTI PCD dataset usage./"
      "一个演示KITTI点云数据集用法的命令行示例程序。");

  /**
   * @brief 添加全局verbose标志/Add global verbose flag
   */
  auto& verbose_flag =
      app.add_flag("verbose", "v", "Enable verbose output (global)./启用详细输出（全局）。");

  /**
   * @brief 添加数据集目录参数/Add dataset directory argument
   */
  auto& dataset_dir_arg = app.add_option<std::string>(
      "dataset_dir", "d", "Path to the KITTI dataset binary file directory./KITTI数据集二进制文件目录路径。");

  /**
   * @brief 设置命令行回调/Set command line callback
   *
   * @code
   * // 示例：命令行回调会根据参数加载数据集并遍历前10帧/Example: The callback loads the dataset and iterates the first 10 frames
   * @endcode
   */
  app.set_callback(
      [&]()
      {
        // 如果启用verbose则设置日志级别为TRACE/Set logger level to TRACE if verbose is enabled
        if (verbose_flag.get()) {
          logger.set_level(thread_logger_t::Level::TRACE);
        }

        /**
         * @brief 加载KITTI点云数据集/Load KITTI point cloud dataset
         */
        kitti_pcd_dataset_t dataset(dataset_dir_arg.get());
        std::size_t dataset_size = dataset.size();
        LOG_INFO_S << "Found " << dataset_size << " point clouds in dataset./"
                   << "在数据集中找到" << dataset_size << "个点云。";

        // 遍历前10帧点云/Iterate the first 10 point clouds
        for (std::size_t i = 0;
             i < std::min(static_cast<std::size_t>(10), dataset_size);
             i++)
        {
          LOG_INFO_S << "Processing point cloud " << i << " of " << dataset_size
                     << " have " << dataset.get_next()->get()->size()
                     << " points./正在处理第" << i << "个点云，共" << dataset_size
                     << "个，包含" << dataset.get_next()->get()->size() << "个点。";
        }
        return 0;
      });

  int exit_code = 0;
  try {
    exit_code = app.run(argc, argv);
  } catch (const std::exception& e) {
    LOG_CRITICAL_S << "Unhandled exception: " << e.what()
                   << "/未处理的异常: " << e.what();
    exit_code = 1;
  }
  thread_logger_t::shutdown();
  return exit_code;
}