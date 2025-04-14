#pragma once

#include <atomic>  // For atomic boolean flag
#include <functional>  // For std::function, std::bind
#include <future>  // For asynchronous task results (std::future, std::packaged_task)
#include <iostream>  // For std::cout, std::cerr
#include <memory>  // For std::make_shared
#include <stdexcept>  // For runtime exceptions
#include <thread>  // C++ thread library
#include <type_traits>  // For std::invoke_result_t
#include <utility>  // For std::forward, std::move
#include <vector>  // For storing worker threads

// Include concurrent_queue
#include "cpp-toolbox/container/concurrent_queue.hpp"
#include "cpp-toolbox/logger/thread_logger.hpp"
// Export macro definition
#include <cpp-toolbox/cpp-toolbox_export.hpp>
// Macro definitions
#include <cpp-toolbox/macro.hpp>

namespace toolbox::base
{

/**
 * @brief A singleton thread pool implementation that manages a pool of worker
 * threads
 *
 * This class provides a thread-safe singleton thread pool that can execute
 * tasks asynchronously. Tasks are submitted as functions/lambdas and are queued
 * for execution by worker threads.
 *
 * @code{.cpp}
 * // Get thread pool instance
 * auto& pool = thread_pool_singleton_t::instance();
 *
 * // Submit a task and get future
 * auto future = pool.submit([](int x) { return x * 2; }, 42);
 *
 * // Get result
 * int result = future.get(); // result = 84
 * @endcode
 */
class CPP_TOOLBOX_EXPORT thread_pool_singleton_t
{
public:
  /**
   * @brief Get the singleton instance of the thread pool
   * @return Reference to the singleton thread pool instance
   */
  static thread_pool_singleton_t& instance()
  {
    static thread_pool_singleton_t instance;
    return instance;
  }

  // Delete copy and move operations to ensure singleton pattern
  thread_pool_singleton_t(const thread_pool_singleton_t&) = delete;
  thread_pool_singleton_t& operator=(const thread_pool_singleton_t&) = delete;
  thread_pool_singleton_t(thread_pool_singleton_t&&) = delete;
  thread_pool_singleton_t& operator=(thread_pool_singleton_t&&) = delete;

  /**
   * @brief Destructor that stops the thread pool and waits for all worker
   * threads to finish
   */
  ~thread_pool_singleton_t();

  /**
   * @brief Submit a task to be executed by the thread pool
   *
   * This method queues a task for asynchronous execution by one of the worker
   * threads. The task can be a function, lambda, or any callable object.
   *
   * @tparam F Type of the callable
   * @tparam Args Types of the arguments
   * @param f The callable to execute
   * @param args Arguments to pass to the callable
   * @return std::future containing the eventual result
   * @throws std::runtime_error if the thread pool has been stopped
   *
   * @code{.cpp}
   * // Submit a lambda
   * auto future1 = pool.submit([]() { return "Hello"; });
   *
   * // Submit a function with arguments
   * auto future2 = pool.submit([](int x, int y) { return x + y; }, 2, 3);
   * @endcode
   */
  template<class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result_t<F, Args...>>;

  /**
   * @brief Get the number of worker threads in the pool
   * @return The number of worker threads
   */
  size_t get_thread_count() const
  {
    return workers_.size();
  }

private:
  /**
   * @brief Private constructor for singleton pattern
   */
  thread_pool_singleton_t();

  // Member variables
  std::vector<std::thread> workers_;
  toolbox::container::concurrent_queue_t<std::function<void()>> tasks_;
  std::atomic<bool> stop_;

  /**
   * @brief Initialize and start the thread pool
   * @param threads Number of threads to create (0 means use hardware
   * concurrency)
   */
  void start_pool(size_t threads = 0);

  /**
   * @brief Stop the thread pool and cleanup resources
   */
  void stop_pool();

  /**
   * @brief Main worker thread function that processes tasks
   * @param worker_id ID of the worker thread for logging/debugging
   */
  void worker_loop(size_t worker_id);

};  // class thread_pool_singleton_t

// Template implementation

template<class F, class... Args>
auto thread_pool_singleton_t::submit(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result_t<F, Args...>>
{
  using return_type = typename std::invoke_result_t<F, Args...>;

  if (stop_.load(std::memory_order_relaxed)) {
    throw std::runtime_error("Cannot submit task to stopped thread pool");
  }

  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();

  tasks_.enqueue(
      [task]() mutable
      {
        try {
          (*task)();
        } catch (const std::future_error& fe) {
          LOG_ERROR_S << "Worker caught future_error: " << fe.what();
        } catch (const std::exception& e) {
          LOG_ERROR_S << "Worker caught exception: " << e.what();
        } catch (...) {
          LOG_ERROR_S << "Worker caught unknown exception.";
        }
      });

  return res;
}

}  // namespace toolbox::base
