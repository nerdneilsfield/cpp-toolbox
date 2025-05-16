#pragma once

#include <chrono>
#include <string>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::utils
{

/**
 * @class stop_watch_timer_t
 * @brief A high-resolution stopwatch timer for measuring elapsed time.
 *
 * This class provides a simple interface for measuring elapsed time with
 * nanosecond precision. It supports starting, stopping, and resetting the
 * timer, as well as retrieving elapsed time in milliseconds or seconds.
 *
 * @code
 * // Basic usage
 * stop_watch_timer_t timer("MyTimer");
 * timer.start();
 * // ... perform some operations ...
 * timer.stop();
 * double elapsed_ms = timer.elapsed_time_ms();
 * timer.print_stats();
 *
 * // Multiple start/stop cycles
 * stop_watch_timer_t timer;
 * timer.start();
 * // ... first operation ...
 * timer.stop();
 * timer.start();
 * // ... second operation ...
 * timer.stop();
 * double total_seconds = timer.elapsed_time();
 * @endcode
 */
class CPP_TOOLBOX_EXPORT stop_watch_timer_t
{
public:
  /**
   * @brief Construct a new stop_watch_timer_t object
   * @param name The name of the timer (default: "default_timer")
   */
  stop_watch_timer_t(std::string name = "default_timer");

  /**
   * @brief Set the name of the timer
   * @param name The new name for the timer
   */
  void set_name(const std::string& name);

  /**
   * @brief Get the name of the timer
   * @return const std::string& The current name of the timer
   */
  auto get_name() const -> const std::string&;

  /**
   * @brief Start or resume the timer
   * @note If the timer is already running, this will restart the timing
   */
  void start();

  /**
   * @brief Stop (pause) the timer and accumulate the duration
   * @note If the timer is not running, this function does nothing
   */
  void stop();

  /**
   * @brief Reset the timer to zero
   * @note This also stops the timer if it was running
   */
  void reset();

  /**
   * @brief Get the total elapsed time in milliseconds
   * @return double The elapsed time in milliseconds
   * @note This includes all accumulated time between start/stop cycles
   */
  auto elapsed_time_ms() const -> double;

  /**
   * @brief Get the total elapsed time in seconds
   * @return double The elapsed time in seconds
   * @note This includes all accumulated time between start/stop cycles
   */
  auto elapsed_time() const -> double;

  /**
   * @brief Print the timer statistics to standard output
   * @details Prints the timer name and elapsed time in a human-readable format
   */
  void print_stats() const;

private:
  std::string name_;  ///< The name of the timer
  std::chrono::nanoseconds total_duration_;  ///< Accumulated duration
  std::chrono::time_point<std::chrono::high_resolution_clock>
      start_time_;  ///< Last start time
  bool running_;  ///< Timer running state (true if running)
};

/**
 * @class simple_timer_interface_t
 * @brief Interface for managing a collection of timers
 *
 * This interface provides a unified way to manage multiple timers through
 * a single interface. It supports starting, stopping, resetting, and querying
 * timers by index.
 *
 * @code
 * // Basic usage
 * simple_timer_interface_t* timers = create_timer(3);
 * timers->start(0);
 * // ... perform operation ...
 * timers->stop(0);
 * double elapsed = timers->elapsed_time_ms(0);
 * timers->print_all_stats();
 * delete_timer(&timers);
 * @endcode
 */
class CPP_TOOLBOX_EXPORT simple_timer_interface_t
{
public:
  /**
   * @brief Virtual destructor to ensure proper cleanup
   */
  virtual ~simple_timer_interface_t() = default;

  /**
   * @brief Start the timer at specified index
   * @param index The index of the timer to start
   */
  virtual void start(int index) = 0;

  /**
   * @brief Stop the timer at specified index
   * @param index The index of the timer to stop
   */
  virtual void stop(int index) = 0;

  /**
   * @brief Reset the timer at specified index
   * @param index The index of the timer to reset
   */
  virtual void reset(int index) = 0;

  /**
   * @brief Print statistics for the timer at specified index
   * @param index The index of the timer to print
   */
  virtual void print_stats(int index) const = 0;

  /**
   * @brief Print statistics for all timers
   */
  virtual void print_all_stats() const = 0;

  /**
   * @brief Get elapsed time in milliseconds for specified timer
   * @param index The index of the timer to query
   * @return Elapsed time in milliseconds
   */
  virtual auto elapsed_time_ms(int index) const -> double = 0;

  /**
   * @brief Get elapsed time in seconds for specified timer
   * @param index The index of the timer to query
   * @return Elapsed time in seconds
   */
  virtual auto elapsed_time(int index) const -> double = 0;

  /**
   * @brief Get the number of timers in the collection
   * @return Number of timers
   */
  virtual auto size() const -> size_t = 0;
};

/**
 * @class simple_timer_array_t
 * @brief Concrete implementation of simple_timer_interface_t using an array of
 * timers
 *
 * This class implements the simple_timer_interface_t using a vector of
 * stop_watch_timer_t objects. It provides thread-safe operations on multiple
 * timers.
 *
 * @code
 * // Basic usage
 * simple_timer_array_t timers(3);
 * timers.start(0);
 * // ... perform operation ...
 * timers.stop(0);
 * double elapsed = timers.elapsed_time_ms(0);
 * timers.print_all_stats();
 * @endcode
 */
class CPP_TOOLBOX_EXPORT simple_timer_array_t : public simple_timer_interface_t
{
public:
  /**
   * @brief Construct a simple_timer_array_t with specified size
   * @param size Number of timers to create
   */
  explicit simple_timer_array_t(int size);

  /**
   * @brief Construct a simple_timer_array_t with specified names
   * @param names Vector of names for each timer
   */
  explicit simple_timer_array_t(const std::vector<std::string>& names);

  // Delete copy constructor and assignment operator
  simple_timer_array_t(const simple_timer_array_t&) = delete;
  auto operator=(const simple_timer_array_t&) -> simple_timer_array_t& = delete;

  // Allow move semantics
  simple_timer_array_t(simple_timer_array_t&&) = default;
  auto operator=(simple_timer_array_t&&) -> simple_timer_array_t& = default;

  ~simple_timer_array_t() override = default;

  // Implement interface methods
  void start(int index) override;
  void stop(int index) override;
  void reset(int index) override;
  auto elapsed_time_ms(int index) const -> double override;
  auto elapsed_time(int index) const -> double override;
  void print_stats(int index) const override;
  void print_all_stats() const override;
  auto size() const -> size_t override;

private:
  /**
   * @brief Check if index is valid
   * @param index Index to check
   * @throws std::out_of_range if index is invalid
   */
  void check_id(int index) const;

  std::vector<stop_watch_timer_t> timers_;  ///< Collection of timers
};

/**
 * @brief Create a timer collection with specified size
 * @param timer_interface Pointer to store the created interface
 * @param size Number of timers to create
 * @return true if creation succeeded, false otherwise
 *
 * @code
 * simple_timer_interface_t* timers;
 * create_timer(&timers, 3);
 * @endcode
 */
inline auto create_timer(simple_timer_interface_t** timer_interface, int size)
    -> bool
{
  *timer_interface =
      static_cast<simple_timer_interface_t*>(new simple_timer_array_t(size));
  return (*timer_interface != nullptr) ? true : false;
}

/**
 * @brief Create a timer collection with specified names
 * @param timer_interface Pointer to store the created interface
 * @param names Vector of names for each timer
 * @return true if creation succeeded, false otherwise
 *
 * @code{.cpp}
 * simple_timer_interface_t* timers;
 * std::vector<std::string> names = {"timer1", "timer2", "timer3"};
 * create_timer_with_names(&timers, names);
 * @endcode
 */
inline auto create_timer_with_names(simple_timer_interface_t** timer_interface,
                                    const std::vector<std::string>& names)
    -> bool
{
  *timer_interface = reinterpret_cast<simple_timer_interface_t*>(
      new simple_timer_array_t(names));
  return (*timer_interface != nullptr) ? true : false;
}

/**
 * @brief Delete a timer collection
 * @param timer_interface Pointer to the interface to delete
 * @return true if deletion succeeded, false otherwise
 *
 * @code{.cpp}
 * simple_timer_interface_t* timers;
 * create_timer(&timers, 3);
 * // ... use timers ...
 * delete_timer(&timers);
 * @endcode
 */
inline auto delete_timer(simple_timer_interface_t** timer_interface) -> bool
{
  if (*timer_interface != nullptr) {
    delete *timer_interface;
    *timer_interface = nullptr;
  }
  return true;
}

/**
 * @brief Start a specific timer
 * @param timer_interface Pointer to the timer interface
 * @param id Index of the timer to start
 * @return true if operation succeeded, false otherwise
 *
 * @code
 * simple_timer_interface_t* timers;
 * create_timer(&timers, 3);
 * start_timer(&timers, 0);
 * @endcode
 */
inline auto start_timer(simple_timer_interface_t** timer_interface, int id)
    -> bool
{
  if (*timer_interface != nullptr) {
    (*timer_interface)->start(id);
  }
  return true;
}

/**
 * @brief Stop a specific timer
 * @param timer_interface Pointer to the timer interface
 * @param id Index of the timer to stop
 * @return true if operation succeeded, false otherwise
 *
 * @code
 * simple_timer_interface_t* timers;
 * create_timer(&timers, 3);
 * start_timer(&timers, 0);
 * // ... perform operation ...
 * stop_timer(&timers, 0);
 * @endcode
 */
inline auto stop_timer(simple_timer_interface_t** timer_interface, int id)
    -> bool
{
  if (*timer_interface != nullptr) {
    (*timer_interface)->stop(id);
  }
  return true;
}

/**
 * @brief Reset a specific timer
 * @param timer_interface Pointer to the timer interface
 * @param id Index of the timer to reset
 * @return true if operation succeeded, false otherwise
 *
 * @code
 * simple_timer_interface_t* timers;
 * create_timer(&timers, 3);
 * start_timer(&timers, 0);
 * // ... perform operation ...
 * reset_timer(&timers, 0);
 * @endcode
 */
inline auto reset_timer(simple_timer_interface_t** timer_interface, int id)
    -> bool
{
  if (*timer_interface != nullptr) {
    (*timer_interface)->reset(id);
  }
  return true;
}

/**
 * @brief Display statistics for a specific timer
 * @param timer_interface Pointer to the timer interface
 * @param id Index of the timer to display
 * @return true if operation succeeded, false otherwise
 *
 * @code
 * simple_timer_interface_t* timers;
 * create_timer(&timers, 3);
 * start_timer(&timers, 0);
 * // ... perform operation ...
 * display_timer(&timers, 0);
 * @endcode
 */
inline auto display_timer(simple_timer_interface_t** timer_interface, int id)
    -> bool
{
  if (*timer_interface != nullptr) {
    (*timer_interface)->print_stats(id);
  }
  return true;
}

/**
 * @brief Display statistics for all timers
 * @param timer_interface Pointer to the timer interface
 * @return true if operation succeeded, false otherwise
 *
 * @code
 * start_timer(&timers, 0);
 * // ... perform operation ...
 * display_all_timer(&timers);
 * @endcode
 */
inline auto display_all_timer(simple_timer_interface_t** timer_interface)
    -> bool
{
  if (*timer_interface != nullptr) {
    (*timer_interface)->print_all_stats();
  }
  return true;
}

/**
 * @brief Get the elapsed time in seconds for a specific timer
 * @param timer_interface Pointer to the timer interface
 * @param id Index of the timer to query
 * @return Elapsed time in seconds. Returns 0.0 if timer_interface is null
 *
 * @code{.cpp}
 * // Basic usage
 * simple_timer_interface_t* timers;
 * create_timer(&timers, 3);
 * start_timer(&timers, 0);
 * // ... perform operation ...
 * double elapsed_seconds = get_timer_elapsed(&timers, 0);
 *
 * // Safe usage with null check
 * simple_timer_interface_t* timers = nullptr;
 * double elapsed = get_timer_elapsed(&timers, 0); // Returns 0.0
 * @endcode
 */
inline auto get_timer_elapsed(simple_timer_interface_t** timer_interface,
                              int id) -> double
{
  if (*timer_interface != nullptr) {
    return (*timer_interface)->elapsed_time(id);
  }
  return 0.0;
}

/**
 * @brief Get the elapsed time in milliseconds for a specific timer
 * @param timer_interface Pointer to the timer interface
 * @param id Index of the timer to query
 * @return Elapsed time in milliseconds. Returns 0.0 if timer_interface is null
 *
 * @code{.cpp}
 * // Basic usage
 * simple_timer_interface_t* timers;
 * create_timer(&timers, 3);
 * start_timer(&timers, 0);
 * // ... perform operation ...
 * double elapsed_ms = get_timer_elapsed_ms(&timers, 0);
 *
 * // Comparing seconds and milliseconds
 * simple_timer_interface_t* timers;
 * create_timer(&timers, 1);
 * start_timer(&timers, 0);
 * std::this_thread::sleep_for(std::chrono::milliseconds(1500));
 * stop_timer(&timers, 0);
 * double seconds = get_timer_elapsed(&timers, 0);    // ~1.5 seconds
 * double milliseconds = get_timer_elapsed_ms(&timers, 0); // ~1500.0 ms
 * @endcode
 */
inline auto get_timer_elapsed_ms(simple_timer_interface_t** timer_interface,
                                 int id) -> double
{
  if (*timer_interface != nullptr) {
    return (*timer_interface)->elapsed_time_ms(id);
  }
  return 0.0;
}

}  // namespace toolbox::utils
