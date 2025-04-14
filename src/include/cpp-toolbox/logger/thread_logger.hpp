#pragma once
#if __cplusplus >= 202002L && defined(__cpp_lib_format)
#  include <format>
#  include <stdexcept>  // For std::format_error
#endif
#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

#include "cpp-toolbox/macro.hpp"

// Include the concurrent queue header
#include "cpp-toolbox/container/concurrent_queue.hpp"

// #define PROJECT_SOURCE_DIR

// !TODO: Why docxgen doesn't work?

namespace toolbox::logger
{

/**
 * @brief Type trait to check if a type is a container.
 *
 * This template checks if a type meets the basic requirements of a container:
 * - Has a value_type typedef
 * - Has an iterator typedef
 * - Has begin() and end() methods
 *
 * @tparam T The type to check for container properties.
 * @tparam _ SFINAE helper parameter (default void).
 *
 * @code{.cpp}
 * // Check if std::vector is a container
 * static_assert(is_container<std::vector<int>>::value, "vector is a
 * container");
 *
 * // Check if int is a container
 * static_assert(!is_container<int>::value, "int is not a container");
 * @endcode
 */
template<typename T, typename = void>
struct CPP_TOOLBOX_EXPORT is_container : std::false_type
{
};

template<typename T>
struct CPP_TOOLBOX_EXPORT
    is_container<T,
                 std::void_t<typename T::value_type,
                             typename T::iterator,
                             decltype(std::declval<T>().begin()),
                             decltype(std::declval<T>().end())>>
    : std::true_type
{
};

/**
 * @brief Helper variable template for is_container trait.
 *
 * This provides a more convenient way to access the is_container trait value.
 *
 * @tparam T The type to check for container properties.
 *
 * @code{.cpp}
 * // Check if std::map is a container
 * constexpr bool is_map_container = is_container_v<std::map<int, int>>;
 * @endcode
 */
template<typename T>
inline constexpr bool is_container_v = is_container<T>::value;

/**
 * @brief Type trait to check if a type has stream insertion operator (<<).
 *
 * This template checks if a type can be inserted into an ostream using the
 * stream insertion operator.
 *
 * @tparam T The type to check for stream insertion capability.
 *
 * @code{.cpp}
 * // Check if int has stream operator
 * static_assert(has_stream_operator<int>::value, "int has stream operator");
 *
 * // Check if custom type has stream operator
 * struct MyType {};
 * static_assert(!has_stream_operator<MyType>::value, "MyType lacks stream
 * operator");
 * @endcode
 */
template<typename T>
struct CPP_TOOLBOX_EXPORT has_stream_operator
{
  template<typename U>
  static auto test(int) -> std::is_same<decltype(std::declval<std::ostream&>()
                                                 << std::declval<const U&>()),
                                        std::ostream&>;

  template<typename>
  static auto test(...) -> std::false_type;

  static constexpr bool value = decltype(test<T>(0))::value;
};

/**
 * @brief Helper variable template for has_stream_operator trait.
 *
 * This provides a more convenient way to access the has_stream_operator trait
 * value.
 *
 * @tparam T The type to check for stream insertion capability.
 *
 * @code{.cpp}
 * // Check if std::string has stream operator
 * constexpr bool can_stream_string = has_stream_operator_v<std::string>;
 * @endcode
 */
template<typename T>
inline constexpr bool has_stream_operator_v = has_stream_operator<T>::value;

/**
 * @brief Type trait to check if a type has an ostream method.
 *
 * This template checks if a type has a member operator<< method that can
 * accept an ostream reference.
 *
 * @tparam T The type to check for ostream method.
 *
 * @code{.cpp}
 * struct MyType {
 *   std::ostream& operator<<(std::ostream& os) const { return os; }
 * };
 * static_assert(has_ostream_method<MyType>::value, "MyType has ostream
 * method");
 * @endcode
 */
template<typename T>
struct CPP_TOOLBOX_EXPORT has_ostream_method
{
  template<typename U>
  static auto test(int) -> std::is_same<
      decltype(std::declval<U>().operator<<(std::declval<std::ostream&>())),
      std::ostream&>;

  template<typename>
  static auto test(...) -> std::false_type;

