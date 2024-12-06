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

// #define PROJECT_SOURCE_DIR

namespace toolbox::logger
{

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
inline constexpr bool is_container_v = is_container<T>::value;

template<typename T>
struct has_stream_operator
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
class has_ostream_method
{
  template<typename U>
  static auto test(int) -> std::is_same<
      decltype(std::declval<U>().operator<<(std::declval<std::ostream&>())),
      std::ostream&>;

  template<typename>
  static auto test(...) -> std::false_type;

public:
  static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
inline constexpr bool has_ostream_method_v = has_ostream_method<T>::value;

// inline auto get_file_relative_path(const std::string& project_source_dir,
//                                    const std::string& file_path_str)
//     -> std::string
// {
//   static const auto base_path = std::filesystem::path(project_source_dir);
//   static const auto file_path = std::filesystem::path(file_path_str);
//   static const auto relative_path =
//       std::filesystem::relative(file_path, base_path);
//   std::cout << "project_source_dir: " << project_source_dir << "\n";
//   std::cout << "file_path: " << file_path_str << "\n";
//   std::cout << "relative_path: " << relative_path.string() << "\n";
//   return relative_path.string();
// }

class ThreadLogger
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

  static auto instance() -> ThreadLogger&;
  ~ThreadLogger();

  auto level() -> Level { return level_; }
  auto level_str() -> std::string { return level_to_string(level_); }
  auto set_level(Level level) -> void { level_ = level; }

  // 格式化日志类
  class ThreadFormatLogger
  {
  public:
    ThreadFormatLogger(ThreadLogger& logger, Level level);

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
      if (level_ < logger_.level())
        return;
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

    ThreadLogger& logger_;
    Level level_;
  };

  // 流式日志类
  class ThreadStreamLogger
  {
  public:
    ThreadStreamLogger(ThreadLogger& logger, Level level);
    ~ThreadStreamLogger();

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
                            ThreadStreamLogger&>
    {
      if (level_ < logger_.level())
        return *this;
      ss_ << "[";
      bool first = true;
      for (const auto& item : container) {
        if (!first)
          ss_ << ", ";
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
    auto operator<<(const std::tuple<Args...>& t) -> ThreadStreamLogger&
    {
      if (level_ < logger_.level())
        return *this;
      print_tuple(t, std::index_sequence_for<Args...> {});
      return *this;
    }

    auto red(const std::string& text) -> ThreadStreamLogger&;
    auto green(const std::string& text) -> ThreadStreamLogger&;
    auto yellow(const std::string& text) -> ThreadStreamLogger&;
    auto bold(const std::string& text) -> ThreadStreamLogger&;

    /**
     * @brief Overload operator<< for const char*. This will print the const
     * char* to the log stream.
     *
     * @param value The const char* to print.
     *
     * @return A reference to this logger.
     */
    auto operator<<(const char* value) -> ThreadStreamLogger&
    {
      if (level_ < logger_.level())
        return *this;
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
        -> std::enable_if_t<has_stream_operator_v<T>, ThreadStreamLogger&>
    {
      // If the log level is less than the logger's level, then do nothing.
      if (level_ < logger_.level())
        return *this;

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
                            ThreadStreamLogger&>
    {
      // If the log level is less than the logger's level, then do nothing.
      if (level_ < logger_.level())
        return *this;

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
    auto operator<<(ThreadStreamLogger& logger) -> ThreadStreamLogger&
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
    auto operator<<(const std::map<K, V>& map) -> ThreadStreamLogger&
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

    /**
     * @brief Overload operator<< for std::unordered_map. This will print the
     *        contents of the map to the log stream.
     *
     * @param map The map to print to the log stream.
     *
     * @return A reference to this logger.
     */
    template<typename K, typename V>
    auto operator<<(const std::unordered_map<K, V>& map) -> ThreadStreamLogger&
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
      if (level_ < logger_.level())
        return;

      // Print an opening parenthesis.
      ss_ << "(";

      // Recursively print each element of the tuple, separated by commas.
      ((ss_ << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);

      // Print a closing parenthesis.
      ss_ << ")";
    }

    ThreadLogger& logger_;
    Level level_;
    std::stringstream ss_;
  };

  auto trace_f() -> ThreadFormatLogger { return {*this, Level::TRACE}; }
  auto debug_f() -> ThreadFormatLogger { return {*this, Level::DEBUG}; }
  auto info_f() -> ThreadFormatLogger { return {*this, Level::INFO}; }
  auto warn_f() -> ThreadFormatLogger { return {*this, Level::WARN}; }
  auto error_f() -> ThreadFormatLogger { return {*this, Level::ERROR}; }
  auto critical_f() -> ThreadFormatLogger { return {*this, Level::CRITICAL}; }

  auto trace_s() -> ThreadStreamLogger { return {*this, Level::TRACE}; }
  auto debug_s() -> ThreadStreamLogger { return {*this, Level::DEBUG}; }
  auto info_s() -> ThreadStreamLogger { return {*this, Level::INFO}; }
  auto warn_s() -> ThreadStreamLogger { return {*this, Level::WARN}; }
  auto error_s() -> ThreadStreamLogger { return {*this, Level::ERROR}; }
  auto critical_s() -> ThreadStreamLogger { return {*this, Level::CRITICAL}; }

private:
  ThreadLogger();
  ThreadLogger(const ThreadLogger&) = delete;
  auto operator=(const ThreadLogger&) -> ThreadLogger& = delete;

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

}  // namespace toolbox::logger

// 格式化日志宏
#define LOG_TRACE_F toolbox::logger::ThreadLogger::instance().trace_f()
#define LOG_DEBUG_F toolbox::logger::ThreadLogger::instance().debug_f()
#define LOG_INFO_F toolbox::logger::ThreadLogger::instance().info_f()
#define LOG_WARN_F toolbox::logger::ThreadLogger::instance().warn_f()
#define LOG_ERROR_F toolbox::logger::ThreadLogger::instance().error_f()
#define LOG_CRITICAL_F toolbox::logger::ThreadLogger::instance().critical_f()

// 流式日志宏
#define LOG_TRACE_S toolbox::logger::ThreadLogger::instance().trace_s()
#define LOG_DEBUG_S toolbox::logger::ThreadLogger::instance().debug_s()
#define LOG_INFO_S toolbox::logger::ThreadLogger::instance().info_s()
#define LOG_WARN_S toolbox::logger::ThreadLogger::instance().warn_s()
#define LOG_ERROR_S toolbox::logger::ThreadLogger::instance().error_s()
#define LOG_CRITICAL_S toolbox::logger::ThreadLogger::instance().critical_s()

#define LOG_DEBUG_D(x) LOG_DEBUG_S << __FILE__ << ":" << __LINE__ << " " << x
#define LOG_INFO_D(x) LOG_INFO_S << __FILE__ << ":" << __LINE__ << " " << x
#define LOG_WARN_D(x) LOG_WARN_S << __FILE__ << ":" << __LINE__ << " " << x
#define LOG_ERROR_D(x) LOG_ERROR_S << __FILE__ << ":" << __LINE__ << " " << x
#define LOG_CRITICAL_D(x) LOG_CRITICAL_S << __FILE__ << ":" << __LINE__ << " " << x

