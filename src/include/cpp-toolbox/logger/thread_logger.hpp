#pragma once
#if __cplusplus >= 202002L && defined(__cpp_lib_format)
#  include <format>
#  include <stdexcept>  // 用于std::format_error / For std::format_error
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

// 包含并发队列头文件 / Include the concurrent queue header
#include "cpp-toolbox/container/concurrent_queue.hpp"

// 包含对象池 / Include object pool
#include "cpp-toolbox/base/object_pool.hpp"

// #define PROJECT_SOURCE_DIR

// !TODO: Why docxgen doesn't work?

namespace toolbox::logger
{

/**
 * @brief 检查类型是否为容器的类型特征 / Type trait to check if a type is a
 * container
 *
 * @details 此模板检查类型是否满足容器的基本要求 / This template checks if a
 * type meets the basic requirements of a container:
 * - 具有value_type类型定义 / Has a value_type typedef
 * - 具有iterator类型定义 / Has an iterator typedef
 * - 具有begin()和end()方法 / Has begin() and end() methods
 *
 * @tparam T 要检查容器属性的类型 / The type to check for container properties
 * @tparam _ SFINAE辅助参数(默认void) / SFINAE helper parameter (default void)
 *
 * @code{.cpp}
 * // 检查std::vector是否为容器 / Check if std::vector is a container
 * static_assert(is_container<std::vector<int>>::value, "vector is a
 * container");
 *
 * // 检查int是否为容器 / Check if int is a container
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
 * @brief is_container特征的辅助变量模板 / Helper variable template for
 * is_container trait
 *
 * @details 提供了一种更方便的方式来访问is_container特征值 / This provides a
 * more convenient way to access the is_container trait value
 *
 * @tparam T 要检查容器属性的类型 / The type to check for container properties
 *
 * @code{.cpp}
 * // 检查std::map是否为容器 / Check if std::map is a container
 * constexpr bool is_map_container = is_container_v<std::map<int, int>>;
 * @endcode
 */
template<typename T>
inline constexpr bool is_container_v = is_container<T>::value;

/**
 * @brief 检查类型是否具有流插入运算符(<<)的类型特征 / Type trait to check if a
 * type has stream insertion operator (<<)
 *
 * @details 此模板检查类型是否可以使用流插入运算符插入到ostream中 / This
 * template checks if a type can be inserted into an ostream using the stream
 * insertion operator
 *
 * @tparam T 要检查流插入能力的类型 / The type to check for stream insertion
 * capability
 *
 * @code{.cpp}
 * // 检查int是否具有流运算符 / Check if int has stream operator
 * static_assert(has_stream_operator<int>::value, "int has stream operator");
 *
 * // 检查自定义类型是否具有流运算符 / Check if custom type has stream operator
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
 * @brief has_stream_operator特征的辅助变量模板 / Helper variable template for
 * has_stream_operator trait
 *
 * @details 提供了一种更方便的方式来访问has_stream_operator特征值 / This
 * provides a more convenient way to access the has_stream_operator trait value
 *
 * @tparam T 要检查流插入能力的类型 / The type to check for stream insertion
 * capability
 *
 * @code{.cpp}
 * // 检查std::string是否具有流运算符 / Check if std::string has stream operator
 * constexpr bool can_stream_string = has_stream_operator_v<std::string>;
 * @endcode
 */
template<typename T>
inline constexpr bool has_stream_operator_v = has_stream_operator<T>::value;

/**
 * @brief 检查类型是否具有ostream方法的类型特征 / Type trait to check if a type
 * has an ostream method
 *
 * @details 此模板检查类型是否具有可以接受ostream引用的成员operator<<方法 / This
 * template checks if a type has a member operator<< method that can accept an
 * ostream reference
 *
 * @tparam T 要检查ostream方法的类型 / The type to check for ostream method
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
 * @brief has_ostream_method特征的辅助变量模板 / Helper variable template for
 * has_ostream_method trait
 *
 * @details 提供了一种更方便的方式来访问has_ostream_method特征值 / This provides
 * a more convenient way to access the has_ostream_method trait value
 *
 * @tparam T 要检查ostream方法的类型 / The type to check for ostream method
 *
 * @code{.cpp}
 * struct MyType {};
 * constexpr bool has_ostream = has_ostream_method_v<MyType>;
 * @endcode
 */
template<typename T>
inline constexpr bool has_ostream_method_v = has_ostream_method<T>::value;

/**
 * @brief 具有多个日志级别和格式的线程安全日志类 / Thread-safe logging class
 * with multiple logging levels and formats
 *
 * @details
 * thread_logger_t类提供了一个线程安全的日志机制,支持不同的日志级别(TRACE,
 * DEBUG, INFO, WARN, ERROR,
 * CRITICAL)。它通过其嵌套类thread_stream_logger_t和thread_format_logger_t提供基于流和格式的日志功能。/
 * The thread_logger_t class provides a thread-safe logging mechanism with
 * support for different logging levels (TRACE, DEBUG, INFO, WARN, ERROR,
 * CRITICAL). It offers both stream-based and format-based logging capabilities
 * through its nested classes thread_stream_logger_t and thread_format_logger_t.
 *
 * @note 此类实现了单例模式,并且对于来自多个线程的并发日志操作是线程安全的 /
 * This class implements the singleton pattern and is thread-safe for concurrent
 * logging operations from multiple threads
 *
 * @see thread_stream_logger_t
 * @see thread_format_logger_t
 *
 * @code{.cpp}
 * // 使用流式日志记录器的基本用法 / Basic usage with stream logger
 * auto& logger = toolbox::logger::thread_logger_t::instance();
 * logger.set_level(toolbox::logger::thread_logger_t::Level::INFO);
 * LOG_INFO_S << "Application started with version " << version;
 *
 * // 使用格式化日志记录器的基本用法 / Basic usage with format logger
 * auto& logger = toolbox::logger::thread_logger_t::instance();
 * logger.set_level(toolbox::logger::thread_logger_t::Level::DEBUG);
 * LOG_DEBUG_F("Processing {} items in batch {}", item_count, batch_id);
 * @endcode
 */
class CPP_TOOLBOX_EXPORT thread_logger_t
{
public:
  /**
   * @brief 日志级别枚举 / Enumeration of logging levels
   *
   * @details
   * 定义了日志消息的严重程度级别,从最不严重(TRACE)到最严重(CRITICAL)。低于当前日志级别的消息将被忽略。/
   * Defines the severity levels for log messages, ordered from least severe
   * (TRACE) to most severe (CRITICAL). Messages with a level below the current
   * logging level will be ignored.
   */
  enum class Level : uint8_t
  {
    TRACE,  ///< 用于调试的详细跟踪信息 / Detailed trace information for
            ///< debugging
    DEBUG,  ///< 用于开发的调试级别信息 / Debug-level information for
            ///< development
    INFO,  ///< 一般操作消息 / General operational messages
    WARN,  ///< 需要注意的警告条件 / Warning conditions that might need
           ///< attention
    ERROR,  ///< 需要调查的错误条件 / Error conditions that need investigation
    CRITICAL  ///< 需要立即采取行动的关键条件 / Critical conditions requiring
              ///< immediate action
  };