  static constexpr bool value = decltype(test<T>(0))::value;
};

/**
 * @brief Helper variable template for has_ostream_method trait.
 *
 * This provides a more convenient way to access the has_ostream_method trait
 * value.
 *
 * @tparam T The type to check for ostream method.
 *
 * @code{.cpp}
 * struct MyType {};
 * constexpr bool has_ostream = has_ostream_method_v<MyType>;
 * @endcode
 */
template<typename T>
inline constexpr bool has_ostream_method_v = has_ostream_method<T>::value;

/**
 * @brief Thread-safe logging class with multiple logging levels and formats.
 *
 * @details The `thread_logger_t` class provides a thread-safe logging mechanism
 * with support for different logging levels (TRACE, DEBUG, INFO, WARN, ERROR,
 * CRITICAL). It offers both stream-based and format-based logging capabilities
 * through its nested classes `thread_stream_logger_t` and
 * `thread_format_logger_t`.
 *
 * @note This class implements the singleton pattern and is thread-safe for
 * concurrent logging operations from multiple threads.
 *
 * @see thread_stream_logger_t
 * @see thread_format_logger_t
 *
 * @code{.cpp}
 * // Basic usage with stream logger
 * auto& logger = toolbox::logger::thread_logger_t::instance();
 * logger.set_level(toolbox::logger::thread_logger_t::Level::INFO);
 * LOG_INFO_S << "Application started with version " << version;
 *
 * // Basic usage with format logger
 * auto& logger = toolbox::logger::thread_logger_t::instance();
 * logger.set_level(toolbox::logger::thread_logger_t::Level::DEBUG);
 * LOG_DEBUG_F("Processing {} items in batch {}", item_count, batch_id);
 * @endcode
 */
class CPP_TOOLBOX_EXPORT thread_logger_t
{
public:
  /**
   * @brief Enumeration of logging levels.
   *
   * @details Defines the severity levels for log messages, ordered from least
   * severe (TRACE) to most severe (CRITICAL). Messages with a level below the
   * current logging level will be ignored.
   */
  enum class Level : uint8_t
  {
    TRACE,  ///< Detailed trace information for debugging
    DEBUG,  ///< Debug-level information for development
    INFO,  ///< General operational messages
    WARN,  ///< Warning conditions that might need attention
    ERROR,  ///< Error conditions that need investigation
    CRITICAL  ///< Critical conditions requiring immediate action
  };

  /**
   * @brief Get the singleton instance of the logger.
   *
   * Uses dynamic allocation and ensures the instance persists until program
   * exit to mitigate issues with static destruction order. Thread-safe
   * initialization.
   *
   * @return Reference to the thread-safe logger instance.
   */
  static auto instance() -> thread_logger_t&;

  /**
   * @brief Get the current logging level.
   *
   * @return The current logging level as a Level enum value.
   *
   * @code{.cpp}
   * if (logger.level() <= thread_logger_t::Level::DEBUG) {
   *     // Perform debug-specific operations
   * }
   * @endcode
   */
  auto level() -> Level { return level_; }

  /**
   * @brief Get the current logging level as a string.
   *
   * @return The current logging level as a human-readable string.
   *
   * @code{.cpp}
   * std::cout << "Current log level: " << logger.level_str() << std::endl;
   * @endcode
   */
  auto level_str() -> std::string { return level_to_string(level_); }

  /**
   * @brief Set the logging level.
   *
   * @param level The new logging level to set.
   *
   * @code{.cpp}
   * logger.set_level(thread_logger_t::Level::WARN);
   * @endcode
   */
  auto set_level(Level level) -> void { level_ = level; }

  /**
   * @brief Format-based logger for printf-style message formatting.
   *
   * @details The `thread_format_logger_t` class provides a thread-safe way to
   * log messages using a printf-style format string. It supports format
   * specifiers of the form `{}` which are replaced with the provided arguments.
   *
   * @note Messages are only logged if their level is equal to or higher than
   * the current logging level.
   *
   * @code{.cpp}
   * thread_logger_t::thread_format_logger_t logger(
   *     thread_logger_t::instance(),
   *     thread_logger_t::Level::INFO);
   * logger("Processing {} of {} items", processed_count, total_items);
   * @endcode
   */
  class CPP_TOOLBOX_EXPORT thread_format_logger_t
  {
  public:
    /**
     * @brief Construct a format logger with a specific logging level.
     *
     * @param logger Reference to the parent thread_logger_t instance
     * @param level The logging level for messages from this logger
     */
    thread_format_logger_t(thread_logger_t& logger, Level level);

