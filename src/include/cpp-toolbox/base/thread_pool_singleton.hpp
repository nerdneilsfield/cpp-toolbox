#pragma once

#include <future>
#include <type_traits>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/macro.hpp>

#include "cpp-toolbox/base/thread_pool.hpp"

namespace toolbox::base
{

/**
 * @brief 简单的线程池单例封装/A simple singleton wrapper around thread_pool_t
 */
class CPP_TOOLBOX_EXPORT thread_pool_singleton_t
{
public:
  /**
   * @brief 获取单例实例/Get the singleton instance
   */
  static thread_pool_singleton_t& instance()
  {
    static thread_pool_singleton_t inst;
    return inst;
  }

  /**
   * @brief 提交任务/Submit a task
   */
  template<class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result_t<F, Args...>>
  {
    return pool_.submit(std::forward<F>(f), std::forward<Args>(args)...);
  }

  /**
   * @brief 获取线程数量/Get underlying worker count
   */
  size_t get_thread_count() const { return pool_.get_thread_count(); }

private:
  thread_pool_singleton_t() = default;
  ~thread_pool_singleton_t() = default;

  CPP_TOOLBOX_DISABLE_COPY(thread_pool_singleton_t)
  CPP_TOOLBOX_DISABLE_MOVE(thread_pool_singleton_t)

  thread_pool_t pool_{};
};

}  // namespace toolbox::base