  /**
   * @brief 获取日志记录器的单例实例 / Get the singleton instance of the logger
   *
   * @details
   * 使用动态分配并确保实例在程序退出前一直存在,以减轻静态析构顺序问题。初始化是线程安全的。/
   * Uses dynamic allocation and ensures the instance persists until program
   * exit to mitigate issues with static destruction order. Thread-safe
   * initialization.
   *
   * @return 对线程安全日志记录器实例的引用 / Reference to the thread-safe
   * logger instance
   */
  static auto instance() -> thread_logger_t&;

  /**
   * @brief 获取当前日志级别 / Get the current logging level
   *
   * @return 当前日志级别作为Level枚举值 / The current logging level as a Level
   * enum value
   *
   * @code{.cpp}
   * if (logger.level() <= thread_logger_t::Level::DEBUG) {
   *     // 执行调试特定操作 / Perform debug-specific operations
   * }
   * @endcode
   */
  auto level() -> Level
  {
    return level_;
  }

  /**
   * @brief 获取当前日志级别的字符串表示 / Get the current logging level as a
   * string
   *
   * @return 当前日志级别的人类可读字符串 / The current logging level as a
   * human-readable string
   *
   * @code{.cpp}
   * std::cout << "Current log level: " << logger.level_str() << std::endl;
   * @endcode
   */
  auto level_str() -> std::string
  {
    return level_to_string(level_);
  }

  /**
   * @brief 设置日志级别 / Set the logging level
   *
   * @param level 要设置的新日志级别 / The new logging level to set
   *
   * @code{.cpp}
   * logger.set_level(thread_logger_t::Level::WARN);
   * @endcode
   */
  auto set_level(Level level) -> void
  {
    level_ = level;
  }

  /**
   * @brief 基于格式的日志记录器,用于printf风格的消息格式化 / Format-based
   * logger for printf-style message formatting
   *
   * @details
   * thread_format_logger_t类提供了一种使用printf风格格式字符串记录消息的线程安全方式。它支持形式为{}的格式说明符,这些说明符将被提供的参数替换。/
   * The thread_format_logger_t class provides a thread-safe way to log messages
   * using a printf-style format string. It supports format specifiers of the
   * form {} which are replaced with the provided arguments.
   *
   * @note 只有当消息的级别等于或高于当前日志级别时才会记录消息 / Messages are
   * only logged if their level is equal to or higher than the current logging
   * level
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
     * @brief 使用特定日志级别构造格式日志记录器 / Construct a format logger
     * with a specific logging level
     *
     * @param logger 对父thread_logger_t实例的引用 / Reference to the parent
     * thread_logger_t instance
     * @param level 此日志记录器的日志级别 / The logging level for messages from
     * this logger
     */
    thread_format_logger_t(thread_logger_t& logger, Level level);

    /**
     * @brief 记录格式化消息 / Log a formatted message
     *
     * @tparam Args 格式参数的类型 / Types of the format arguments
     * @param format 包含{}占位符的格式字符串 / The format string containing {}
     * placeholders
     * @param args 要插入格式字符串的值 / The values to insert into the format
     * string
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
      // C++20: 使用std::format以获得更好的性能和标准合规性 / Use std::format
      // for better performance and standard compliance
      try {
        std::string message = std::format(format, std::forward<Args>(args)...);
        logger_.enqueue(level_, std::move(message));
      } catch (const std::format_error& e) {
        // 优雅地处理潜在的格式错误 / Handle potential format errors gracefully
        std::string error_message = "[Formatting Error] ";
        error_message += e.what();
        error_message += " | Original format: '";
        error_message += format;
        error_message += "'";
        logger_.enqueue(Level::ERROR, std::move(error_message));
      }
#else
      // Pre-C++20: 使用现有的自定义format_message实现 / Use the existing custom
      // format_message implementation
      std::string message = format_message(format, std::forward<Args>(args)...);
      logger_.enqueue(level_, std::move(message));
#endif
    }

  private:
    /**
     * @brief 格式化没有参数的消息 / Format a message with no arguments
     *
     * @param format 格式字符串 / The format string
     * @return 格式化的消息 / The formatted message
     */
    static auto format_message(const char* format) -> std::string
    {
      return {format};  // 无参数的简单实现 / Simple implementation for no args
    }