    /**
     * @brief Log a formatted message.
     *
     * @tparam Args Types of the format arguments
     * @param format The format string containing `{}` placeholders
     * @param args The values to insert into the format string
     *
     * @code{.cpp}
     * thread_format_logger_t logger(thread_logger_t::instance(),
     *     thread_logger_t::Level::DEBUG);
     * logger("Processing time: {} ms", elapsed_time);
     * @endcode
     */
    template<typename... Args>
    void operator()(const char* format, Args&&... args)
    {
      if (level_ < logger_.level()) {
        return;
      }

#if __cplusplus >= 202002L && defined(__cpp_lib_format)
      // C++20: Use std::format for better performance and standard compliance
      try {
        std::string message = std::format(format, std::forward<Args>(args)...);
        logger_.enqueue(level_, std::move(message));
      } catch (const std::format_error& e) {
        // Handle potential format errors gracefully
        std::string error_message = "[Formatting Error] ";
        error_message += e.what();
        error_message += " | Original format: '";
        error_message += format;
        error_message += "'";
        logger_.enqueue(Level::ERROR, std::move(error_message));
      }
#else
      // Pre-C++20: Use the existing custom format_message implementation
      std::string message = format_message(format, std::forward<Args>(args)...);
      logger_.enqueue(level_, std::move(message));
#endif
    }

  private:
    /**
     * @brief Format a message with no arguments.
     *
     * @param format The format string
     * @return The formatted message
     */
    static auto format_message(const char* format) -> std::string
    {
      return {format};  // Simple implementation for no args
    }

    /**
     * @brief Recursively format a message with arguments.
     *
     * @tparam T Type of the current argument
     * @tparam Args Types of remaining arguments
     * @param format The format string
     * @param value The current argument value
     * @param args Remaining arguments
     * @return The formatted message
     *
     * @code{.cpp}
     * std::string msg = format_message("{} + {} = {}", 2, 3, 5);
     * // msg will be "2 + 3 = 5"
     * @endcode
     */
    template<typename T, typename... Args>
    auto format_message(const char* format, T&& value, Args&&... args)
        -> std::string
    {
      std::string result;
      while (*format) {
        if (*format == '{' && *(format + 1) == '}') {
          std::stringstream ss;
          ss << value;
          result += ss.str();
          return result
              + format_message(format + 2, std::forward<Args>(args)...);
        }
        result += *format++;
      }
      return result;
    }

    thread_logger_t& logger_;  ///< Reference to the parent logger
    Level level_;  ///< Logging level for this format logger
  };

  /**
   * @brief Thread-safe stream logger class for formatted output.
   *
   * @details The `thread_stream_logger_t` class provides a thread-safe
   * mechanism for logging messages using stream operators. It supports various
   * data types including containers, tuples, and C-style strings. The logger
   * automatically handles message formatting and ensures thread safety during
   * logging operations.
   *
   * @note This class is typically used through the thread_logger_t interface
   * and its associated macros. Direct instantiation is not recommended.
   *
   * @see thread_logger_t
   *
   * @code{.cpp}
   * // Basic usage with stream logger
   * auto& logger = toolbox::logger::thread_logger_t::instance();
   * LOG_INFO_S << "Processing " << items.size() << " items";
   *
   * // Logging containers
   * std::vector<int> values = {1, 2, 3};
   * LOG_DEBUG_S << "Values: " << values;  // Output: Values: [1, 2, 3]
   * @endcode
   */
  class CPP_TOOLBOX_EXPORT thread_stream_logger_t
  {
  public:
    /**
     * @brief Construct a new thread stream logger object.
     *
     * @param logger Reference to the parent thread logger.
     * @param level Logging level for this stream logger.
     */
    thread_stream_logger_t(thread_logger_t& logger, Level level);

    /**
     * @brief Destroy the thread stream logger object.
     *
     * @details The destructor ensures the final message is flushed to the
     * underlying logger.
     */
    ~thread_stream_logger_t();

    /**
     * @brief Get the logged message as a string.
     *
     * @details This method returns the current content of the log stream as
     * a string. It can be used to inspect or manipulate the message before
     * it is written to the log.
     *
     * @return The logged message as a string.
     *
     * @code{.cpp}
     * auto& logger = thread_logger_t::instance();
     * thread_stream_logger_t stream_logger(logger, Level::INFO);
     * stream_logger << "Test message";
     * std::string msg = stream_logger.str();  // msg = "Test message"
     * @endcode
     */
    auto str() -> std::string { return ss_.str(); }

