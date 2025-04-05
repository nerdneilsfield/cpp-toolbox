#include <cstddef>
#include <exception>
#include <functional>
#include <optional>
#include <type_traits>
#include <typeinfo>
#include <memory>

#include "cpp-toolbox/container/concurrent_queue.hpp"
#include "cpp-toolbox/base/thread_pool.hpp"

// Include the third-party header ONLY in the .cpp file
#include <utility>  // For std::move

#include <concurrentqueue.h>  // Path depends on how you integrated it

template class CPP_TOOLBOX_EXPORT
    toolbox::container::concurrent_queue_t<std::unique_ptr<toolbox::base::detail::task_base>>;

namespace toolbox::container
{

// Define the implementation struct within the .cpp file
template<typename T>
struct concurrent_queue_t<T>::Impl
{
  // The actual concurrent queue instance from the library
  moodycamel::ConcurrentQueue<T> queue;

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

// --- Explicit Template Instantiation ---
// IMPORTANT: Because the implementation of the template methods is in this .cpp
// file, you MUST explicitly instantiate the template for EACH type `T` that you
// intend to use with `concurrent_queue_wrapper_t`. Otherwise, you will get
// linker errors. Add instantiations for all types you need here.

// Example instantiation for the type needed by the thread pool:
template class CPP_TOOLBOX_EXPORT
    toolbox::container::concurrent_queue_t<std::function<void()>>;

// Example instantiation for another type:
// template class CPP_TOOLBOX_EXPORT
// toolbox::container::concurrent_queue_t<int>; template class
// CPP_TOOLBOX_EXPORT toolbox::container::concurrent_queue_t<MyClass>;

}  // namespace toolbox::container