    /**
     * @brief 递归格式化带参数的消息 / Recursively format a message with
     * arguments
     *
     * @tparam T 当前参数的类型 / Type of the current argument
     * @tparam Args 剩余参数的类型 / Types of remaining arguments
     * @param format 格式字符串 / The format string
     * @param value 当前参数值 / The current argument value
     * @param args 剩余参数 / Remaining arguments
     * @return 格式化的消息 / The formatted message
     *
     * @code{.cpp}
     * std::string msg = format_message("{} + {} = {}", 2, 3, 5);
     * // msg将是"2 + 3 = 5" / msg will be "2 + 3 = 5"
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

    thread_logger_t&
        logger_;  ///< 对父日志记录器的引用 / Reference to the parent logger
    Level level_;  ///< 此格式日志记录器的日志级别 / Logging level for this
                   ///< format logger
  };

  /**
   * @brief 用于格式化输出的线程安全流式日志记录器类 / Thread-safe stream logger
   * class for formatted output
   *
   * @details
   * thread_stream_logger_t类提供了一个使用流运算符记录消息的线程安全机制。它支持各种数据类型,包括容器、元组和C风格字符串。日志记录器自动处理消息格式化并确保日志操作期间的线程安全。/
   * The thread_stream_logger_t class provides a thread-safe mechanism for
   * logging messages using stream operators. It supports various data types
   * including containers, tuples, and C-style strings. The logger automatically
   * handles message formatting and ensures thread safety during logging
   * operations.
   *
   * @note 此类通常通过thread_logger_t接口及其相关宏使用。不建议直接实例化。/
   * This class is typically used through the thread_logger_t interface and its
   * associated macros. Direct instantiation is not recommended.
   *
   * @see thread_logger_t
   *
   * @code{.cpp}
   * // 使用流式日志记录器的基本用法 / Basic usage with stream logger
   * auto& logger = toolbox::logger::thread_logger_t::instance();
   * LOG_INFO_S << "Processing " << items.size() << " items";
   *
   * // 记录容器 / Logging containers
   * std::vector<int> values = {1, 2, 3};
   * LOG_DEBUG_S << "Values: " << values;  // 输出: Values: [1, 2, 3] / Output:
   * Values: [1, 2, 3]
   * @endcode
   */
  class CPP_TOOLBOX_EXPORT thread_stream_logger_t
  {
  public:
    /**
     * @brief 使用池构造新的线程流日志记录器对象 / Construct a new thread stream
     * logger object using the pool
     *
     * @param logger 对父线程日志记录器的引用 / Reference to the parent thread
     * logger
     * @param level 此流日志记录器的日志级别 / Logging level for this stream
     * logger
     * @param pool 对stringstream对象池的引用 / Reference to the stringstream
     * object pool
     */
    thread_stream_logger_t(
        thread_logger_t& logger,
        Level level,
        toolbox::base::object_pool_t<std::stringstream>& pool);

    /**
     * @brief 销毁线程流日志记录器对象 / Destroy the thread stream logger object
     *
     * @details 将最终消息入队。ss_ptr_析构函数自动将stringstream返回到池中。/
     * Enqueues the final message. The stringstream is automatically returned to
     * the pool by the ss_ptr_ destructor.
     */
    ~thread_stream_logger_t();

    // 删除复制/移动构造函数/赋值运算符 / Deleted copy/move
    // constructors/assignments
    thread_stream_logger_t(const thread_stream_logger_t&) = delete;
    thread_stream_logger_t& operator=(const thread_stream_logger_t&) = delete;
    thread_stream_logger_t(thread_stream_logger_t&&) = delete;
    thread_stream_logger_t& operator=(thread_stream_logger_t&&) = delete;

    /**
     * @brief 将记录的消息作为字符串获取 / Get the logged message as a string
     * @return 记录的消息作为字符串 / The logged message as a string
     */
    auto str() -> std::string
    {
      return ss_ptr_->str();
    }  // 使用指针 / Use pointer

    /**
     * @brief 重载容器的operator<< / Overload operator<< for containers
     *
     * @details
     * 此运算符允许记录没有自己的流运算符的容器类型(如向量、列表)。容器元素以逗号分隔的列表形式打印,用方括号括起来。/
     * This operator allows logging of container types (e.g., vectors, lists)
     * that don't have their own stream operator. The container elements are
     * printed in a comma-separated list enclosed in square brackets.
     *
     * @tparam T 容器类型(必须满足is_container_v) / Container type (must satisfy
     * is_container_v)
     * @param container 要记录的容器 / The container to log
     * @return 对此日志记录器的引用,用于链式调用 / Reference to this logger for
     * chaining
     *
     * @code{.cpp}
     * std::vector<int> vec = {1, 2, 3};
     * LOG_INFO_S << "Vector: " << vec;  // 输出: Vector: [1, 2, 3] / Output:
     * Vector: [1, 2, 3]
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
      *ss_ptr_ << "[";  // 使用指针 / Use pointer
      bool first = true;
      for (const auto& item : container) {
        if (!first) {
          *ss_ptr_ << ", ";
        }
        *ss_ptr_ << item;  // 使用指针 / Use pointer
        first = false;
      }
      *ss_ptr_ << "]";  // 使用指针 / Use pointer
      return *this;
    }

    /**
     * @brief 重载元组的operator<< / Overload operator<< for tuples
     *
     * @details
     * 此运算符允许记录std::tuple对象。元组元素以逗号分隔的列表形式打印,用圆括号括起来。/
     * This operator allows logging of std::tuple objects. The tuple elements
     * are printed in a comma-separated list enclosed in parentheses.
     *
     * @tparam Args 元组元素的类型 / Types of tuple elements
     * @param t 要记录的元组 / The tuple to log
     * @return 对此日志记录器的引用,用于链式调用 / Reference to this logger for
     * chaining
     *
     * @code{.cpp}
     * auto t = std::make_tuple(1, "test", 3.14);
     * LOG_DEBUG_S << "Tuple: " << t;  // 输出: Tuple: (1, test, 3.14) / Output:
     * Tuple: (1, test, 3.14)
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
     * @brief 以红色格式化文本 / Format text in red color
     *
     * @param text 要格式化的文本 / The text to format
     * @return 对此日志记录器的引用,用于链式调用 / Reference to this logger for
     * chaining
     */
    auto red(const std::string& text) -> thread_stream_logger_t&;