    /**
     * @brief Overload operator<< for containers.
     *
     * @details This operator allows logging of container types (e.g., vectors,
     * lists) that don't have their own stream operator. The container elements
     * are printed in a comma-separated list enclosed in square brackets.
     *
     * @tparam T Container type (must satisfy is_container_v)
     * @param container The container to log
     * @return Reference to this logger for chaining
     *
     * @code{.cpp}
     * std::vector<int> vec = {1, 2, 3};
     * LOG_INFO_S << "Vector: " << vec;  // Output: Vector: [1, 2, 3]
     * @endcode
     */
    template<typename T>
    auto operator<<(const T& container)
        -> std::enable_if_t<is_container_v<T> && !has_stream_operator_v<T>,
                            thread_stream_logger_t&>
    {
      if (level_ < logger_.level()) {
        return *this;
      }
      ss_ << "[";
      bool first = true;
      for (const auto& item : container) {
        if (!first) {
          ss_ << ", ";
        }
        ss_ << item;
        first = false;
      }
      ss_ << "]";
      return *this;
    }

    /**
     * @brief Overload operator<< for tuples.
     *
     * @details This operator allows logging of std::tuple objects. The tuple
     * elements are printed in a comma-separated list enclosed in parentheses.
     *
     * @tparam Args Types of tuple elements
     * @param t The tuple to log
     * @return Reference to this logger for chaining
     *
     * @code{.cpp}
     * auto t = std::make_tuple(1, "test", 3.14);
     * LOG_DEBUG_S << "Tuple: " << t;  // Output: Tuple: (1, test, 3.14)
     * @endcode
     */
    template<typename... Args>
    auto operator<<(const std::tuple<Args...>& t) -> thread_stream_logger_t&
    {
      if (level_ < logger_.level()) {
        return *this;
      }
      print_tuple(t, std::index_sequence_for<Args...> {});
      return *this;
    }

    /**
     * @brief Format text in red color.
     *
     * @param text The text to format
     * @return Reference to this logger for chaining
     */
    auto red(const std::string& text) -> thread_stream_logger_t&;

    /**
     * @brief Format text in green color.
     *
     * @param text The text to format
     * @return Reference to this logger for chaining
     */
    auto green(const std::string& text) -> thread_stream_logger_t&;

    /**
     * @brief Format text in yellow color.
     *
     * @param text The text to format
     * @return Reference to this logger for chaining
     */
    auto yellow(const std::string& text) -> thread_stream_logger_t&;

    /**
     * @brief Format text in bold style.
     *
     * @param text The text to format
     * @return Reference to this logger for chaining
     */
    auto bold(const std::string& text) -> thread_stream_logger_t&;

    /**
     * @brief Overload operator<< for C-style strings.
     *
     * @details This operator allows logging of C-style strings (const char*).
     * The string is directly written to the log stream.
     *
     * @param value The C-style string to log
     * @return Reference to this logger for chaining
     *
     * @code{.cpp}
     * LOG_INFO_S << "C-style string: " << "Hello World";
     * @endcode
     */
    auto operator<<(const char* value) -> thread_stream_logger_t&
    {
      if (level_ < logger_.level()) {
        return *this;
      }
      ss_ << value;
      return *this;
    }

    // template<typename T,
    //     typename = std::enable_if_t<!std::is_same_v<T, ThreadStreamLogger>>
    // >
    // auto operator()(const T& value) -> ThreadStreamLogger& {
    //     ss_ << value;
    //     return *this;
    // }

    /**
     * @brief Overload operator<< for types that have an operator<< with an
     * ostream.
     *
     * @details This operator allows logging of any type that supports the
     * standard stream insertion operator (operator<<). It checks the logging
     * level before performing the operation and only logs if the current level
     * is sufficient.
     *
     * @tparam T The type of value to log, must have operator<< defined
     * @param value The value to print to the log stream
     * @return Reference to this logger for chaining
     *
     * @code{.cpp}
     * // Logging basic types
     * LOG_INFO_S << 42 << " " << 3.14 << " " << true;
     * // Output: 42 3.14 true
     *
     * // Logging custom types
     * struct Point { int x, y; };
     * std::ostream& operator<<(std::ostream& os, const Point& p) {
     *     return os << "(" << p.x << "," << p.y << ")";
     * }
     * LOG_DEBUG_S << Point{1, 2};
     * // Output: (1,2)
     * @endcode
     */
    template<typename T>
    auto operator<<(const T& value)
        -> std::enable_if_t<has_stream_operator_v<T>, thread_stream_logger_t&>
    {
      if (level_ < logger_.level()) {
        return *this;
      }
      ss_ << value;
      return *this;
    }

