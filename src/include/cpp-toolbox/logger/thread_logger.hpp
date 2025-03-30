#pragma once
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

#include "cpp-toolbox/macro.hpp"

// #define PROJECT_SOURCE_DIR

// !TODO: Why docxgen doesn't work?


namespace toolbox::logger
{

/**
 * @brief Check if a type is a container.
 *
 * @tparam T The type to check.
 * @tparam _  A dummy type to enable SFINAE.
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

template<typename T>
inline constexpr bool is_container_v = is_container<T>::value;

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

template<typename T>
inline constexpr bool has_stream_operator_v = has_stream_operator<T>::value;

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

template<typename T>
inline constexpr bool has_ostream_method_v = has_ostream_method<T>::value;


/**
 * @brief ThreadLogger class
 *
 * @details ThreadLogger is a logging class that provides methods to log messages 
 * in a thread-safe manner. It supports different logging levels and can be used 
 * to log messages in different formats.
 *
 * @note This class is thread-safe and can be used across multiple threads.
 *
 * @see thread_stream_logger_t
 * @see thread_format_logger_t
 *
 * Example usage:
 * @code
 * auto& logger = toolbox::logger::thread_logger_t::instance();
 * logger.set_level(toolbox::logger::thread_logger_t::Level::DEBUG);
 * LOG_DEBUG_S << "This is a debug message";
 * @endcode
 */
class CPP_TOOLBOX_EXPORT thread_logger_t
{
public:
  enum class Level : uint8_t
  {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
  };

  static auto instance() -> thread_logger_t&;
  ~thread_logger_t();

  /**
   * @brief Get the current logging level.
   *
   * @return The current logging level.
   */
  auto level() -> Level { return level_; }

  /**
   * @brief Get the current logging level as a string.
   *
   * @return The current logging level as a string.
   */
  auto level_str() -> std::string { return level_to_string(level_); }

  /**
   * @brief Set the logging level.
   *
   * @param level The new logging level.
   */
  auto set_level(Level level) -> void { level_ = level; }

  /**
   * @brief FormatLogger class
   *
   * FormatLogger is a class that provides methods to log formatted messages
   * in a thread-safe manner.
   */
  class CPP_TOOLBOX_EXPORT thread_format_logger_t
  {
  public:
    thread_format_logger_t(thread_logger_t& logger, Level level);

    /**
     * Formats a message using a printf-like format string, and logs
     * the formatted message at the level that this logger was created
     * with.
     *
     * Example usage:
     * @code
     * ThreadLogger::FormatLogger logger{ThreadLogger::instance(),
     * ThreadLogger::Level::INFO}; logger("Logging message with argument: %d",
     * 123);
     * @encode
     *
     * @param format The format string. This string should contain
     * format specifiers of the form `{}`. These will be replaced with
     * the values of the arguments that follow.
     *
     * @param args The values to insert into the format string.
     */
    template<typename... Args>
    void operator()(const char* format, Args&&... args)
    {
      if (level_ < logger_.level()) {
        return;
      }
      std::string message = format_message(format, std::forward<Args>(args)...);
      logger_.enqueue(level_, message);
    }

  private:
    static auto format_message(const char* format) -> std::string;

    /**
     * Format a message using a printf-like format string, and returns
     * the formatted message.
     *
     * Example usage:
     * @code
     * ThreadLogger::FormatLogger logger{ThreadLogger::instance(),
     * ThreadLogger::Level::INFO};
     * std::string message = logger.format_message(
     *     "Logging message with argument: %d", 123);
     * @encode
     *
     * @param format The format string. This string should contain
     * format specifiers of the form `{}`. These will be replaced with
     * the values of the arguments that follow.
     *
     * @param args The values to insert into the format string.
     *
     * @return The formatted message.
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

    thread_logger_t& logger_;
    Level level_;
  };

  /**
   * @brief StreamLogger class
   *
   * StreamLogger is a class that provides methods to log messages in a
   * thread-safe manner.
   */
  class CPP_TOOLBOX_EXPORT thread_stream_logger_t
  {
  public:
    thread_stream_logger_t(thread_logger_t& logger, Level level);
    ~thread_stream_logger_t();