    /**
     * @brief 以绿色格式化文本 / Format text in green color
     *
     * @param text 要格式化的文本 / The text to format
     * @return 对此日志记录器的引用,用于链式调用 / Reference to this logger for
     * chaining
     */
    auto green(const std::string& text) -> thread_stream_logger_t&;

    /**
     * @brief 以黄色格式化文本 / Format text in yellow color
     *
     * @param text 要格式化的文本 / The text to format
     * @return 对此日志记录器的引用,用于链式调用 / Reference to this logger for
     * chaining
     */
    auto yellow(const std::string& text) -> thread_stream_logger_t&;

    /**
     * @brief 以粗体格式化文本 / Format text in bold style
     *
     * @param text 要格式化的文本 / The text to format
     * @return 对此日志记录器的引用,用于链式调用 / Reference to this logger for
     * chaining
     */
    auto bold(const std::string& text) -> thread_stream_logger_t&;

    /**
     * @brief 重载C风格字符串的operator<< / Overload operator<< for C-style
     * strings
     *
     * @details 此运算符允许记录C风格字符串(const
     * char*)。字符串直接写入日志流。/ This operator allows logging of C-style
     * strings (const char*). The string is directly written to the log stream.
     *
     * @param value 要记录的C风格字符串 / The C-style string to log
     * @return 对此日志记录器的引用,用于链式调用 / Reference to this logger for
     * chaining
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
      *ss_ptr_ << value;  // 使用指针 / Use pointer
      return *this;
    }

    /**
     * @brief 重载具有ostream operator<<的类型的operator<< / Overload operator<<
     * for types that have an operator<< with an ostream
     *
     * @details
     * 此运算符允许记录任何支持标准流插入运算符(operator<<)的类型。它在执行操作前检查日志级别,只有当当前级别足够时才记录。/
     * This operator allows logging of any type that supports the standard
     * stream insertion operator (operator<<). It checks the logging level
     * before performing the operation and only logs if the current level is
     * sufficient.
     *
     * @tparam T 要记录的值的类型,必须定义了operator<< / The type of value to
     * log, must have operator<< defined
     * @param value 要打印到日志流的值 / The value to print to the log stream
     * @return 对此日志记录器的引用,用于链式调用 / Reference to this logger for
     * chaining
     *
     * @code{.cpp}
     * // 记录基本类型 / Logging basic types
     * LOG_INFO_S << 42 << " " << 3.14 << " " << true;
     * // 输出: 42 3.14 true / Output: 42 3.14 true
     *
     * // 记录自定义类型 / Logging custom types
     * struct Point { int x, y; };
     * std::ostream& operator<<(std::ostream& os, const Point& p) {
     *     return os << "(" << p.x << "," << p.y << ")";
     * }
     * LOG_DEBUG_S << Point{1, 2};
     * // 输出: (1,2) / Output: (1,2)
     * @endcode
     */
    template<typename T>
    auto operator<<(const T& value)
        -> std::enable_if_t<has_stream_operator_v<T>, thread_stream_logger_t&>
    {
      if (level_ < logger_.level()) {
        return *this;
      }
      *ss_ptr_ << value;  // Use pointer
      return *this;
    }

    /**
     * @brief 重载具有成员operator<<的类型的operator<< / Overload operator<< for
     * types with member operator<<
     *
     * @details
     * 此运算符处理将operator<<实现为成员函数而不是自由函数的类型。对于不能修改全局命名空间来添加流运算符的类型特别有用。/
     * This operator handles types that implement operator<< as a member
     * function rather than a free function. It's particularly useful for types
     * that can't modify the global namespace to add stream operators.
     *
     * @tparam T 要记录的值的类型,必须有成员operator<< / The type of value to
     * log, must have member operator<<
     * @param value 要打印到日志流的值 / The value to print to the log stream
     * @return 对此日志记录器的引用,用于链式调用 / Reference to this logger for
     * chaining
     *
     * @code{.cpp}
     * struct CustomLogger {
     *     std::ostream& operator<<(std::ostream& os) const {
     *         return os << "Custom logger output";
     *     }
     * };
     * LOG_INFO_S << CustomLogger();
     * // 输出: Custom logger output / Output: Custom logger output
     * @endcode
     */
    template<typename T>
    auto operator<<(T&& value) -> std::enable_if_t<
        !has_stream_operator_v<T> && has_ostream_method_v<T>,
        thread_stream_logger_t&>
    {
      if (level_ < logger_.level()) {
        return *this;
      }
      value.operator<<(*ss_ptr_);  // 使用指针 / Use pointer
      return *this;
    }