    /**
     * @brief Overload operator<< for types with member operator<<.
     *
     * @details This operator handles types that implement operator<< as a
     * member function rather than a free function. It's particularly useful for
     * types that can't modify the global namespace to add stream operators.
     *
     * @tparam T The type of value to log, must have member operator<<
     * @param value The value to print to the log stream
     * @return Reference to this logger for chaining
     *
     * @code{.cpp}
     * struct Logger {
     *     std::ostream& operator<<(std::ostream& os) const {
     *         return os << "Logger instance";
     *     }
     * };
     * LOG_INFO_S << Logger();
     * // Output: Logger instance
     * @endcode
     */
    template<typename T>
    auto operator<<(T&& value)
        -> std::enable_if_t<!has_stream_operator_v<T>
                                && has_ostream_method_v<T>,
                            thread_stream_logger_t&>
    {
      if (level_ < logger_.level()) {
        return *this;
      }
      value.operator<<(ss_);
      return *this;
    }

    /**
     * @brief Overload operator<< for combining loggers.
     *
     * @details This operator allows combining the contents of one logger into
     * another. It's useful for building complex log messages from multiple
     * sources while maintaining the same logging level.
     *
     * @param logger The other logger whose contents to append
     * @return Reference to this logger for chaining
     *
     * @code{.cpp}
     * auto& logger1 = LOG_INFO_S << "Part 1: ";
     * auto& logger2 = LOG_DEBUG_S << "Part 2";
     * logger1 << logger2;  // Only logs if INFO level is enabled
     * // Output: Part 1: Part 2
     * @endcode
     */
    auto operator<<(thread_stream_logger_t& logger) -> thread_stream_logger_t&
    {
      ss_ << logger.str();
      return *this;
    }

    /**
     * @brief Overload operator<< for std::map containers.
     *
     * @details This operator provides formatted output for std::map containers,
     * displaying key-value pairs in a readable format. The output is similar to
     * Python's dictionary representation.
     *
     * @tparam K The key type of the map
     * @tparam V The value type of the map
     * @param map The map to print to the log stream
     * @return Reference to this logger for chaining
     *
     * @code{.cpp}
     * std::map<std::string, int> ages = {{"Alice", 30}, {"Bob", 25}};
     * LOG_DEBUG_S << ages;
     * // Output: {Alice: 30, Bob: 25}
     * @endcode
     */
    template<typename K, typename V>
    auto operator<<(const std::map<K, V>& map) -> thread_stream_logger_t&
    {
      if (level_ < logger_.level()) {
        return *this;
      }
      ss_ << "{";
      bool first = true;
      for (const auto& [key, value] : map) {
        if (!first) {
          ss_ << ", ";
        }
        ss_ << key << ": " << value;
        first = false;
      }
      ss_ << "}";
      return *this;
    }

    /**
     * @brief Overload operator<< for std::unordered_map containers.
     *
     * @details This operator provides formatted output for std::unordered_map
     * containers. The output format is similar to std::map, but the order of
     * elements may vary due to the unordered nature of the container.
     *
     * @tparam K The key type of the map
     * @tparam V The value type of the map
     * @param map The map to print to the log stream
     * @return Reference to this logger for chaining
     *
     * @code{.cpp}
     * std::unordered_map<int, std::string> ids = {{1, "Alice"}, {2, "Bob"}};
     * LOG_INFO_S << ids;
     * // Output: {1: Alice, 2: Bob} (order may vary)
     * @endcode
     */
    template<typename K, typename V>
    auto operator<<(const std::unordered_map<K, V>& map)
        -> thread_stream_logger_t&
    {
      if (level_ < logger_.level())
        return *this;

      ss_ << "{";
      bool first = true;
      for (const auto& [key, value] : map) {
        if (!first)
          ss_ << ", ";
        ss_ << key << ": " << value;
        first = false;
      }
      ss_ << "}";
      return *this;
    }

