#pragma once

#include <atomic>  // 用于原子布尔标志/For atomic boolean flag
#include <functional>  // 用于 std::function, std::bind/For std::function, std::bind
#include <future>  // 用于异步任务结果/For asynchronous task results (std::future, std::packaged_task)
#include <iostream>  // 用于标准输入输出/For std::cout, std::cerr
#include <memory>  // 用于智能指针/For std::make_shared
#include <stdexcept>  // 用于运行时异常/For runtime exceptions
#include <thread>  // C++ 线程库/C++ thread library
#include <deque>   // 任务双端队列/For task deques
#include <mutex>   // 互斥锁/For std::mutex
#include <type_traits>  // 用于类型特征/For std::invoke_result_t
#include <utility>  // 用于完美转发和移动语义/For std::forward, std::move
#include <vector>  // 用于存储工作线程/For storing worker threads

#include "cpp-toolbox/base/detail/task_base.hpp"
// 导出宏定义/Export macro definition
#include <cpp-toolbox/cpp-toolbox_export.hpp>
// 宏定义/Macro definitions
#include <cpp-toolbox/macro.hpp>

namespace toolbox::base
{

/**
 * @brief 支持任务窃取的简单 C++17 线程池实现/A simple C++17 thread pool
 * implementation with basic work stealing
 *
 * @details
 * 该线程池允许提交任务并异步获取结果。构造时创建固定数量的工作线程并为每个线程分配本地双端队列。
 * 当本地队列为空时,线程会尝试从其他线程窃取任务。/This thread pool allows
 * submitting tasks and asynchronously retrieving results. Each worker has a
 * local deque and will try to steal tasks from others when idle.
 *
 * @example
 * @code{.cpp}
 * // 创建一个默认线程数的线程池/Create a thread pool with default number of
 * threads thread_pool_t pool;
 *
 * // 提交一个简单任务/Submit a simple task
 * auto future = pool.submit([]() { return 42; });
 * int result = future.get(); // result will be 42
 *
 * // 提交带参数的任务/Submit a task with arguments
 * auto future2 = pool.submit([](int a, int b) { return a + b; }, 10, 20);
 * int result2 = future2.get(); // result2 will be 30
 *
 * // 提交一个可能抛出异常的任务/Submit a task that may throw
 * auto future3 = pool.submit([]() {
 *     throw std::runtime_error("Task failed");
 * });
 * try {
 *     future3.get();
 * } catch(const std::exception& e) {
 *     std::cerr << e.what() << std::endl;
 * }
 * @endcode
 */
class CPP_TOOLBOX_EXPORT thread_pool_t
{
public:
  /**
   * @brief 构造并初始化线程池/Constructs and initializes the thread pool
   *
   * @param threads
   * 要创建的工作线程数量,默认为硬件并发数。如果无法确定硬件并发数或为0,则至少创建1个线程/Number
   * of worker threads to create. Defaults to hardware concurrency. If hardware
   * concurrency cannot be determined or is 0, creates at least 1 thread
   * @throws std::invalid_argument 如果线程数参数为0/if thread count parameter
   * is 0
   *
   * @example
   * @code{.cpp}
   * // 创建4线程的线程池/Create pool with 4 threads
   * thread_pool_t pool(4);
   *
   * // 创建默认线程数的线程池/Create pool with default number of threads
   * thread_pool_t default_pool;
   * @endcode
   */
  explicit thread_pool_t(size_t threads = 0);

  /**
   * @brief 析构函数,停止线程池并等待所有工作线程完成/Destructor that stops the
   * thread pool and waits for all worker threads to finish
   *
   * @details
   * 等待线程已经取出的任务完成。析构后线程池将不可用/Waits for tasks already
   * dequeued by threads to complete. After destruction, the thread pool becomes
   * unusable
   */
  ~thread_pool_t();

  /**
   * @brief 获取线程池中的工作线程数量/Get the number of worker threads
   * @return 当前工作线程数量/The current number of worker threads
   */
  size_t get_thread_count() const { return workers_.size(); }

  /**
   * @brief 向线程池提交任务以供执行/Submits a task to the thread pool for
   * execution
   *
   * @tparam F 任务函数的类型/Type of the task function
   * @tparam Args 任务函数参数的类型/Types of the task function arguments
   * @param f
   * 任务函数(可以是函数指针、lambda、std::function或任何可调用对象)/Task
   * function (can be function pointer, lambda, std::function, or any callable
   * object)
   * @param args 传递给任务函数的参数/Arguments to pass to the task function
   * @return std::future 对象,可用于获取任务的返回值或捕获其异常/std::future
   * object that can be used to get the task's return value or catch its
   * exceptions
   * @throws std::runtime_error 如果线程池已停止且无法接受新任务/if the thread
   * pool has been stopped and cannot accept new tasks
   *
   * @example
   * @code{.cpp}
   * // 提交一个lambda任务/Submit a lambda task
   * auto future = pool.submit([]() { return 42; });
   *
   * // 提交带参数的任务/Submit a task with arguments
   * auto future2 = pool.submit([](int a, int b) { return a + b; }, 10, 20);
   *
   * // 提交一个抛出异常的任务/Submit a task that throws an exception
   * auto future3 = pool.submit([]() { throw std::runtime_error("error"); });
   * try {
   *     future3.get();
   * } catch (const std::exception& e) {
   *     // 处理异常/Handle exception
   * }
   * @endcode
   */
  template<class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result_t<F, Args...>>;