    /**
     * @brief 重载用于组合日志记录器的operator<< / Overload operator<< for
     * combining loggers
     *
     * @details
     * 此运算符允许将一个日志记录器的内容组合到另一个中。在保持相同日志级别的同时从多个来源构建复杂日志消息时很有用。/
     * This operator allows combining the contents of one logger into another.
     * It's useful for building complex log messages from multiple sources while
     * maintaining the same logging level.
     *
     * @param logger 要追加其内容的其他日志记录器 / The other logger whose
     * contents to append
     * @return 对此日志记录器的引用,用于链式调用 / Reference to this logger for
     * chaining
     *
     * @code{.cpp}
     * auto& logger1 = LOG_INFO_S << "状态: " << "正常";
     * auto& logger2 = LOG_DEBUG_S << " 详细信息: " << "所有系统运行中";
     * logger1 << logger2;  // 仅在INFO级别启用时记录 / Only logs if INFO level
     * is enabled
     * // 输出: 状态: 正常 详细信息: 所有系统运行中 / Output: Status: Normal
     * Details: All systems running
     * @endcode
     */
    auto operator<<(thread_stream_logger_t& logger) -> thread_stream_logger_t&
    {
      *ss_ptr_ << logger.str();  // 使用指针 / Use pointer
      return *this;
    }

    /**
     * @brief 重载std::map容器的operator<< / Overload operator<< for std::map
     * containers
     *
     * @details
     * 此运算符为std::map容器提供格式化输出,以可读格式显示键值对。输出类似于Python的字典表示。/
     * This operator provides formatted output for std::map containers,
     * displaying key-value pairs in a readable format. The output is similar to
     * Python's dictionary representation.
     *
     * @tparam K map的键类型 / The key type of the map
     * @tparam V map的值类型 / The value type of the map
     * @param map 要打印到日志流的map / The map to print to the log stream
     * @return 对此日志记录器的引用,用于链式调用 / Reference to this logger for
     * chaining
     *
     * @code{.cpp}
     * std::map<std::string, int> scores = {{"张三", 95}, {"李四", 88}};
     * LOG_DEBUG_S << "考试成绩: " << scores;
     * // 输出: 考试成绩: {张三: 95, 李四: 88} / Output: Exam scores: {张三: 95,
     * 李四: 88}
     *
     * std::map<int, std::string> ids = {{1001, "项目A"}, {1002, "项目B"}};
     * LOG_INFO_S << "项目列表: " << ids;
     * // 输出: 项目列表: {1001: 项目A, 1002: 项目B} / Output: Project list:
     * {1001: Project A, 1002: Project B}
     * @endcode
     */
    template<typename K, typename V>
    auto operator<<(const std::map<K, V>& map) -> thread_stream_logger_t&
    {
      if (level_ < logger_.level()) {
        return *this;
      }
      *ss_ptr_ << "{";  // 使用指针 / Use pointer
      bool first = true;
      for (const auto& [key, value] : map) {
        if (!first) {
          *ss_ptr_ << ", ";
        }
        *ss_ptr_ << key << ": " << value;  // 使用指针 / Use pointer
        first = false;
      }
      *ss_ptr_ << "}";  // 使用指针 / Use pointer
      return *this;
    }

    /**
     * @brief 重载std::unordered_map容器的operator<< / Overload operator<< for
     * std::unordered_map containers
     *
     * @details
     * 此运算符为std::unordered_map容器提供格式化输出。输出格式类似于std::map,但由于容器的无序性质,元素顺序可能会变化。/
     * This operator provides formatted output for std::unordered_map
     * containers. The output format is similar to std::map, but the order of
     * elements may vary due to the unordered nature of the container.
     *
     * @tparam K map的键类型 / The key type of the map
     * @tparam V map的值类型 / The value type of the map
     * @param map 要打印到日志流的map / The map to print to the log stream
     * @return 对此日志记录器的引用,用于链式调用 / Reference to this logger for
     * chaining
     *
     * @code{.cpp}
     * std::unordered_map<std::string, double> prices = {{"苹果", 5.5},
     * {"香蕉", 3.8}}; LOG_INFO_S << "水果价格: " << prices;
     * // 输出: 水果价格: {苹果: 5.5, 香蕉: 3.8} (顺序可能变化) / Output: Fruit
     * prices: {苹果: 5.5, 香蕉: 3.8} (order may vary)
     *
     * std::unordered_map<int, std::string> status = {{200, "成功"}, {404,
     * "未找到"}}; LOG_DEBUG_S << "状态码: " << status;
     * // 输出: 状态码: {200: 成功, 404: 未找到} (顺序可能变化) / Output: Status
     * codes: {200: Success, 404: Not Found} (order may vary)
     * @endcode
     */
    template<typename K, typename V>
    auto operator<<(const std::unordered_map<K, V>& map)
        -> thread_stream_logger_t&
    {
      if (level_ < logger_.level())
        return *this;
      *ss_ptr_ << "{";  // 使用指针 / Use pointer
      bool first = true;
      for (const auto& [key, value] : map) {
        if (!first)
          *ss_ptr_ << ", ";
        *ss_ptr_ << key << ": " << value;  // 使用指针 / Use pointer
        first = false;
      }
      *ss_ptr_ << "}";  // 使用指针 / Use pointer
      return *this;
    }