  private:
    /**
     * @brief Recursively prints the contents of a tuple to the log stream.
     *
     * @details This helper function prints tuples in a format similar to
     * Python's tuple representation, with elements separated by commas and
     * enclosed in parentheses.
     *
     * @tparam Tuple The tuple type to print
     * @tparam Is Parameter pack of indices for tuple elements
     * @param t The tuple to print
     *
     * @code{.cpp}
     * auto t = std::make_tuple(1, "two", 3.0);
     * LOG_DEBUG_S << t;
     * // Output: (1, two, 3.0)
     * @endcode
     */
    template<typename Tuple, size_t... Is>
    auto print_tuple(const Tuple& t, std::index_sequence<Is...>) -> void
    {
      if (level_ < logger_.level()) {
        return;
      }
      ss_ << "(";
      ((ss_ << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);
      ss_ << ")";
    }

    thread_logger_t& logger_;  ///< Reference to the parent logger
    Level level_;  ///< Current logging level
    std::stringstream ss_;  ///< Internal string stream for message building
  };

  /**
   * @brief Get a format logger for TRACE level messages.
   *
   * @return A thread_format_logger_t instance configured for TRACE level
   *
   * @code{.cpp}
   * LOG_TRACE_F("Processing item {}", item_id);
   * @endcode
   */
  auto trace_f() -> thread_format_logger_t { return {*this, Level::TRACE}; }

  /**
   * @brief Get a format logger for DEBUG level messages.
   *
   * @return A thread_format_logger_t instance configured for DEBUG level
   *
   * @code{.cpp}
   * LOG_DEBUG_F("Received {} bytes of data", data_size);
   * @endcode
   */
  auto debug_f() -> thread_format_logger_t { return {*this, Level::DEBUG}; }

  /**
   * @brief Get a format logger for INFO level messages.
   *
   * @return A thread_format_logger_t instance configured for INFO level
   *
   * @code{.cpp}
   * LOG_INFO_F("Application started successfully");
   * @endcode
   */
  auto info_f() -> thread_format_logger_t { return {*this, Level::INFO}; }

  /**
   * @brief Get a format logger for WARN level messages.
   *
   * @return A thread_format_logger_t instance configured for WARN level
   *
   * @code{.cpp}
   * LOG_WARN_F("Disk space is low: {}MB remaining", free_space);
   * @endcode
   */
  auto warn_f() -> thread_format_logger_t { return {*this, Level::WARN}; }

  /**
   * @brief Get a format logger for ERROR level messages.
   *
   * @return A thread_format_logger_t instance configured for ERROR level
   *
   * @code{.cpp}
   * LOG_ERROR_F("Failed to process request: {}", error_message);
   * @endcode
   */
  auto error_f() -> thread_format_logger_t { return {*this, Level::ERROR}; }

  /**
   * @brief Get a format logger for CRITICAL level messages.
   *
   * @return A thread_format_logger_t instance configured for CRITICAL level
   *
   * @code{.cpp}
   * LOG_CRITICAL_F("System shutdown due to critical error: {}", error);
   * @endcode
   */
  auto critical_f() -> thread_format_logger_t
  {
    return {*this, Level::CRITICAL};
  }

  /**
   * @brief Get a stream logger for TRACE level messages.
   *
   * @return A thread_stream_logger_t instance configured for TRACE level
   *
   * @code{.cpp}
   * LOG_TRACE_S << "Entering function " << __func__;
   * @endcode
   */
  auto trace_s() -> thread_stream_logger_t { return {*this, Level::TRACE}; }

  /**
   * @brief Get a stream logger for DEBUG level messages.
   *
   * @return A thread_stream_logger_t instance configured for DEBUG level
   *
   * @code{.cpp}
   * LOG_DEBUG_S << "Processing " << items.size() << " items";
   * @endcode
   */
  auto debug_s() -> thread_stream_logger_t { return {*this, Level::DEBUG}; }

  /**
   * @brief Get a stream logger for INFO level messages.
   *
   * @return A thread_stream_logger_t instance configured for INFO level
   *
   * @example
   * @code
   * LOG_INFO_S << "Application started with version " << version;
   * @endcode
   */
  /**
   * @brief Get a stream logger for INFO level messages.
   *
   * @return A thread_stream_logger_t instance configured for INFO level
   *
   * @code{.cpp}
   * // Log application version info
   * LOG_INFO_S << "Application started with version " << version;
   * @endcode
   */
  auto info_s() -> thread_stream_logger_t { return {*this, Level::INFO}; }

  /**
   * @brief Get a stream logger for WARN level messages.
   *
   * @return A thread_stream_logger_t instance configured for WARN level
   *
   * @code{.cpp}
   * // Log unexpected value warning
   * LOG_WARN_S << "Unexpected value: " << value << " (expected " << expected <<
   * ")";
   * @endcode
   */
  auto warn_s() -> thread_stream_logger_t { return {*this, Level::WARN}; }