  // 删除拷贝构造函数和拷贝赋值运算符以防止意外复制/Delete copy constructor and
  // copy assignment operator to prevent accidental copying
  CPP_TOOLBOX_DISABLE_COPY(thread_pool_t)
  // 删除移动构造函数和移动赋值运算符以简化生命周期管理/Delete move constructor
  // and move assignment operator to simplify lifecycle management
  CPP_TOOLBOX_DISABLE_MOVE(thread_pool_t)

private:
  // 工作线程列表/List of worker threads
  std::vector<std::thread> workers_;
  // 每个工作线程的任务双端队列/Per worker task deque
  std::vector<std::deque<std::unique_ptr<detail::task_base>>> worker_queues_;
  // 保护每个双端队列的互斥锁/Mutex protecting each deque
  std::vector<std::unique_ptr<std::mutex>> queue_mutexes_;
  // 提交任务时下一个目标线程索引/Next worker index for task submission
  std::atomic<size_t> next_worker_{0};
  // 指示线程池是否应该停止的原子标志/Atomic flag indicating whether the thread
  // pool should stop
  std::atomic<bool> stop_;

  // 工作线程主循环/Worker loop implementing work stealing
  void worker_loop(size_t worker_id);
};

// --- 模板成员函数实现/Template Member Function Implementation ---

template<class F, class... Args>
auto thread_pool_t::submit(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result_t<F, Args...>>
{
  using return_type = typename std::invoke_result_t<F, Args...>;

  if (stop_.load(std::memory_order_relaxed)) {
    throw std::runtime_error("Cannot submit task to stopped thread pool");
  }

  // 1. 创建 promise 并获取 future/Create promise and get future
  auto promise = std::make_shared<std::promise<return_type>>();
  std::future<return_type> future = promise->get_future();

  // 2. 创建执行工作并设置 promise 的 lambda/Create the lambda that does the
  // work and sets the promise
  auto task_payload =
      // 如果 lambda 需要修改其捕获的变量则使用 mutable/Use mutable if the
      // lambda needs to modify its captures
      [func = std::forward<F>(
           f),  // 移动/转发原始可调用对象/Move/forward the original callable
       args_tuple = std::make_tuple(
           std::forward<Args>(args)...),  // 移动/转发参数/Move/forward args
       promise_ptr = std::move(
           promise)]() mutable {  // 通过移动的 shared_ptr 捕获 promise/Capture
                                  // promise by moved shared_ptr
        try {
          if constexpr (std::is_void_v<return_type>) {
            std::apply(
                func,
                std::move(
                    args_tuple));  // 调用原始函数/Invoke original function
            promise_ptr->set_value();  // 设置 void promise/Set void promise
          } else {
            return_type result = std::apply(
                func,
                std::move(
                    args_tuple));  // 调用原始函数/Invoke original function
            promise_ptr->set_value(std::move(
                result));  // 用结果设置 promise/Set promise with result
          }
        } catch (...) {
          promise_ptr->set_exception(
              std::current_exception());  // 在 promise 上设置异常/Set exception
                                          // on promise
        }
      };  // lambda 结束/End of task_payload lambda

  // 3. 使用派生类模板创建类型擦除的任务包装器/Create the type-erased task
  // wrapper using the derived class template
  //    获取任务负载 lambda 的具体类型/Get the concrete type of the task_payload
  //    lambda
  using PayloadType = decltype(task_payload);
  //    创建持有 task_derived<PayloadType> 的 unique_ptr<task_base>/Create
  //    unique_ptr<task_base> holding task_derived<PayloadType>
  auto task_wrapper_ptr = std::make_unique<detail::task_derived<PayloadType>>(
      std::move(task_payload)  // 将负载 lambda 移动到包装器中/Move the payload
                               // lambda into the wrapper
  );

  // 4. 将任务放入某个工作线程的本地队列/Push the task to a worker's local deque
  size_t idx = next_worker_.fetch_add(1, std::memory_order_relaxed) %
               workers_.size();
  {
    std::lock_guard<std::mutex> lock(*queue_mutexes_[idx]);
    worker_queues_[idx].push_back(std::move(task_wrapper_ptr));
  }

  // 5. 返回 future/Return the future
  return future;
}
}  // namespace toolbox::base