  private:
    /**
     * @brief 递归打印元组内容到日志流 / Recursively prints the contents of a
     * tuple to the log stream
     *
     * @details
     * 此辅助函数以类似Python元组表示的格式打印元组,元素用逗号分隔并用括号括起。/
     * This helper function prints tuples in a format similar to Python's tuple
     * representation, with elements separated by commas and enclosed in
     * parentheses.
     *
     * @tparam Tuple 要打印的元组类型 / The tuple type to print
     * @tparam Is 元组元素的索引参数包 / Parameter pack of indices for tuple
     * elements
     * @param t 要打印的元组 / The tuple to print
     *
     * @code{.cpp}
     * auto student = std::make_tuple("张三", 20, 85.5);
     * LOG_DEBUG_S << "学生信息: " << student;
     * // 输出: 学生信息: (张三, 20, 85.5) / Output: Student info: (张三,
     * 20, 85.5)
     * @endcode
     */
    template<typename Tuple, size_t... Is>
    auto print_tuple(const Tuple& t, std::index_sequence<Is...>) -> void
    {
      if (level_ < logger_.level()) {
        return;
      }
      *ss_ptr_ << "(";  // 使用指针 / Use pointer
      ((*ss_ptr_ << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);
      *ss_ptr_ << ")";  // 使用指针 / Use pointer
    }

    thread_logger_t&
        logger_;  ///< 对父日志记录器的引用 / Reference to the parent logger
    Level level_;  ///< 当前日志级别 / Current logging level
    // 将stringstream存储在从池中获取的unique_ptr中 / Store the stringstream in
    // a unique_ptr obtained from the pool
    toolbox::base::object_pool_t<std::stringstream>::PooledObjectPtr ss_ptr_;
  };

  /**
   * @brief 获取用于TRACE级别消息的格式化日志记录器 / Get a format logger for
   * TRACE level messages
   *
   * @return 配置为TRACE级别的thread_format_logger_t实例 / A
   * thread_format_logger_t instance configured for TRACE level
   *
   * @code{.cpp}
   * LOG_TRACE_F("正在处理项目 {}", item_id);
   * // 输出: 正在处理项目 123 / Output: Processing item 123
   * @endcode
   */
  auto trace_f() -> thread_format_logger_t
  {
    return {*this, Level::TRACE};
  }

  /**
   * @brief 获取用于DEBUG级别消息的格式化日志记录器 / Get a format logger for
   * DEBUG level messages
   *
   * @return 配置为DEBUG级别的thread_format_logger_t实例 / A
   * thread_format_logger_t instance configured for DEBUG level
   *
   * @code{.cpp}
   * LOG_DEBUG_F("已接收 {} 字节数据", data_size);
   * // 输出: 已接收 1024 字节数据 / Output: Received 1024 bytes of data
   * @endcode
   */
  auto debug_f() -> thread_format_logger_t
  {
    return {*this, Level::DEBUG};
  }

  /**
   * @brief 获取用于INFO级别消息的格式化日志记录器 / Get a format logger for
   * INFO level messages
   *
   * @return 配置为INFO级别的thread_format_logger_t实例 / A
   * thread_format_logger_t instance configured for INFO level
   *
   * @code{.cpp}
   * LOG_INFO_F("应用程序启动成功,版本: {}", version);
   * // 输出: 应用程序启动成功,版本: 1.0.0 / Output: Application started
   * successfully, version: 1.0.0
   * @endcode
   */
  auto info_f() -> thread_format_logger_t
  {
    return {*this, Level::INFO};
  }

  /**
   * @brief 获取用于WARN级别消息的格式化日志记录器 / Get a format logger for
   * WARN level messages
   *
   * @return 配置为WARN级别的thread_format_logger_t实例 / A
   * thread_format_logger_t instance configured for WARN level
   *
   * @code{.cpp}
   * LOG_WARN_F("磁盘空间不足: 剩余{}MB", free_space);
   * // 输出: 磁盘空间不足: 剩余100MB / Output: Disk space is low: 100MB
   * remaining
   * @endcode
   */
  auto warn_f() -> thread_format_logger_t
  {
    return {*this, Level::WARN};
  }

  /**
   * @brief 获取用于ERROR级别消息的格式化日志记录器 / Get a format logger for
   * ERROR level messages
   *
   * @return 配置为ERROR级别的thread_format_logger_t实例 / A
   * thread_format_logger_t instance configured for ERROR level
   *
   * @code{.cpp}
   * LOG_ERROR_F("请求处理失败: {}", error_message);
   * // 输出: 请求处理失败: 连接超时 / Output: Failed to process request:
   * Connection timeout
   * @endcode
   */
  auto error_f() -> thread_format_logger_t
  {
    return {*this, Level::ERROR};
  }

  /**
   * @brief 获取用于CRITICAL级别消息的格式化日志记录器 / Get a format logger for
   * CRITICAL level messages
   *
   * @return 配置为CRITICAL级别的thread_format_logger_t实例 / A
   * thread_format_logger_t instance configured for CRITICAL level
   *
   * @code{.cpp}
   * LOG_CRITICAL_F("系统因严重错误关闭: {}", error);
   * // 输出: 系统因严重错误关闭: 内存耗尽 / Output: System shutdown due to
   * critical error: Out of memory
   * @endcode
   */
  auto critical_f() -> thread_format_logger_t
  {
    return {*this, Level::CRITICAL};
  }

  /**
   * @brief 获取用于TRACE级别消息的流式日志记录器 / Get a stream logger for
   * TRACE level messages
   *
   * @return 配置为TRACE级别的thread_stream_logger_t实例 / A
   * thread_stream_logger_t instance configured for TRACE level
   *
   * @code{.cpp}
   * LOG_TRACE_S << "进入函数 " << __func__ << " 参数: " << param;
   * // 输出: 进入函数 process_data 参数: 42 / Output: Entering function
   * process_data param: 42
   * @endcode
   */
  auto trace_s() -> thread_stream_logger_t
  {
    return {*this, Level::TRACE, stringstream_pool_};
  }