  /**
   * @brief Get a stream logger for ERROR level messages.
   *
   * @return A thread_stream_logger_t instance configured for ERROR level
   *
   * @code{.cpp}
   * // Log file open error
   * LOG_ERROR_S << "Failed to open file: " << filename;
   * @endcode
   */
  auto error_s() -> thread_stream_logger_t { return {*this, Level::ERROR}; }

  /**
   * @brief Get a stream logger for CRITICAL level messages.
   *
   * @return A thread_stream_logger_t instance configured for CRITICAL level
   *
   * @code{.cpp}
   * // Log critical system error
   * LOG_CRITICAL_S << "System shutdown due to critical error in " << component;
   * @endcode
   */
  auto critical_s() -> thread_stream_logger_t
  {
    return {*this, Level::CRITICAL};
  }

  /**
   * @brief Explicitly shuts down the logger instance's worker thread.
   *
   * Stops the background worker thread. Should be called once before program
   * termination to ensure clean shutdown, especially in complex scenarios
   * like test frameworks.
   */
  static void shutdown();

private:
  // Constructor remains private
  thread_logger_t();
  // Delete copy/move operations
  thread_logger_t(const thread_logger_t&) = delete;
  auto operator=(const thread_logger_t&) -> thread_logger_t& = delete;
  thread_logger_t(thread_logger_t&&) = delete;
  auto operator=(thread_logger_t&&) -> thread_logger_t& = delete;

  // Make destructor private or protected if we want absolutely no deletion
  // ~thread_logger_t() = default; // Or define it as empty in cpp

  void start();
  void stop();
  void enqueue(Level level, std::string message);
  void processLogs();

  static auto level_to_string(Level level) -> std::string;

  // Queue member remains
  toolbox::container::concurrent_queue_t<std::pair<Level, std::string>> queue_;
  // Worker thread member remains
  std::thread worker_;
  std::atomic<bool> running_ {false};
  Level level_ = Level::INFO;

