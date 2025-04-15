#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>  // For potential exceptions if needed
#include <vector>  // Alternative storage

namespace toolbox::base
{

template<typename T>
class object_pool_t;  // Forward declaration

/**
 * @brief Custom deleter for std::unique_ptr to return objects to an
 * object_pool_t.
 * @tparam T The type of object managed by the pool.
 */
template<typename T>
class PoolDeleter
{
  object_pool_t<T>* pool_ = nullptr;  // Non-owning pointer to the pool

public:
  /**
   * @brief Construct a PoolDeleter.
   * @param pool Pointer to the object pool to release objects back to.
   */
  explicit PoolDeleter(object_pool_t<T>* pool = nullptr)
      : pool_(pool)
  {
  }

  /**
   * @brief The function called by std::unique_ptr instead of delete.
   *        Releases the object back to the pool if the pool pointer is valid.
   * @param ptr Raw pointer to the object being managed.
   */
  void operator()(T* ptr) const
  {
    if (pool_ && ptr) {
      pool_->release(ptr);  // Tell the pool to take it back
    } else {
      // If no pool is associated, perform default delete.
      // This handles cases where acquire might create a new object
      // outside the pool's direct management or if the pool is destroyed early.
      delete ptr;
    }
  }
};

/**
 * @brief A thread-safe object pool template.
 *
 * Manages a pool of reusable objects of type T to reduce allocation overhead.
 * Uses RAII via std::unique_ptr with a custom deleter to manage object
 * lifetimes.
 *
 * @tparam T The type of object to pool. Must be default-constructible.
 */
template<typename T>
class object_pool_t
{
public:
  /**
   * @brief Alias for a unique_ptr managing a pooled object.
   * Automatically returns the object to the pool when it goes out of scope.
   */
  using PooledObjectPtr = std::unique_ptr<T, PoolDeleter<T>>;

  /**
   * @brief Construct an object pool.
   * @param initial_size Number of objects to create initially. Defaults to 0.
   * @param resetter Optional function to call on an object to reset its state
   *                 before returning it to the pool.
   */
  explicit object_pool_t(size_t initial_size = 0,
                         std::function<void(T&)> resetter = nullptr)
      : reset_func_(std::move(resetter))
  {
    for (size_t i = 0; i < initial_size; ++i) {
      pool_.push(std::make_unique<T>());
    }
  }

  // Deleted copy/move operations for simplicity
  object_pool_t(const object_pool_t&) = delete;
  object_pool_t& operator=(const object_pool_t&) = delete;
  object_pool_t(object_pool_t&&) = delete;
  object_pool_t& operator=(object_pool_t&&) = delete;

  /**
   * @brief Acquire an object from the pool.
   *
   * If the pool is empty, a new object is created.
   * The returned unique_ptr will automatically release the object back
   * to the pool upon destruction.
   *
   * @return A std::unique_ptr managing the acquired object.
   */
  PooledObjectPtr acquire()
  {
    std::unique_ptr<T> obj_ptr =
        nullptr;  // Use unique_ptr for temporary ownership

    {  // Lock scope
      std::lock_guard<std::mutex> lock(mutex_);
      if (!pool_.empty()) {
        obj_ptr = std::move(pool_.front());  // Take from pool
        pool_.pop();
      }
    }  // Unlock mutex before potentially creating a new object

    if (!obj_ptr) {  // If pool was empty
      obj_ptr = std::make_unique<T>();  // Create a new one
    }

    // Return the object wrapped in a unique_ptr with our custom deleter
    return PooledObjectPtr(obj_ptr.release(), PoolDeleter<T>(this));
  }

  /**
   * @brief Release an object back to the pool.
   *
   * Called automatically by the PoolDeleter when the unique_ptr goes out of
   * scope. Resets the object's state if a reset function was provided.
   *
   * @param ptr Raw pointer to the object to release. Ownership is taken by the
   * pool.
   */
  void release(T* ptr)
  {
    if (!ptr)
      return;

    // Reset object state if a reset function is available
    if (reset_func_) {
      try {
        reset_func_(*ptr);
      } catch (const std::exception& e) {
        // Log error? Don't let exceptions from resetter break the pool.
        // Consider logging using a mechanism that doesn't use this pool.
        fprintf(stderr, "[ObjectPool] Exception during reset: %s\n", e.what());
        // Decide whether to still pool the object or delete it.
        // Deleting might be safer if reset failed critically.
        delete ptr;
        return;
      } catch (...) {
        fprintf(stderr, "[ObjectPool] Unknown exception during reset.\n");
        delete ptr;
        return;
      }
    }

    std::unique_ptr<T> obj_ptr(ptr);  // Re-wrap raw pointer to manage ownership
    {  // Lock scope
      std::lock_guard<std::mutex> lock(mutex_);
      pool_.push(std::move(obj_ptr));  // Add unique_ptr back to the pool
    }
  }

  // Optional: Add methods for pool statistics (size, available), clear, etc.
  // size_t size() const { std::lock_guard<std::mutex> lock(mutex_); return
  // pool_.size(); }

private:
  friend class PoolDeleter<T>;  // Allow deleter to call release

  std::queue<std::unique_ptr<T>> pool_;
  mutable std::mutex mutex_;  // Marked mutable if size() is const
  std::function<void(T&)> reset_func_;
};

}  // namespace toolbox::base