    /**
     * @brief Get the logged message as a string.
     *
     * @return The logged message as a string.
     */
    auto str() -> std::string { return ss_.str(); }

    // add support for basic type
    // template<typename T,
    //     typename = std::enable_if_t<
    //         std::is_arithmetic_v<T> ||
    //         std::is_same_v<T, std::string> ||
    //         std::is_same_v<T, const char*>
    //     >
    // >
    // auto operator<<(const T& value) -> ThreadStreamLogger& {
    //     if (level_ < logger_.level()) return *this;
    //     ss_ << value;
    //     return *this;
    // }

    // add support for container
    /**
     * Overload operator<< for containers. This will print the contents of the
     * container to the log stream.
     *
     * @note This overload will only be selected if the container does not have
     * a `operator<<` overload of its own. This is a fallback for containers
     *       that do not have a custom `operator<<` overload.
     *
     * @param container The container to print.
     *
     * @return A reference to this logger.
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
     * @brief Overload operator<< for tuples. This will print the contents of
     * the tuple to the log stream.
     *
     * @param t The tuple to print.
     *
     * @return A reference to this logger.
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

    auto red(const std::string& text) -> thread_stream_logger_t&;
    auto green(const std::string& text) -> thread_stream_logger_t&;
    auto yellow(const std::string& text) -> thread_stream_logger_t&;
    auto bold(const std::string& text) -> thread_stream_logger_t&;

    /**
     * @brief Overload operator<< for const char*. This will print the const
     * char* to the log stream.
     *
     * @param value The const char* to print.
     *
     * @return A reference to this logger.
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
     * @param value The value to print to the log stream.
     *
     * @return A reference to this logger.
     */
    template<typename T>
    auto operator<<(const T& value)
        -> std::enable_if_t<has_stream_operator_v<T>, thread_stream_logger_t&>
    {
      // If the log level is less than the logger's level, then do nothing.
      if (level_ < logger_.level()) {
        return *this;
      }

      // Print the value to the log stream.
      ss_ << value;

      // Return a reference to this logger so that the user can chain calls to
      // operator<<.
      return *this;
    }

    /**
     * @brief Overload operator<< for types that do not have an operator<< with
     * an ostream, but do have a member function operator<< that takes an
     * ostream.
     *
     * @param value The value to print to the log stream.
     *
     * @return A reference to this logger.
     */
    template<typename T>
    auto operator<<(T&& value)
        -> std::enable_if_t<!has_stream_operator_v<T>
                                && has_ostream_method_v<T>,
                            thread_stream_logger_t&>
    {
      // If the log level is less than the logger's level, then do nothing.
      if (level_ < logger_.level()) {
        return *this;
      }

      // Call the value's operator<< member function with the log stream.
      value.operator<<(ss_);

      // Return a reference to this logger so that the user can chain calls to
      // operator<<.
      return *this;
    }

    /**
     * @brief Overload operator<< for ThreadStreamLogger. This will print the
     * contents of the other logger to this logger.
     *
     * @param logger The other logger to print to this logger.
     *
     * @return A reference to this logger.
     */
    auto operator<<(thread_stream_logger_t& logger) -> thread_stream_logger_t&
    {
      // Print the contents of the other logger to this logger.
      ss_ << logger.str();

      // Return a reference to this logger so that the user can chain calls to
      // operator<<.
      return *this;
    }

    /**
     * @brief Overload operator<< for std::map. This will print the contents of
     * the map to the log stream.
     *
     * @param map The map to print to the log stream.
     *
     * @return A reference to this logger.
     */
    template<typename K, typename V>
    auto operator<<(const std::map<K, V>& map) -> thread_stream_logger_t&
    {
      // If the log level is less than the logger's level, then do nothing.
      if (level_ < logger_.level()) {
        return *this;
      }

      // Print the contents of the map to the log stream.
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

      // Return a reference to this logger so that the user can chain calls to
      // operator<<.
      return *this;
    }

