#include <chrono>      // For std::chrono::milliseconds
#include <iostream>    // Optional, for debug output
#include <stdexcept>   // For std::invalid_argument
#include <utility>
#include <memory>

#include "cpp-toolbox/base/thread_pool.hpp"  // Include header file

namespace toolbox::base
{

/**
 * @brief Constructs a thread pool with specified number of threads
 * @param threads Number of threads to create. If 0, uses hardware concurrency
 * @throws std::invalid_argument if thread count is 0 after all fallbacks
 */
thread_pool_t::thread_pool_t(size_t threads)
    : stop_(false)
{
  // If thread count is 0, try to get hardware concurrency
  size_t num_threads = threads;
  if (num_threads == 0) {
    num_threads = std::thread::hardware_concurrency();
    // If hardware concurrency cannot be determined or is 0, use at least 1
    // thread
    if (num_threads == 0) {
      num_threads = 1;
    }
  }

  if (num_threads == 0) {  // Should never reach here, but defensive programming
    throw std::invalid_argument("Thread pool cannot have 0 threads");
  }

  // 预先分配空间以避免在添加元素时进行复制操作
  worker_queues_.reserve(num_threads);
  queue_mutexes_.reserve(num_threads);

  // 使用 shared_ptr 包装每个 deque 以避免复制问题
  for (size_t i = 0; i < num_threads; ++i) {
    // 为每个工作线程创建一个新的 deque
    worker_queues_.push_back(std::make_shared<std::deque<std::unique_ptr<detail::task_base>>>());
    queue_mutexes_.push_back(std::make_unique<std::mutex>());
  }

  workers_.reserve(num_threads);
  for (size_t i = 0; i < num_threads; ++i) {
    workers_.emplace_back(&thread_pool_t::worker_loop, this, i);
  }
}

/**
 * @brief Destructor that stops all threads and waits for them to finish
 */
thread_pool_t::~thread_pool_t()
{
  stop_.store(true, std::memory_order_release);
  for (std::thread& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

void thread_pool_t::worker_loop(size_t worker_id)
{
  while (true) {
    std::unique_ptr<detail::task_base> task;

    {
      std::lock_guard<std::mutex> lock(*queue_mutexes_[worker_id]);
      auto& queue = *worker_queues_[worker_id];
      if (!queue.empty()) {
        // 使用 std::move 确保 unique_ptr 被移动而不是复制
        task = std::move(queue.back());
        queue.pop_back();
      }
    }

    if (!task) {
      for (size_t n = 0; n < worker_queues_.size(); ++n) {
        size_t victim = (worker_id + n + 1) % worker_queues_.size();
        std::lock_guard<std::mutex> lock(*queue_mutexes_[victim]);
        auto& queue = *worker_queues_[victim];
        if (!queue.empty()) {
          // 使用 std::move 确保 unique_ptr 被移动而不是复制
          task = std::move(queue.front());
          queue.pop_front();
          break;
        }
      }
    }

    if (!task) {
      if (stop_.load(std::memory_order_acquire)) {
        bool empty = true;
        for (size_t i = 0; i < worker_queues_.size(); ++i) {
          std::lock_guard<std::mutex> lock(*queue_mutexes_[i]);
          auto& queue = *worker_queues_[i];
          if (!queue.empty()) {
            empty = false;
            break;
          }
        }
        if (empty) {
          return;
        }
      }
      std::this_thread::yield();
      continue;
    }

    try {
      task->execute();
    } catch (const std::exception& e) {
      std::cerr << "Worker thread " << worker_id
                << " caught exception during task execution: " << e.what()
                << std::endl;
    } catch (...) {
      std::cerr << "Worker thread " << worker_id
                << " caught unknown exception during task execution."
                << std::endl;
    }
  }
}

}  // namespace toolbox::base