  /**
   * @brief 获取用于DEBUG级别消息的流式日志记录器 / Get a stream logger for
   * DEBUG level messages
   *
   * @return 配置为DEBUG级别的thread_stream_logger_t实例 / A
   * thread_stream_logger_t instance configured for DEBUG level
   *
   * @code{.cpp}
   * LOG_DEBUG_S << "正在处理 " << items.size() << " 个项目";
   * // 输出: 正在处理 5 个项目 / Output: Processing 5 items
   * @endcode
   */
  auto debug_s() -> thread_stream_logger_t
  {
    return {*this, Level::DEBUG, stringstream_pool_};
  }

  /**
   * @brief 获取用于INFO级别消息的流式日志记录器 / Get a stream logger for INFO
   * level messages
   *
   * @return 配置为INFO级别的thread_stream_logger_t实例 / A
   * thread_stream_logger_t instance configured for INFO level
   *
   * @code{.cpp}
   * LOG_INFO_S << "应用程序启动,版本: " << version << " 配置: " << config_file;
   * // 输出: 应用程序启动,版本: 2.0.0 配置: config.json / Output: Application
   * started with version 2.0.0 config: config.json
   * @endcode
   */
  auto info_s() -> thread_stream_logger_t
  {
    return {*this, Level::INFO, stringstream_pool_};
  }

  /**
   * @brief 获取用于WARN级别消息的流式日志记录器 / Get a stream logger for WARN
   * level messages
   *
   * @return 配置为WARN级别的thread_stream_logger_t实例 / A
   * thread_stream_logger_t instance configured for WARN level
   *
   * @code{.cpp}
   * LOG_WARN_S << "意外的值: " << value << " (预期: " << expected << ")";
   * // 输出: 意外的值: 404 (预期: 200) / Output: Unexpected value: 404
   * (expected: 200)
   * @endcode
   */
  auto warn_s() -> thread_stream_logger_t
  {
    return {*this, Level::WARN, stringstream_pool_};
  }

  /**
   * @brief 获取用于ERROR级别消息的流式日志记录器 / Get a stream logger for
   * ERROR level messages
   *
   * @return 配置为ERROR级别的thread_stream_logger_t实例 / A
   * thread_stream_logger_t instance configured for ERROR level
   *
   * @code{.cpp}
   * // 记录文件打开错误 / Log file open error
   * LOG_ERROR_S << "无法打开文件: " << filename << ", 错误码: " << errno;
   *
   * // 记录数据库错误 / Log database error
   * LOG_ERROR_S << "数据库查询失败: " << sql << ", 错误信息: " <<
   * db.lastError();
   * @endcode
   */
  auto error_s() -> thread_stream_logger_t
  {
    return {*this, Level::ERROR, stringstream_pool_};
  }

  /**
   * @brief 获取用于CRITICAL级别消息的流式日志记录器 / Get a stream logger for
   * CRITICAL level messages
   *
   * @return 配置为CRITICAL级别的thread_stream_logger_t实例 / A
   * thread_stream_logger_t instance configured for CRITICAL level
   *
   * @code{.cpp}
   * // 记录严重系统错误 / Log critical system error
   * LOG_CRITICAL_S << "系统组件" << component << "发生严重错误,正在关闭系统";
   *
   * // 记录资源耗尽 / Log resource exhaustion
   * LOG_CRITICAL_S << "内存耗尽: 已用" << used_mem << "MB, 总计" << total_mem
   * << "MB";
   * @endcode
   */
  auto critical_s() -> thread_stream_logger_t
  {
    return {*this, Level::CRITICAL, stringstream_pool_};
  }

  /**
   * @brief 显式关闭日志记录器实例的工作线程 / Explicitly shuts down the logger
   * instance's worker thread
   *
   * @details
   * 停止后台工作线程。在程序终止前应调用一次以确保干净关闭,特别是在测试框架等复杂场景中。/
   * Stops the background worker thread. Should be called once before program
   * termination to ensure clean shutdown, especially in complex scenarios like
   * test frameworks.
   *
   * @code{.cpp}
   * int main() {
   *   // 使用日志记录器 / Use logger
   *   LOG_INFO_S << "程序启动";
   *
   *   // 程序结束前关闭日志 / Shutdown logger before program ends
   *   thread_logger_t::shutdown();
   *   return 0;
   * }
   * @endcode
   */
  static void shutdown();

private:
  // 构造函数保持私有 / Constructor remains private
  thread_logger_t();
  // 删除复制/移动操作 / Delete copy/move operations
  thread_logger_t(const thread_logger_t&) = delete;
  auto operator=(const thread_logger_t&) -> thread_logger_t& = delete;
  thread_logger_t(thread_logger_t&&) = delete;
  auto operator=(thread_logger_t&&) -> thread_logger_t& = delete;

  void start();
  void stop();
  void enqueue(Level level, std::string message);
  void processLogs();

  static auto level_to_string(Level level) -> std::string;

  // 队列成员保持不变 / Queue member remains
  toolbox::container::concurrent_queue_t<std::pair<Level, std::string>> queue_;
  // 工作线程成员保持不变 / Worker thread member remains
  std::thread worker_;
  std::atomic<bool> running_ {false};
  Level level_ = Level::INFO;

  // 单例模式的静态成员 / Static members for singleton pattern
  static std::atomic<thread_logger_t*> instance_ptr_;
  static std::mutex instance_mutex_;
  static std::atomic<bool> shutdown_called_;

