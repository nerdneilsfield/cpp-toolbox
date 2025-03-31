#pragma once

#include <chrono>
#include <string>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::utils
{

/**
 * @class StopWatchTimer
 * @brief A high-resolution stopwatch timer for measuring elapsed time.
 *
 * This class provides a simple interface for measuring elapsed time with
 * nanosecond precision. It supports starting, stopping, and resetting the
 * timer, as well as retrieving elapsed time in milliseconds or seconds.
 *
 * @example
 * // Basic usage
 * StopWatchTimer timer("MyTimer");
 * timer.start();
 * // ... perform some operations ...
 * timer.stop();
 * double elapsed_ms = timer.elapsed_time_ms();
 * timer.print_stats();
 *
 * @example
 * // Multiple start/stop cycles
 * StopWatchTimer timer;
 * timer.start();
 * // ... first operation ...
 * timer.stop();
 * timer.start();
 * // ... second operation ...
 * timer.stop();
 * double total_seconds = timer.elapsed_time();
 */
class CPP_TOOLBOX_EXPORT StopWatchTimer
{
public:
  /**
   * @brief Construct a new StopWatchTimer object
   * @param name The name of the timer (default: "default_timer")
   */
  StopWatchTimer(std::string name = "default_timer");

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
 * @class SimpleTimerInterface
 * @brief Interface for managing a collection of timers
 *
 * This interface provides a unified way to manage multiple timers through
 * a single interface. It supports starting, stopping, resetting, and querying
 * timers by index.
 *
 * @example
 * // Basic usage
 * SimpleTimerInterface* timers = createTimer(3);
 * timers->start(0);
 * // ... perform operation ...
 * timers->stop(0);
 * double elapsed = timers->elapsed_time_ms(0);
 * timers->print_all_stats();
 * deleteTimer(&timers);
 */
class CPP_TOOLBOX_EXPORT SimpleTimerInterface
{
public:
  /**
   * @brief Virtual destructor to ensure proper cleanup
   */
  virtual ~SimpleTimerInterface() = default;

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
 * @class SimpleTimerArray
 * @brief Concrete implementation of SimpleTimerInterface using an array of
 * timers
 *
 * This class implements the SimpleTimerInterface using a vector of
 * StopWatchTimer objects. It provides thread-safe operations on multiple
 * timers.
 *
 * @example
 * // Basic usage
 * SimpleTimerArray timers(3);
 * timers.start(0);
 * // ... perform operation ...
 * timers.stop(0);
 * double elapsed = timers.elapsed_time_ms(0);
 * timers.print_all_stats();
 */
class CPP_TOOLBOX_EXPORT SimpleTimerArray : public SimpleTimerInterface
{
public:
  /**
   * @brief Construct a SimpleTimerArray with specified size
   * @param size Number of timers to create
   */
  explicit SimpleTimerArray(int size);

  /**
   * @brief Construct a SimpleTimerArray with specified names
   * @param names Vector of names for each timer
   */
  SimpleTimerArray(const std::vector<std::string>& names);

  // Delete copy constructor and assignment operator
  SimpleTimerArray(const SimpleTimerArray&) = delete;
  auto operator=(const SimpleTimerArray&) -> SimpleTimerArray& = delete;

  // Allow move semantics
  SimpleTimerArray(SimpleTimerArray&&) = default;
  auto operator=(SimpleTimerArray&&) -> SimpleTimerArray& = default;

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

  std::vector<StopWatchTimer> timers_;  ///< Collection of timers
};

/**
 * @brief Create a timer collection with specified size
 * @param timer_interface Pointer to store the created interface
 * @param size Number of timers to create
 * @return true if creation succeeded, false otherwise
 *
 * @example
 * SimpleTimerInterface* timers;
 * createTimer(&timers, 3);
 */
inline bool createTimer(SimpleTimerInterface** timer_interface, int size)
{
  *timer_interface = reinterpret_cast<SimpleTimerInterface*>(
      new SimpleTimerArrayInterface(size));
  return (*timer_interface != nullptr) ? true : false;
}

/**
 * @brief Create a timer collection with specified names
 * @param timer_interface Pointer to store the created interface
 * @param names Vector of names for each timer
 * @param size Number of timers to create
 * @return true if creation succeeded, false otherwise
 *
 * @example
 * SimpleTimerInterface* timers;
 * std::vector<std::string> names = {"timer1", "timer2", "timer3"};
 * createTimerWithName(&timers, names, 3);
 */
inline bool createTimerWithName(SimpleTimerInterface** timer_interface,
                                const std::vector<std::string>& name,
                                int size)
{
  *timer_interface = reinterpret_cast<SimpleTimerInterface*>(
      new SimpleTimerArrayInterface(name, size));
  return (*timer_interface != nullptr) ? true : false;
}

/**
 * @brief Delete a timer collection
 * @param timer_interface Pointer to the interface to delete
 * @return true if deletion succeeded, false otherwise
 *
 * @example
 * SimpleTimerInterface* timers;
 * createTimer(&timers, 3);
 * // ... use timers ...
 * deleteTimer(&timers);
 */
inline bool deleteTimer(SimpleTimerInterface** timer_interface)
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
 * @example
 * SimpleTimerInterface* timers;
 * createTimer(&timers, 3);
 * startTimer(&timers, 0);
 */
inline bool startTimer(SimpleTimerInterface** timer_interface, int id)
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
 * @example
 * SimpleTimerInterface* timers;
 * createTimer(&timers, 3);
 * startTimer(&timers, 0);
 * // ... perform operation ...
 * stopTimer(&timers, 0);
 */
inline bool stopTimer(SimpleTimerInterface** timer_interface, int id)
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
 * @example
 * SimpleTimerInterface* timers;
 * createTimer(&timers, 3);
 * startTimer(&timers, 0);
 * // ... perform operation ...
 * resetTimer(&timers, 0);
 */
inline bool resetTimer(SimpleTimerInterface** timer_interface, int id)
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
 * @example
 * SimpleTimerInterface* timers;
 * createTimer(&timers, 3);
 * startTimer(&timers, 0);
 * // ... perform operation ...
 * displayTimer(&timers, 0);
 */
inline bool displayTimer(SimpleTimerInterface** timer_interface, int id)
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
 * @example
 * SimpleTimerInterface* timers;
 * createTimer(&timers, 3);
 * startTimer(&timers, 0);
 * // ... perform operation ...
 * displayAllTimer(&timers);
 */
inline bool displayAllTimer(SimpleTimerInterface** timer_interface)
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
 * @example
 * // Basic usage
 * SimpleTimerInterface* timers;
 * createTimer(&timers, 3);
 * startTimer(&timers, 0);
 * // ... perform operation ...
 * double elapsed_seconds = getTimerElapsed(&timers, 0);
 *
 * @example
 * // Safe usage with null check
 * SimpleTimerInterface* timers = nullptr;
 * double elapsed = getTimerElapsed(&timers, 0); // Returns 0.0
 */
inline double getTimerElapsed(SimpleTimerInterface** timer_interface, int id)
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
 * @example
 * // Basic usage
 * SimpleTimerInterface* timers;
 * createTimer(&timers, 3);
 * startTimer(&timers, 0);
 * // ... perform operation ...
 * double elapsed_ms = getTimerElapsedMs(&timers, 0);
 *
 * @example
 * // Comparing seconds and milliseconds
 * SimpleTimerInterface* timers;
 * createTimer(&timers, 1);
 * startTimer(&timers, 0);
 * std::this_thread::sleep_for(std::chrono::milliseconds(1500));
 * stopTimer(&timers, 0);
 * double seconds = getTimerElapsed(&timers, 0);    // ~1.5 seconds
 * double milliseconds = getTimerElapsedMs(&timers, 0); // ~1500.0 ms
 */
inline double getTimerElapsedMs(SimpleTimerInterface** timer_interface, int id)
{
  if (*timer_interface != nullptr) {
    return (*timer_interface)->elapsed_time_ms(id);
  }
  return 0.0;
}

}  // namespace toolbox::utils
