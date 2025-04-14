#include <chrono>  // Needed for timeout
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "cpp-toolbox/container/concurrent_queue.hpp"

#include "cpp-toolbox/base/thread_pool.hpp"
#include "cpp-toolbox/logger/thread_logger.hpp"

// Include the third-party header ONLY in the .cpp file
#include <blockingconcurrentqueue.h>  // Path depends on how you integrated it

namespace toolbox::container
{

// Define the implementation struct within the .cpp file
template<typename T>
struct concurrent_queue_t<T>::Impl
{
  // Use the Blocking version of the queue to access blocking methods
  moodycamel::BlockingConcurrentQueue<T> queue;
  // Remove the unnecessary consumer token
  // moodycamel::ConsumerToken consumer_token{queue};

  // Constructor (optional, default is likely fine)
  Impl() = default;
};

// --- Constructor ---
// Must allocate the Impl object
template<typename T>
concurrent_queue_t<T>::concurrent_queue_t()
    : impl_(std::make_unique<Impl>())
{
}

// --- Destructor ---
// Must be defined here, even if empty, so unique_ptr can see Impl's definition
template<typename T>
concurrent_queue_t<T>::~concurrent_queue_t() = default;

// --- Method Implementations ---
// Forward calls to the underlying queue stored in impl_

template<typename T>
void concurrent_queue_t<T>::enqueue(T&& item)
{
  impl_->queue.enqueue(std::forward<T>(item));
}

template<typename T>
bool concurrent_queue_t<T>::try_dequeue(T& item)
{
  return impl_->queue.try_dequeue(item);
}

template<typename T>
std::optional<T> concurrent_queue_t<T>::try_dequeue()
{
  T item;
  if (impl_->queue.try_dequeue(item)) {
    return std::optional<T>(std::move(item));
  }
  return std::nullopt;
}

template<typename T>
size_t concurrent_queue_t<T>::size_approx() const
{
  return impl_->queue.size_approx();
}

// --- Implementation for wait_dequeue_timed ---
template<typename T>
bool concurrent_queue_t<T>::wait_dequeue_timed(
    T& item, std::chrono::microseconds timeout)
{
  // Call the correct moodycamel API, converting duration to microseconds count
  return impl_->queue.wait_dequeue_timed(
      item, static_cast<std::uint64_t>(timeout.count()));
}

// --- Explicit Template Instantiations ---
// Define explicit instantiations requested in the header.
// Do NOT repeat CPP_TOOLBOX_EXPORT here, it's on the class definition.

using TaskPtr = std::unique_ptr<toolbox::base::detail::task_base>;
using VoidFunc = std::function<void()>;
using LogEntry =
    std::pair<toolbox::logger::thread_logger_t::Level, std::string>;

template class toolbox::container::concurrent_queue_t<TaskPtr>;
template class toolbox::container::concurrent_queue_t<VoidFunc>;
template class toolbox::container::concurrent_queue_t<LogEntry>;

}  // namespace toolbox::container