  // 添加字符串流池 / Add the stringstream pool
  toolbox::base::object_pool_t<std::stringstream> stringstream_pool_;
};

}  // namespace toolbox::logger

/**
 * @defgroup LoggingMacros 日志宏 / Logging Macros
 * @brief 用于不同级别和格式的便捷日志记录宏 / Macros for convenient logging
 * with different levels and formats
 */

/**
 * @def LOG_TRACE_F
 * @ingroup LoggingMacros
 * @brief TRACE级别格式化日志的宏 / Macro for TRACE level format logging
 * @code{.cpp}
 * LOG_TRACE_F("正在处理项目 {} / Processing item {}", item_id);
 * @endcode
 */

/**
 * @def LOG_DEBUG_F
 * @ingroup LoggingMacros
 * @brief DEBUG级别格式化日志的宏 / Macro for DEBUG level format logging
 * @code{.cpp}
 * LOG_DEBUG_F("收到 {} 字节数据 / Received {} bytes of data", data_size);
 * @endcode
 */

/**
 * @def LOG_INFO_F
 * @ingroup LoggingMacros
 * @brief INFO级别格式化日志的宏 / Macro for INFO level format logging
 * @code{.cpp}
 * LOG_INFO_F("应用程序启动成功 / Application started successfully");
 * @endcode
 */

/**
 * @def LOG_WARN_F
 * @ingroup LoggingMacros
 * @brief WARN级别格式化日志的宏 / Macro for WARN level format logging
 * @code{.cpp}
 * LOG_WARN_F("磁盘空间不足: 剩余{}MB / Disk space is low: {}MB remaining",
 * free_space);
 * @endcode
 */

/**
 * @def LOG_ERROR_F
 * @ingroup LoggingMacros
 * @brief ERROR级别格式化日志的宏 / Macro for ERROR level format logging
 * @code{.cpp}
 * LOG_ERROR_F("请求处理失败: {} / Failed to process request: {}",
 * error_message);
 * @endcode
 */

/**
 * @def LOG_CRITICAL_F
 * @ingroup LoggingMacros
 * @brief CRITICAL级别格式化日志的宏 / Macro for CRITICAL level format logging
 * @code{.cpp}
 * LOG_CRITICAL_F("模块 {} 发生系统故障 / System failure in module {}",
 * module_name);
 * @endcode
 */

/**
 * @def LOG_TRACE_S
 * @ingroup LoggingMacros
 * @brief TRACE级别流式日志的宏 / Macro for TRACE level stream logging
 * @code{.cpp}
 * LOG_TRACE_S << "进入函数 / Entering function " << function_name;
 * @endcode
 */

/**
 * @def LOG_DEBUG_S
 * @ingroup LoggingMacros
 * @brief DEBUG级别流式日志的宏 / Macro for DEBUG level stream logging
 * @code{.cpp}
 * LOG_DEBUG_S << "正在处理 / Processing " << items.size() << " 个项目 / items";
 * @endcode
 */

/**
 * @def LOG_INFO_S
 * @ingroup LoggingMacros
 * @brief INFO级别流式日志的宏 / Macro for INFO level stream logging
 * @code{.cpp}
 * LOG_INFO_S << "应用程序版本 / Application version: " << version;
 * @endcode
 */

/**
 * @def LOG_WARN_S
 * @ingroup LoggingMacros
 * @brief WARN级别流式日志的宏 / Macro for WARN level stream logging
 * @code{.cpp}
 * LOG_WARN_S << "内存不足 / Low memory: " << free_memory << "MB 可用 /
 * available";
 * @endcode
 */

/**
 * @def LOG_ERROR_S
 * @ingroup LoggingMacros
 * @brief ERROR级别流式日志的宏 / Macro for ERROR level stream logging
 * @code{.cpp}
 * LOG_ERROR_S << "无法打开文件 / Failed to open file: " << filename;
 * @endcode
 */

/**
 * @def LOG_CRITICAL_S
 * @ingroup LoggingMacros
 * @brief CRITICAL级别流式日志的宏 / Macro for CRITICAL level stream logging
 * @code{.cpp}
 * LOG_CRITICAL_S << "组件 / System shutdown due to critical error in " <<
 * component;
 * @endcode
 */

/**
 * @def LOG_DEBUG_D
 * @ingroup LoggingMacros
 * @brief 带源代码位置的DEBUG级别日志的宏 / Macro for DEBUG level logging with
 * source location
 * @param x 要记录的消息 / The message to log
 * @code{.cpp}
 * LOG_DEBUG_D("意外的值 / Unexpected value: " << value);
 * @endcode
 */

/**
 * @def LOG_INFO_D
 * @ingroup LoggingMacros
 * @brief 带源代码位置的INFO级别日志的宏 / Macro for INFO level logging with
 * source location
 * @param x 要记录的消息 / The message to log
 * @code{.cpp}
 * LOG_INFO_D("初始化完成 / Initialization complete");
 * @endcode
 */

/**
 * @def LOG_WARN_D
 * @ingroup LoggingMacros
 * @brief 带源代码位置的WARN级别日志的宏 / Macro for WARN level logging with
 * source location
 * @param x 要记录的消息 / The message to log
 * @code{.cpp}
 * LOG_WARN_D("检测到潜在的内存泄漏 / Potential memory leak detected");
 * @endcode
 */

/**
 * @def LOG_ERROR_D
 * @ingroup LoggingMacros
 * @brief 带源代码位置的ERROR级别日志的宏 / Macro for ERROR level logging with
 * source location
 * @param x 要记录的消息 / The message to log
 * @code{.cpp}
 * LOG_ERROR_D("无法连接到数据库 / Failed to connect to database");
 * @endcode
 */

/**
 * @def LOG_CRITICAL_D
 * @ingroup LoggingMacros
 * @brief 带源代码位置的CRITICAL级别日志的宏 / Macro for CRITICAL level logging
 * with source location
 * @param x 要记录的消息 / The message to log
 * @code{.cpp}
 * LOG_CRITICAL_D("检测到系统故障 / System failure detected");
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
