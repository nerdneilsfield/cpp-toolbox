#pragma once

#ifdef CPP_TOOLBOX_OLD_THREAD_POOL

#  include <vector>  // 用于存储工作线程/For storing worker threads
// #include <queue>             // 不再需要标准队列/No longer need standard
// queue
#  include <thread>  // C++ 线程库/C++ thread library
// #include <mutex>             // 不再需要互斥锁/No longer need mutex
// #include <condition_variable> // 不再需要条件变量/No longer need condition
// variable
#  include <atomic>  // 用于原子布尔标志/For atomic boolean flags
#  include <functional>  // 用于 std::function, std::bind/For std::function, std::bind
#  include <future>  // 用于异步获取任务结果/For async task results
#  include <iostream>  // 用于日志输出/For logging output
#  include <memory>  // 用于智能指针/For smart pointers
#  include <stdexcept>  // 用于异常处理/For exception handling
#  include <type_traits>  // 用于类型特征/For type traits
#  include <utility>  // 用于移动语义/For move semantics

#  include <cpp-toolbox/cpp-toolbox_export.hpp>
#  include <cpp-toolbox/macro.hpp>

#  include "cpp-toolbox/container/lock_free_queue.hpp"

namespace toolbox::base
{

/**
 * @brief 一个使用无锁队列的高性能 C++17 线程池实现/A high-performance C++17
 * thread pool implementation using lock-free queue
 *
 * @details
 * 该线程池使用无锁队列作为任务队列,支持异步提交任务并获取结果。它在构造时创建固定数量的工作线程,在析构时优雅地停止所有线程。
 * /This thread pool uses a lock-free queue for task scheduling, supports
 * asynchronous task submission and result retrieval. It creates a fixed number
 * of worker threads at construction and gracefully stops them at destruction.
 *
 * @example
 * @code
 * // 创建一个4线程的线程池/Create a thread pool with 4 threads
 * toolbox::base::thread_pool_t pool(4);
 *
 * // 提交一个返回整数的任务/Submit a task that returns an integer
 * auto future = pool.submit([]() {
 *   return 42;
 * });
 *
 * // 获取结果/Get the result
 * int result = future.get(); // result = 42
 *
 * // 提交带参数的任务/Submit a task with parameters
 * auto future2 = pool.submit([](int x, int y) {
 *   return x + y;
 * }, 3, 4);
 *
 * int sum = future2.get(); // sum = 7
 * @endcode
 */
class CPP_TOOLBOX_EXPORT thread_pool_t
{
public:
  /**
   * @brief 构造并启动线程池/Construct and start the thread pool
   *
   * @param threads 要创建的工作线程数量,默认为硬件并发数/Number of worker
   * threads to create, defaults to hardware concurrency
   * @throws std::invalid_argument 如果线程数为0/If thread count is 0
   */
  explicit thread_pool_t(size_t threads = 0);

  /**
   * @brief 析构函数,停止线程池并等待所有任务完成/Destructor that stops the pool
   * and waits for all tasks to complete
   */
  ~thread_pool_t();

  /**
   * @brief 提交任务到线程池执行/Submit a task to the thread pool
   *
   * @tparam F 任务函数类型/Task function type
   * @tparam Args 任务参数类型/Task argument types
   * @param f 任务函数/Task function
   * @param args 任务参数/Task arguments
   * @return std::future<typename std::invoke_result_t<F, Args...>>
   * 用于获取任务结果的future/Future for getting the task result
   * @throws std::runtime_error 如果线程池已停止/If the thread pool is stopped
   *
   * @example
   * @code
   * thread_pool_t pool(4);
   *
   * // 提交一个简单计算任务/Submit a simple calculation task
   * auto future = pool.submit([](int x) { return x * x; }, 5);
   * int result = future.get(); // result = 25
   *
   * // 提交一个字符串处理任务/Submit a string processing task
   * auto future2 = pool.submit([](std::string s) {
   *   std::transform(s.begin(), s.end(), s.begin(), ::toupper);
   *   return s;
   * }, "hello");
   * std::string result2 = future2.get(); // result2 = "HELLO"
   * @endcode
   */
  template<class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result_t<F, Args...>>;

  CPP_TOOLBOX_DISABLE_COPY(thread_pool_t)
  CPP_TOOLBOX_DISABLE_MOVE(thread_pool_t)

private:
  std::vector<std::thread> workers_;  // 工作线程列表/List of worker threads
  toolbox::container::lock_free_queue_t<std::function<void()>>
      tasks_;  // 无锁任务队列/Lock-free task queue
  std::atomic<bool> stop_;  // 停止标志/Stop flag
};

template<class F, class... Args>
auto thread_pool_t::submit(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result_t<F, Args...>>
{
  using return_type = typename std::invoke_result_t<F, Args...>;

  if (stop_.load(std::memory_order_relaxed))
  {  // 使用 relaxed 因为后续 enqueue 有自己的同步/Use relaxed since enqueue has
     // its own synchronization
    throw std::runtime_error(
        "在已停止的线程池上提交任务/Submitting task to stopped thread pool");
  }

  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();

  tasks_.enqueue(
      [task /* Capture shared_ptr */]()
      {
        try {
          (*task)();
        } catch (const std::future_error& fe) {
          std::cerr << "Worker lambda caught future_error for task "
                    << task.get() << ": " << fe.what() << std::endl;
        } catch (const std::exception& e) {
          std::cerr << "Worker lambda caught unexpected exception during "
                       "packaged_task invocation for task "
                    << task.get() << ": " << e.what() << std::endl;
        } catch (...) {
          std::cerr << "Worker lambda caught unknown exception during "
                       "packaged_task invocation for task "
                    << task.get() << std::endl;
        }
      });

  return res;
}

}  // namespace toolbox::base

#endif  // CPP_TOOLBOX_OLD_THREAD_POOL
