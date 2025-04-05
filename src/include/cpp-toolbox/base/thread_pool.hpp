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
// Export macro definition
#include <cpp-toolbox/cpp-toolbox_export.hpp>
// Macro definitions
#include <cpp-toolbox/macro.hpp>

namespace toolbox::base
{

/**
 * @brief A high-performance C++17 thread pool implementation using
 * moodycamel::ConcurrentQueue (via wrapper).
 *
 * This thread pool allows submitting tasks and asynchronously retrieving
 * results. It creates a fixed number of worker threads upon construction and
 * gracefully stops them during destruction. Uses
 * `toolbox::container::concurrent_queue_t` as the underlying task queue.
 *
 * @code{.cpp}
 * // Create a thread pool with default number of threads
 * thread_pool_t pool;
 *
 * // Submit a simple task
 * auto future = pool.submit([]() { return 42; });
 * int result = future.get(); // result will be 42
 *
 * // Submit a task with arguments
 * auto future2 = pool.submit([](int a, int b) { return a + b; }, 10, 20);
 * int result2 = future2.get(); // result2 will be 30
 * @endcode
 */
class CPP_TOOLBOX_EXPORT thread_pool_t
{
public:
  /**
   * @brief Constructs and initializes the thread pool.
   *
   * @param threads Number of worker threads to create. Defaults to hardware
   * concurrency. If hardware concurrency cannot be determined or is 0, creates
   * at least 1 thread.
   * @throws std::invalid_argument if thread count parameter is 0.
   *
   * @code{.cpp}
   * // Create pool with 4 threads
   * thread_pool_t pool(4);
   *
   * // Create pool with default number of threads
   * thread_pool_t default_pool;
   * @endcode
   */
  explicit thread_pool_t(size_t threads = 0);

  /**
   * @brief Destructor that stops the thread pool and waits for all worker
   * threads to finish.
   *
   * Waits for tasks already dequeued by threads to complete.
   * After destruction, the thread pool becomes unusable.
   */
  ~thread_pool_t();

  /**
   * @brief Submits a task to the thread pool for execution.
   *
   * @tparam F Type of the task function.
   * @tparam Args Types of the task function arguments.
   * @param f Task function (can be function pointer, lambda, std::function, or
   * any callable object).
   * @param args Arguments to pass to the task function.
   * @return std::future object that can be used to get the task's return value
   * or catch its exceptions.
   * @throws std::runtime_error if the thread pool has been stopped and cannot
   * accept new tasks.
   *
   * @code{.cpp}
   * // Submit a lambda task
   * auto future = pool.submit([]() { return 42; });
   *
   * // Submit a task with arguments
   * auto future2 = pool.submit([](int a, int b) { return a + b; }, 10, 20);
   *
   * // Submit a task that throws an exception
   * auto future3 = pool.submit([]() { throw std::runtime_error("error"); });
   * try {
   *     future3.get();
   * } catch (const std::exception& e) {
   *     // Handle exception
   * }
   * @endcode
   */
  template<class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result_t<F, Args...>>;

  // Delete copy constructor and copy assignment operator to prevent accidental
  // copying
  CPP_TOOLBOX_DISABLE_COPY(thread_pool_t)
  // Delete move constructor and move assignment operator to simplify lifecycle
  // management
  CPP_TOOLBOX_DISABLE_MOVE(thread_pool_t)

private:
  // List of worker threads
  std::vector<std::thread> workers_;
  // Concurrent queue for tasks
  toolbox::container::concurrent_queue_t<std::function<void()>> tasks_;
  // Atomic flag indicating whether the thread pool should stop
  std::atomic<bool> stop_;
};

// --- Template Member Function Implementation ---

template<class F, class... Args>
auto thread_pool_t::submit(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result_t<F, Args...>>
{
  using return_type = typename std::invoke_result_t<F, Args...>;

  // Check if thread pool is stopped
  if (stop_.load(std::memory_order_relaxed)) {
    throw std::runtime_error("Cannot submit task to stopped thread pool");
  }

  // Create a packaged task to wrap the function and its arguments
  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  // Get the future associated with the packaged task
  std::future<return_type> res = task->get_future();

  // Enqueue the task to the concurrent queue
  tasks_.enqueue(
      [task]()
      {
        try {
          // Execute the packaged task
          (*task)();
        } catch (const std::future_error& fe) {
          std::cerr << "Worker lambda caught future_error for task "
                    << task.get() << ": " << fe.what() << std::endl;
        } catch (const std::exception& e) {
          std::cerr << "Worker lambda caught unexpected exception during "
                    << "packaged_task invocation for task " << task.get()
                    << ": " << e.what() << std::endl;
        } catch (...) {
          std::cerr << "Worker lambda caught unknown exception during "
                    << "packaged_task invocation for task " << task.get()
                    << std::endl;
        }
      });

  return res;
}

}  // namespace toolbox::base