  // Static members for singleton pattern
  static std::atomic<thread_logger_t*> instance_ptr_;
  static std::mutex instance_mutex_;
  static std::atomic<bool> shutdown_called_;
};

}  // namespace toolbox::logger

/**
 * @defgroup LoggingMacros Logging Macros
 * @brief Macros for convenient logging with different levels and formats
 */

/**
 * @def LOG_TRACE_F
 * @ingroup LoggingMacros
 * @brief Macro for TRACE level format logging
 * @code{.cpp}
 * LOG_TRACE_F("Processing item {}", item_id);
 * @endcode
 */

/**
 * @def LOG_DEBUG_F
 * @ingroup LoggingMacros
 * @brief Macro for DEBUG level format logging
 * @code{.cpp}
 * LOG_DEBUG_F("Received {} bytes of data", data_size);
 * @endcode
 */

/**
 * @def LOG_INFO_F
 * @ingroup LoggingMacros
 * @brief Macro for INFO level format logging
 * @code{.cpp}
 * LOG_INFO_F("Application started successfully");
 * @endcode
 */

/**
 * @def LOG_WARN_F
 * @ingroup LoggingMacros
 * @brief Macro for WARN level format logging
 * @code{.cpp}
 * LOG_WARN_F("Disk space is low: {}MB remaining", free_space);
 * @endcode
 */

/**
 * @def LOG_ERROR_F
 * @ingroup LoggingMacros
 * @brief Macro for ERROR level format logging
 * @code{.cpp}
 * LOG_ERROR_F("Failed to process request: {}", error_message);
 * @endcode
 */

/**
 * @def LOG_CRITICAL_F
 * @ingroup LoggingMacros
 * @brief Macro for CRITICAL level format logging
 * @code{.cpp}
 * LOG_CRITICAL_F("System failure in module {}", module_name);
 * @endcode
 */

/**
 * @def LOG_TRACE_S
 * @ingroup LoggingMacros
 * @brief Macro for TRACE level stream logging
 * @code{.cpp}
 * LOG_TRACE_S << "Entering function " << function_name;
 * @endcode
 */

/**
 * @def LOG_DEBUG_S
 * @ingroup LoggingMacros
 * @brief Macro for DEBUG level stream logging
 * @code{.cpp}
 * LOG_DEBUG_S << "Processing " << items.size() << " items";
 * @endcode
 */

/**
 * @def LOG_INFO_S
 * @ingroup LoggingMacros
 * @brief Macro for INFO level stream logging
 * @code{.cpp}
 * LOG_INFO_S << "Application version: " << version;
 * @endcode
 */

/**
 * @def LOG_WARN_S
 * @ingroup LoggingMacros
 * @brief Macro for WARN level stream logging
 * @code{.cpp}
 * LOG_WARN_S << "Low memory: " << free_memory << "MB available";
 * @endcode
 */

/**
 * @def LOG_ERROR_S
 * @ingroup LoggingMacros
 * @brief Macro for ERROR level stream logging
 * @code{.cpp}
 * LOG_ERROR_S << "Failed to open file: " << filename;
 * @endcode
 */

/**
 * @def LOG_CRITICAL_S
 * @ingroup LoggingMacros
 * @brief Macro for CRITICAL level stream logging
 * @code{.cpp}
 * LOG_CRITICAL_S << "System shutdown due to critical error in " << component;
 * @endcode
 */

/**
 * @def LOG_DEBUG_D
 * @ingroup LoggingMacros
 * @brief Macro for DEBUG level logging with source location
 * @param x The message to log
 * @code{.cpp}
 * LOG_DEBUG_D("Unexpected value: " << value);
 * @endcode
 */

/**
 * @def LOG_INFO_D
 * @ingroup LoggingMacros
 * @brief Macro for INFO level logging with source location
 * @param x The message to log
 * @code{.cpp}
 * LOG_INFO_D("Initialization complete");
 * @endcode
 */

/**
 * @def LOG_WARN_D
 * @ingroup LoggingMacros
 * @brief Macro for WARN level logging with source location
 * @param x The message to log
 * @code{.cpp}
 * LOG_WARN_D("Potential memory leak detected");
 * @endcode
 */

/**
 * @def LOG_ERROR_D
 * @ingroup LoggingMacros
 * @brief Macro for ERROR level logging with source location
 * @param x The message to log
 * @code{.cpp}
 * LOG_ERROR_D("Failed to connect to database");
 * @endcode
 */

/**
 * @def LOG_CRITICAL_D
 * @ingroup LoggingMacros
 * @brief Macro for CRITICAL level logging with source location
 * @param x The message to log
 * @code{.cpp}
 * LOG_CRITICAL_D("System failure detected");
 * @endcode
 */

#define LOG_TRACE_F toolbox::logger::thread_logger_t::instance().trace_f()
#define LOG_DEBUG_F toolbox::logger::thread_logger_t::instance().debug_f()
#define LOG_INFO_F toolbox::logger::thread_logger_t::instance().info_f()
#define LOG_WARN_F toolbox::logger::thread_logger_t::instance().warn_f()
#define LOG_ERROR_F toolbox::logger::thread_logger_t::instance().error_f()
#define LOG_CRITICAL_F toolbox::logger::thread_logger_t::instance().critical_f()

#define LOG_TRACE_S toolbox::logger::thread_logger_t::instance().trace_s()
#define LOG_DEBUG_S toolbox::logger::thread_logger_t::instance().debug_s()
#define LOG_INFO_S toolbox::logger::thread_logger_t::instance().info_s()
#define LOG_WARN_S toolbox::logger::thread_logger_t::instance().warn_s()
#define LOG_ERROR_S toolbox::logger::thread_logger_t::instance().error_s()
#define LOG_CRITICAL_S toolbox::logger::thread_logger_t::instance().critical_s()

#define LOG_DEBUG_D(x) \
  LOG_DEBUG_S << __FILE__ << ":" << __LINE__ << " (" << __CURRENT_FUNCTION__ \
              << ") " << x
#define LOG_INFO_D(x) \
  LOG_INFO_S << __FILE__ << ":" << __LINE__ << " (" << __CURRENT_FUNCTION__ \
             << ") " << x
#define LOG_WARN_D(x) \
  LOG_WARN_S << __FILE__ << ":" << __LINE__ << " (" << __CURRENT_FUNCTION__ \
             << ") " << x
#define LOG_ERROR_D(x) \
  LOG_ERROR_S << __FILE__ << ":" << __LINE__ << " (" << __CURRENT_FUNCTION__ \
              << ") " << x
#define LOG_CRITICAL_D(x) \
  LOG_CRITICAL_S << __FILE__ << ":" << __LINE__ << " (" \
                 << __CURRENT_FUNCTION__ << ") " << x
