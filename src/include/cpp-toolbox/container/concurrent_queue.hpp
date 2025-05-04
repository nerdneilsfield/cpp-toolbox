#pragma once

#include <chrono>  // For std::chrono::microseconds
#include <cstddef>  // For size_t
#include <functional>  // Needed for std::function template instantiation
#include <memory>  // For std::unique_ptr
#include <optional>

#include "cpp-toolbox/base/detail/task_base.hpp"
// Assuming export macros are defined elsewhere if needed
#include <cpp-toolbox/cpp-toolbox_export.hpp>
// Assuming disable copy/move macros are defined elsewhere
#include <cpp-toolbox/macro.hpp>

namespace toolbox::container
{

/**
 * @brief 高性能MPMC并发队列的包装器/A wrapper around a high-performance MPMC
 * concurrent queue
 *
 * @details 该类使用PIMPL惯用法隐藏底层第三方库实现细节，同时提供稳定的接口/This
 * class provides a stable interface while hiding the underlying third-party
 * library implementation details using the Pimpl idiom
 *
 * @tparam T 队列中存储的元素类型，必须是可移动的/The type of elements stored in
 * the queue. Must be movable
 *
 * @code{.cpp}
 * // 创建一个整数类型的并发队列/Create a concurrent queue for integers
 * concurrent_queue_t<int> queue;
 *
 * // 生产者线程/Producer thread
 * queue.enqueue(42);
 *
 * // 消费者线程 - 使用引用方式/Consumer thread - using reference
 * int value;
 * if(queue.try_dequeue(value)) {
 *   // 处理值/Process value
 * }
 *
 * // 消费者线程 - 使用optional方式/Consumer thread - using optional
 * if(auto opt = queue.try_dequeue()) {
 *   int value = *opt;
 *   // 处理值/Process value
 * }
 *
 * // 带超时的消费/Timed consumption
 * int value;
 * if(queue.wait_dequeue_timed(value, std::chrono::microseconds(1000))) {
 *   // 在1ms内成功获取到值/Successfully got value within 1ms
 * }
 * @endcode
 */
template<typename T>
class CPP_TOOLBOX_EXPORT concurrent_queue_t
{
public:
  /**
   * @brief 构造队列包装器/Constructs the queue wrapper
   */
  concurrent_queue_t();

  /**
   * @brief 销毁队列包装器并清理资源/Destroys the queue wrapper and cleans up
   * resources
   * @note 由于使用了PIMPL惯用法和unique_ptr，必须在cpp文件中定义/Must be
   * defined in the .cpp file due to Pimpl idiom with unique_ptr
   */
  ~concurrent_queue_t();

  // --- 标准队列操作/Standard Queue Operations ---

  /**
   * @brief 将元素入队/Enqueues an item into the queue
   * @details 支持多生产者线程安全/Thread-safe for multiple producers
   * @param value 要入队的值(将被移动)/The value to enqueue (will be moved)
   */
  void enqueue(T&& value);

  /**
   * @brief 尝试从队列中出队一个元素(非阻塞)/Attempts to dequeue an item from
   * the queue (non-blocking)
   * @details 支持多消费者线程安全/Thread-safe for multiple consumers
   * @param[out] item 如果成功，用于存储出队值的引用/Reference to store the
   * dequeued value if successful
   * @return 如果成功出队返回true，队列为空返回false/True if an item was
   * successfully dequeued, false if the queue was empty
   */
  bool try_dequeue(T& item);

  /**
   * @brief 尝试出队一个元素，以std::optional形式返回(非阻塞)/Attempts to
   * dequeue an item, returning it in an std::optional (non-blocking)
   * @details 支持多消费者线程安全/Thread-safe for multiple consumers
   * @return
   * 如果成功包含出队值的std::optional<T>，否则返回std::nullopt/std::optional<T>
   * containing the dequeued value if successful, or std::nullopt if the queue
   * was empty
   */
  std::optional<T> try_dequeue();

  /**
   * @brief 尝试出队一个元素，阻塞直到有元素可用或超时/Attempts to dequeue an
   * item, blocking until an item is available or timeout
   * @details 支持多消费者线程安全/Thread-safe for multiple consumers
   * @param[out] item 如果成功，用于存储出队值的引用/Reference to store the
   * dequeued value if successful
   * @param timeout 最大等待时间/The maximum duration to wait
   * @return 如果在超时前成功出队返回true，否则返回false/True if an item was
   * successfully dequeued within the timeout, false otherwise
   */
  bool wait_dequeue_timed(T& item, std::chrono::microseconds timeout);

  // --- 额外工具函数/Additional Utility Functions ---

  /**
   * @brief 返回队列中元素的近似数量/Returns an approximate count of items in
   * the queue
   * @details 适用于启发式估计，在高并发场景下可能不精确/Useful for heuristics
   * but may not be exact in a highly concurrent scenario
   * @return 队列中元素的近似数量/Approximate number of items in the queue
   */
  size_t size_approx() const;

  // --- 资源管理/Resource Management ---

  // 为简化起见禁用拷贝和移动/Disable copying and moving for simplicity
  // 如果需要移动语义，移动构造函数/赋值运算符必须在cpp文件中定义/If move
  // semantics are desired, the move constructor/assignment must also be defined
  // in the .cpp file
  CPP_TOOLBOX_DISABLE_COPY_AND_MOVE(concurrent_queue_t)

private:
  // 实现类的前向声明(PIMPL)/Forward declaration of the implementation class
  // (Pimpl)
  class Impl;

  // 指向实现细节的指针/Pointer to the implementation details
  // std::unique_ptr要求Impl在调用析构函数的地方定义/std::unique_ptr requires
  // Impl to be defined where the destructor is called
  std::unique_ptr<Impl> impl_;
};

#if defined(CPP_TOOLBOX_COMPILER_MSVC)
extern template class concurrent_queue_t<
    std::unique_ptr<toolbox::base::detail::task_base,
                    std::default_delete<toolbox::base::detail::task_base>>>;
extern template class concurrent_queue_t<std::function<void()>>;

#else
extern template class CPP_TOOLBOX_EXPORT concurrent_queue_t<
    std::unique_ptr<toolbox::base::detail::task_base,
                    std::default_delete<toolbox::base::detail::task_base>>>;
extern template class CPP_TOOLBOX_EXPORT
    concurrent_queue_t<std::function<void()>>;
// extern template class /* CPP_TOOLBOX_EXPORT */
//     concurrent_queue_t<
//         std::pair<toolbox::logger::thread_logger_t::Level, std::string>>;
#endif
}  // namespace toolbox::container