    /**
     * @brief Overload operator<< for std::unordered_map. This will print the
     *        contents of the map to the log stream.
     *
     * @param map The map to print to the log stream.
     *
     * @return A reference to this logger.
     */
    template<typename K, typename V>
    auto operator<<(const std::unordered_map<K, V>& map)
        -> thread_stream_logger_t&
    {
      // If the log level is less than the logger's level, then do nothing.
      if (level_ < logger_.level())
        return *this;

      // Print the contents of the map to the log stream.
      ss_ << "{";
      bool first = true;
      for (const auto& [key, value] : map) {
        if (!first)
          ss_ << ", ";
        ss_ << key << ": " << value;
        first = false;
      }
      ss_ << "}";

      // Return a reference to this logger so that the user can chain calls to
      // operator<<.
      return *this;
    }

  private:
    /**
     * @brief Recursively prints the contents of a tuple to the log stream.
     *
     * @param t The tuple to print.
     * @param Is A parameter pack of indices into the tuple.
     */
    template<typename Tuple, size_t... Is>
    auto print_tuple(const Tuple& t, std::index_sequence<Is...>) -> void
    {
      if (level_ < logger_.level()) {
        return;
      }

      // Print an opening parenthesis.
      ss_ << "(";

      // Recursively print each element of the tuple, separated by commas.
      ((ss_ << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);

      // Print a closing parenthesis.
      ss_ << ")";
    }

    thread_logger_t& logger_;
    Level level_;
    std::stringstream ss_;
  };

  auto trace_f() -> thread_format_logger_t { return {*this, Level::TRACE}; }
  auto debug_f() -> thread_format_logger_t { return {*this, Level::DEBUG}; }
  auto info_f() -> thread_format_logger_t { return {*this, Level::INFO}; }
  auto warn_f() -> thread_format_logger_t { return {*this, Level::WARN}; }
  auto error_f() -> thread_format_logger_t { return {*this, Level::ERROR}; }
  auto critical_f() -> thread_format_logger_t
  {
    return {*this, Level::CRITICAL};
  }

  auto trace_s() -> thread_stream_logger_t { return {*this, Level::TRACE}; }
  auto debug_s() -> thread_stream_logger_t { return {*this, Level::DEBUG}; }
  auto info_s() -> thread_stream_logger_t { return {*this, Level::INFO}; }
  auto warn_s() -> thread_stream_logger_t { return {*this, Level::WARN}; }
  auto error_s() -> thread_stream_logger_t { return {*this, Level::ERROR}; }
  auto critical_s() -> thread_stream_logger_t
  {
    return {*this, Level::CRITICAL};
  }

private:
  thread_logger_t();
  thread_logger_t(const thread_logger_t&) = delete;
  auto operator=(const thread_logger_t&) -> thread_logger_t& = delete;

  void start();
  void stop();
  void enqueue(Level level, const std::string& message);
  void processLogs();

  static auto level_to_string(Level level) -> std::string;

  template<typename T, typename = void>
  struct is_container : std::false_type
  {
  };

  template<typename T>
  struct is_container<T,
                      std::void_t<typename T::value_type,
                                  typename T::iterator,
                                  decltype(std::declval<T>().begin()),
                                  decltype(std::declval<T>().end())>>
      : std::true_type
  {
  };

  template<typename T>
  static constexpr bool is_container_v = is_container<T>::value;

  std::queue<std::pair<Level, std::string>> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread worker_;
  std::atomic<bool> running_ {false};

  Level level_ = Level::INFO;
};

} // namespace toolbox::logger



// 格式化日志宏
#define LOG_TRACE_F toolbox::logger::thread_logger_t::instance().trace_f()
#define LOG_DEBUG_F toolbox::logger::thread_logger_t::instance().debug_f()
#define LOG_INFO_F toolbox::logger::thread_logger_t::instance().info_f()
#define LOG_WARN_F toolbox::logger::thread_logger_t::instance().warn_f()
#define LOG_ERROR_F toolbox::logger::thread_logger_t::instance().error_f()
#define LOG_CRITICAL_F toolbox::logger::thread_logger_t::instance().critical_f()

// 流式日志宏
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
