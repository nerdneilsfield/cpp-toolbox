#pragma once

#include <atomic>  // 用于原子布尔标志/For atomic boolean flag
#include <functional>  // 用于 std::function, std::bind/For std::function, std::bind
#include <future>  // 用于异步任务结果/For asynchronous task results (std::future, std::packaged_task)
#include <iostream>  // 用于标准输入输出/For std::cout, std::cerr
#include <memory>  // 用于智能指针/For std::make_shared
#include <stdexcept>  // 用于运行时异常/For runtime exceptions
#include <thread>  // C++ 线程库/C++ thread library
#include <type_traits>  // 用于类型特征/For std::invoke_result_t
#include <utility>  // 用于完美转发和移动语义/For std::forward, std::move
#include <vector>  // 用于存储工作线程/For storing worker threads

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/macro.hpp>

#include "cpp-toolbox/container/concurrent_queue.hpp"
#include "cpp-toolbox/logger/thread_logger.hpp"

namespace toolbox::base
{

/**
 * @brief 一个管理工作线程池的单例线程池实现/A singleton thread pool
 * implementation that manages a pool of worker threads
 *
 * @details
 * 该类提供了一个线程安全的单例线程池,可以异步执行任务。任务以函数/lambda的形式提交,并由工作线程执行。
 * /This class provides a thread-safe singleton thread pool that can execute
 * tasks asynchronously. Tasks are submitted as functions/lambdas and are queued
 * for execution by worker threads.
 *
 * @example
 * @code{.cpp}
 * // 获取线程池实例/Get thread pool instance
 * auto& pool = thread_pool_singleton_t::instance();
 *
 * // 提交一个简单任务/Submit a simple task
 * auto future1 = pool.submit([]() { return "Hello"; });
 * std::string result1 = future1.get(); // result1 = "Hello"
 *
 * // 提交带参数的任务/Submit a task with parameters
 * auto future2 = pool.submit([](int x, int y) { return x + y; }, 2, 3);
 * int result2 = future2.get(); // result2 = 5
 *
 * // 提交一个复杂任务/Submit a complex task
 * auto future3 = pool.submit([](std::vector<int> v) {
 *     int sum = 0;
 *     for(auto i : v) sum += i;
 *     return sum;
 * }, std::vector<int>{1,2,3,4,5});
 * int result3 = future3.get(); // result3 = 15
 * @endcode
 */
class CPP_TOOLBOX_EXPORT thread_pool_singleton_t
{
public:
  /**
   * @brief 获取线程池的单例实例/Get the singleton instance of the thread pool
   * @return 返回单例线程池实例的引用/Reference to the singleton thread pool
   * instance
   */
  static thread_pool_singleton_t& instance()
  {
    static thread_pool_singleton_t instance;
    return instance;
  }

  // 删除拷贝和移动操作以确保单例模式/Delete copy and move operations to ensure
  // singleton pattern
  thread_pool_singleton_t(const thread_pool_singleton_t&) = delete;
  thread_pool_singleton_t& operator=(const thread_pool_singleton_t&) = delete;
  thread_pool_singleton_t(thread_pool_singleton_t&&) = delete;
  thread_pool_singleton_t& operator=(thread_pool_singleton_t&&) = delete;

  /**
   * @brief 析构函数,停止线程池并等待所有工作线程完成/Destructor that stops the
   * thread pool and waits for all worker threads to finish
   */
  ~thread_pool_singleton_t();

  /**
   * @brief 向线程池提交任务/Submit a task to be executed by the thread pool
   *
   * @details
   * 该方法将任务加入队列以供工作线程异步执行。任务可以是函数、lambda或任何可调用对象。
   * /This method queues a task for asynchronous execution by one of the worker
   * threads. The task can be a function, lambda, or any callable object.
   *
   * @tparam F 可调用对象的类型/Type of the callable
   * @tparam Args 参数类型包/Types of the arguments
   * @param f 要执行的可调用对象/The callable to execute
   * @param args 传递给可调用对象的参数/Arguments to pass to the callable
   * @return 包含最终结果的 std::future/std::future containing the eventual
   * result
   * @throws std::runtime_error 如果线程池已停止/if the thread pool has been
   * stopped
   *
   * @example
   * @code{.cpp}
   * // 提交无参数的lambda/Submit lambda without parameters
   * auto future1 = pool.submit([]() {
   *     return std::string("Task completed");
   * });
   *
   * // 提交带多个参数的函数/Submit function with multiple parameters
   * auto future2 = pool.submit([](int x, std::string y) {
   *     return y + std::to_string(x);
   * }, 42, "Answer: ");
   * @endcode
   */
  template<class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result_t<F, Args...>>;

  /**
   * @brief 获取线程池中的工作线程数量/Get the number of worker threads in the
   * pool
   * @return 工作线程数量/The number of worker threads
   */
  size_t get_thread_count() const
  {
    return workers_.size();
  }

private:
  /**
   * @brief 单例模式的私有构造函数/Private constructor for singleton pattern
   */
  thread_pool_singleton_t();

  // 成员变量/Member variables
  std::vector<std::thread>
      workers_;  // 工作线程容器/Container for worker threads
  toolbox::container::concurrent_queue_t<std::function<void()>>
      tasks_;  // 任务队列/Task queue
  std::atomic<bool> stop_;  // 停止标志/Stop flag

  /**
   * @brief 初始化并启动线程池/Initialize and start the thread pool
   * @param threads 要创建的线程数量(0表示使用硬件并发数)/Number of threads to
   * create (0 means use hardware concurrency)
   */
  void start_pool(size_t threads = 0);

  /**
   * @brief 停止线程池并清理资源/Stop the thread pool and cleanup resources
   */
  void stop_pool();

  /**
   * @brief 工作线程的主循环函数/Main worker thread function that processes
   * tasks
   * @param worker_id 用于日志记录和调试的工作线程ID/ID of the worker thread for
   * logging/debugging
   */
  void worker_loop(size_t worker_id);

};  // class thread_pool_singleton_t

// 模板实现/Template implementation

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
