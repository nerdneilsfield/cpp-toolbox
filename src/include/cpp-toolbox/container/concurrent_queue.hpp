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
 * @brief A wrapper around a high-performance MPMC concurrent queue.
 *
 * This class provides a stable interface while hiding the underlying
 * third-party library implementation details using the Pimpl idiom.
 *
 * @tparam T The type of elements stored in the queue. Must be movable.
 *
 * @code{.cpp}
 * // Create a concurrent queue for integers
 * concurrent_queue_t<int> queue;
 *
 * // Producer thread
 * queue.enqueue(42);
 *
 * // Consumer thread
 * int value;
 * if(queue.try_dequeue(value)) {
 *   // Process value
 * }
 *
 * // Alternative consumer using optional
 * if(auto opt = queue.try_dequeue()) {
 *   int value = *opt;
 *   // Process value
 * }
 * @endcode
 */
template<typename T>
class CPP_TOOLBOX_EXPORT concurrent_queue_t
{
public:
  /**
   * @brief Constructs the queue wrapper.
   */
  concurrent_queue_t();

  /**
   * @brief Destroys the queue wrapper and cleans up resources.
   * @note Must be defined in the .cpp file due to Pimpl idiom with unique_ptr.
   */
  ~concurrent_queue_t();

  // --- Standard Queue Operations ---

  /**
   * @brief Enqueues an item into the queue.
   * Thread-safe for multiple producers.
   * @param value The value to enqueue (will be moved).
   */
  void enqueue(T&& value);

  /**
   * @brief Attempts to dequeue an item from the queue (non-blocking).
   * Thread-safe for multiple consumers.
   * @param[out] item Reference to store the dequeued value if successful.
   * @return True if an item was successfully dequeued, false if the queue was
   * empty.
   */
  bool try_dequeue(T& item);

  /**
   * @brief Attempts to dequeue an item, returning it in an std::optional
   * (non-blocking). Thread-safe for multiple consumers.
   * @return std::optional<T> containing the dequeued value if successful,
   * or std::nullopt if the queue was empty.
   */
  std::optional<T> try_dequeue();

  /**
   * @brief Attempts to dequeue an item, blocking until an item is available
   *        or the specified timeout elapses (non-blocking).
   * Thread-safe for multiple consumers.
   * @param[out] item Reference to store the dequeued value if successful.
   * @param timeout The maximum duration to wait.
   * @return True if an item was successfully dequeued within the timeout,
   *         false otherwise.
   */
  bool wait_dequeue_timed(T& item, std::chrono::microseconds timeout);

  // --- Additional Utility Functions (Exposed from underlying queue) ---

  /**
   * @brief Returns an approximate count of items in the queue.
   * Useful for heuristics but may not be exact in a highly concurrent scenario.
   * @return Approximate number of items in the queue.
   */
  size_t size_approx() const;

  // --- Resource Management ---

  // Disable copying and moving for simplicity.
  // If move semantics are desired, the move constructor/assignment
  // must also be defined in the .cpp file.
  CPP_TOOLBOX_DISABLE_COPY_AND_MOVE(concurrent_queue_t)

private:
  // Forward declaration of the implementation class (Pimpl)
  class Impl;

  // Pointer to the implementation details.
  // std::unique_ptr requires Impl to be defined where the destructor is called.
  std::unique_ptr<Impl> impl_;
};

extern template class CPP_TOOLBOX_EXPORT
    concurrent_queue_t<std::unique_ptr<toolbox::base::detail::task_base>>;
extern template class CPP_TOOLBOX_EXPORT
    concurrent_queue_t<std::function<void()>>;

}  // namespace toolbox::container