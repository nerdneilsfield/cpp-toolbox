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

#include "cpp-toolbox/base/detail/task_base.hpp"
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
  toolbox::container::concurrent_queue_t<std::unique_ptr<detail::task_base>>
      tasks_;
  // Atomic flag indicating whether the thread pool should stop
  std::atomic<bool> stop_;
};

// --- Template Member Function Implementation ---

template<class F, class... Args>
auto thread_pool_t::submit(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result_t<F, Args...>>
{
  using return_type = typename std::invoke_result_t<F, Args...>;

  if (stop_.load(std::memory_order_relaxed)) {
    throw std::runtime_error("Cannot submit task to stopped thread pool");
  }

  // 1. Create promise and get future
  auto promise = std::make_shared<std::promise<return_type>>();
  std::future<return_type> future = promise->get_future();

  // 2. Create the lambda that does the work and sets the promise
  auto task_payload =
      // Use mutable if the lambda needs to modify its captures (like moving
      // from args_tuple)
      [func = std::forward<F>(f),  // Move/forward the original callable
       args_tuple =
           std::make_tuple(std::forward<Args>(args)...),  // Move/forward args
       promise_ptr = std::move(
           promise)]() mutable {  // Capture promise by moved shared_ptr
        try {
          if constexpr (std::is_void_v<return_type>) {
            std::apply(func,
                       std::move(args_tuple));  // Invoke original function
            promise_ptr->set_value();  // Set void promise
          } else {
            return_type result = std::apply(
                func, std::move(args_tuple));  // Invoke original function
            promise_ptr->set_value(
                std::move(result));  // Set promise with result
          }
        } catch (...) {
          promise_ptr->set_exception(
              std::current_exception());  // Set exception on promise
        }
      };  // End of task_payload lambda

  // 3. Create the type-erased task wrapper using the derived class template
  //    Get the concrete type of the task_payload lambda
  using PayloadType = decltype(task_payload);
  //    Create unique_ptr<task_base> holding task_derived<PayloadType>
  auto task_wrapper_ptr = std::make_unique<detail::task_derived<PayloadType>>(
      std::move(task_payload)  // Move the payload lambda into the wrapper
  );

  // 4. Enqueue the unique_ptr to the base class
  tasks_.enqueue(std::move(task_wrapper_ptr));

  // 5. Return the future
  return future;
}
}  // namespace toolbox::base
