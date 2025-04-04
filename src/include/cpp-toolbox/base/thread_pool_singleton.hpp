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

class CPP_TOOLBOX_EXPORT thread_pool_singleton_t
{
public:
  // --- 单例访问 ---
  static thread_pool_singleton_t& instance()
  {
    static thread_pool_singleton_t instance;
    return instance;
  }

  // 删除拷贝和移动操作
  thread_pool_singleton_t(const thread_pool_singleton_t&) = delete;
  thread_pool_singleton_t& operator=(const thread_pool_singleton_t&) = delete;
  thread_pool_singleton_t(thread_pool_singleton_t&&) = delete;
  thread_pool_singleton_t& operator=(thread_pool_singleton_t&&) = delete;

  // --- 公共接口 ---

  /**
   * @brief 析构函数，停止线程池并等待所有工作线程结束。
   */
  ~thread_pool_singleton_t();

  /**
   * @brief 提交一个任务到线程池执行。
   * (实现保持在头文件中，因为它是一个模板)
   * @return std::future<...> 用于获取结果或异常。
   * @throws std::runtime_error 如果线程池已停止。
   */
  template<class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result_t<F, Args...>>;

  /**
   * @brief 获取线程池中的工作线程数量。
   * @return size_t 线程数量。
   */
  size_t get_thread_count() const { return workers_.size(); }

private:
  // 构造函数声明
  thread_pool_singleton_t();

  std::vector<std::thread> workers_;
  toolbox::container::concurrent_queue_t<std::function<void()>> tasks_;
  std::atomic<bool> stop_;

  // --- 辅助函数 ---
  /**
   * @brief 初始化并启动线程池（在单例首次访问时调用）。
   * @param threads 线程数，0 表示默认。
   */
  void start_pool(size_t threads = 0);

  /**
   * @brief 停止并销毁线程池资源。
   */
  void stop_pool();

  /**
   * @brief 工作线程执行的主循环函数。
   * @param worker_id 工作线程的ID（用于日志/调试）。
   */
  void worker_loop(size_t worker_id);

};  // class thread_pool_singleton_t

// --- Template Member Function Implementation (保持在头文件) ---

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

  tasks_.enqueue([task]() mutable { // mutable 如果 F 是可变 lambda
        try {
            (*task)();
        } catch (const std::future_error& fe) {
            // 可以考虑使用 logger 记录异常
            LOG_ERROR_S << "Worker caught future_error: " << fe.what();
            // std::cerr << "Worker caught future_error: " << fe.what() << std::endl;
        } catch (const std::exception& e) {
            LOG_ERROR_S << "Worker caught exception: " << e.what();
            // std::cerr << "Worker caught exception: " << e.what() << std::endl;
        } catch (...) {
            LOG_ERROR_S << "Worker caught unknown exception.";
            // std::cerr << "Worker caught unknown exception." << std::endl;
        }
    });

  return res;
}

}  // namespace toolbox::base
