/**
 * @file dataloader_example.cpp
 * @brief DataLoader示例/DataLoader example
 * @details 参考 kitti_pcd_dataset_example.cpp 展示如何结合 DataLoader 与
 * Sampler 迭代数据集
 */

#include <algorithm>
#include <string>

#include <cpp-toolbox/io/dataloader/dataloader.hpp>
#include <cpp-toolbox/io/dataloader/sampler.hpp>
#include <cpp-toolbox/io/dataset/kitti_pcd.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>
#include <cpp-toolbox/utils/click.hpp>

using namespace toolbox::utils;
using namespace toolbox::logger;
using namespace toolbox::io;

int main(int argc, char** argv)
{
  auto& logger = thread_logger_t::instance();
  logger.set_level(thread_logger_t::Level::INFO);

  CommandLineApp app(
      "dataloader_example",
      "Demonstrates DataLoader and Sampler usage with KITTI dataset./"
      "演示如何结合DataLoader与Sampler读取KITTI数据集");

  auto& dataset_dir_arg = app.add_option<std::string>(
      "dataset_dir",
      "d",
      "Path to KITTI dataset binary file directory./KITTI数据集二进制文件目录");

  auto& batch_size_opt =
      app.add_option<std::size_t>("batch_size", "b", "Batch size./批次大小")
          .set_default(2);

  auto& shuffle_flag =
      app.add_flag("shuffle", "s", "Use shuffle sampler./使用随机采样");

  auto& seed_opt =
      app.add_option<unsigned>("seed", "S", "Shuffle seed./随机种子")
          .set_default(123);

  auto& prefetch_opt =
      app.add_option<std::size_t>(
             "prefetch", "p", "Number of prefetch batches./预取批次数")
          .set_default(0);

  auto& num_batches_opt =
      app.add_option<std::size_t>(
             "num_batches", "n", "Number of batches to iterate./要迭代的批次数")
          .set_default(2);

  auto& verbose_flag =
      app.add_flag("verbose", "v", "Enable verbose output./启用详细输出");

  app.set_callback(
      [&]()
      {
        if (verbose_flag.get()) {
          logger.set_level(thread_logger_t::Level::TRACE);
        }

        kitti_pcd_dataset_t dataset(dataset_dir_arg.get());
        std::size_t dataset_size = dataset.size();
        LOG_INFO_S << "Dataset size: " << dataset_size
                   << "/数据集大小: " << dataset_size;

        auto show_batches = [&](auto& loader)
        {
          std::size_t batch_idx = 0;
          for (const auto& batch : loader) {
            LOG_INFO_S << "Batch " << batch_idx << " size: " << batch.size()
                       << "/批次" << batch_idx << " 大小: " << batch.size();
            for (std::size_t i = 0; i < batch.size(); ++i) {
              const auto& cloud_ptr = batch[i];
              if (cloud_ptr) {
                LOG_INFO_S << "  Cloud " << i << " has " << cloud_ptr->size()
                           << " points./点云" << i << "含" << cloud_ptr->size()
                           << "个点";
              }
            }
            if (++batch_idx >= num_batches_opt.get()) {
              break;
            }
          }
        };

        if (shuffle_flag.get()) {
          shuffle_policy_t policy(seed_opt.get());
          sampler_t<shuffle_policy_t> sampler(dataset_size, policy);
          dataloader_t loader(
              dataset, sampler, batch_size_opt.get(), prefetch_opt.get());
          show_batches(loader);
        } else {
          sampler_t<sequential_policy_t> sampler(dataset_size);
          dataloader_t loader(
              dataset, sampler, batch_size_opt.get(), prefetch_opt.get());
          show_batches(loader);
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
