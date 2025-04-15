.. index:: pair: example; /home/dengqi/Source/langs/cpp/cpp-toolbox/src/include/cpp-toolbox/logger/thread_logger.hpp
.. _doxid-_2home_2dengqi_2Source_2langs_2cpp_2cpp-toolbox_2src_2include_2cpp-toolbox_2logger_2thread_logger_8hpp-example:

/home/dengqi/Source/langs/cpp/cpp-toolbox/src/include/cpp-toolbox/logger/thread_logger.hpp
==========================================================================================

Get a stream logger for INFO level messages.

Get a stream logger for INFO level messages.

.. ref-code-block:: cpp

	LOG_INFO_S << "Application started with version " << version;



.. ref-code-block:: cpp

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
	
	// Include object pool
	#include "cpp-toolbox/base/object_pool.hpp"
	
	// #define PROJECT_SOURCE_DIR
	
	// !TODO: Why docxgen doesn't work?
	
	namespace :ref:`toolbox::logger <doxid-namespacetoolbox_1_1logger>`
	{
	
	template<typename T, typename = void>
	struct CPP_TOOLBOX_EXPORT is_container : std::false_type
	{
	};
	
	template<typename T>
	struct CPP_TOOLBOX_EXPORT
	    is_container<T,
	                 std:::ref:`void_t <doxid-namespacetoolbox_1_1traits_1_1detail_1af947ecd84c1315e86403e59d3508e4f3>`<typename T::value_type,
	                             typename T::iterator,
	                             decltype(std::declval<T>().begin()),
	                             decltype(std::declval<T>().end())>>
	    : std::true_type
	{
	};
	
	template<typename T>
	inline constexpr bool :ref:`is_container_v <doxid-namespacetoolbox_1_1logger_1ae92163dce5c70e6094943043c3c76b49>` = is_container<T>::value;
	
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
	inline constexpr bool :ref:`has_stream_operator_v <doxid-namespacetoolbox_1_1logger_1aaf82a52d321593496c80328501f31622>` = :ref:`has_stream_operator\<T>::value <doxid-structtoolbox_1_1logger_1_1has__stream__operator_1a7fd2e9e81dc24f6c22f29b2d55cf5746>`;
	
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
	inline constexpr bool :ref:`has_ostream_method_v <doxid-namespacetoolbox_1_1logger_1a69ac1b9f2684929bd3b9f229e15fb322>` = :ref:`has_ostream_method\<T>::value <doxid-structtoolbox_1_1logger_1_1has__ostream__method_1ac4b2319fca9f3e19506d9202c9f7ea69>`;
	
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
	
	  auto level() -> Level
	  {
	    return level_;
	  }
	
	  auto level_str() -> std::string
	  {
	    return level_to_string(level_);
	  }
	
	  auto set_level(Level level) -> void
	  {
	    level_ = level;
	  }
	
	  class CPP_TOOLBOX_EXPORT thread_format_logger_t
	  {
	  public:
	    thread_format_logger_t(thread_logger_t& logger, Level level);
	
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
	    static auto format_message(const char* format) -> std::string
	    {
	      return {format};  // Simple implementation for no args
	    }
	
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
	
	  class CPP_TOOLBOX_EXPORT thread_stream_logger_t
	  {
	  public:
	    thread_stream_logger_t(
	        thread_logger_t& logger,
	        Level level,
	        :ref:`toolbox::base::object_pool_t\<std::stringstream> <doxid-classtoolbox_1_1base_1_1object__pool__t>`& pool);
	
	    ~thread_stream_logger_t();
	
	    // Deleted copy/move constructors/assignments
	    thread_stream_logger_t(const thread_stream_logger_t&) = delete;
	    thread_stream_logger_t& operator=(const thread_stream_logger_t&) = delete;
	    thread_stream_logger_t(thread_stream_logger_t&&) = delete;
	    thread_stream_logger_t& operator=(thread_stream_logger_t&&) = delete;
	
	    auto str() -> std::string
	    {
	      return ss_ptr_->str();
	    }  // Use pointer
	
	    template<typename T>
	    auto :ref:`operator\<\< <doxid-namespacetoolbox_1_1types_1a1249a479c6547ab56be6a1907421ccf2>`(const T& container)
	        -> std::enable_if_t<is_container_v<T> && !has_stream_operator_v<T>,
	                            thread_stream_logger_t&>
	    {
	      if (level_ < logger_.level()) {
	        return *this;
	      }
	      *ss_ptr_ << "[";  // Use pointer
	      bool first = true;
	      for (const auto& item : container) {
	        if (!first) {
	          *ss_ptr_ << ", ";
	        }
	        *ss_ptr_ << item;  // Use pointer
	        first = false;
	      }
	      *ss_ptr_ << "]";  // Use pointer
	      return *this;
	    }
	
	    template<typename... Args>
	    auto :ref:`operator\<\< <doxid-namespacetoolbox_1_1types_1a1249a479c6547ab56be6a1907421ccf2>`(const std::tuple<Args...>& t) -> thread_stream_logger_t&
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
	
	    auto :ref:`operator\<\< <doxid-namespacetoolbox_1_1types_1a1249a479c6547ab56be6a1907421ccf2>`(const char* value) -> thread_stream_logger_t&
	    {
	      if (level_ < logger_.level()) {
	        return *this;
	      }
	      *ss_ptr_ << value;  // Use pointer
	      return *this;
	    }
	
	    template<typename T>
	    auto :ref:`operator\<\< <doxid-namespacetoolbox_1_1types_1a1249a479c6547ab56be6a1907421ccf2>`(const T& value)
	        -> std::enable_if_t<has_stream_operator_v<T>, thread_stream_logger_t&>
	    {
	      if (level_ < logger_.level()) {
	        return *this;
	      }
	      *ss_ptr_ << value;  // Use pointer
	      return *this;
	    }
	
	    template<typename T>
	    auto :ref:`operator\<\< <doxid-namespacetoolbox_1_1types_1a1249a479c6547ab56be6a1907421ccf2>`(T&& value) -> std::enable_if_t<
	        !has_stream_operator_v<T> && has_ostream_method_v<T>,
	        thread_stream_logger_t&>
	    {
	      if (level_ < logger_.level()) {
	        return *this;
	      }
	      value.operator<<(*ss_ptr_);  // Use pointer
	      return *this;
	    }
	
	    auto :ref:`operator\<\< <doxid-namespacetoolbox_1_1types_1a1249a479c6547ab56be6a1907421ccf2>`(thread_stream_logger_t& logger) -> thread_stream_logger_t&
	    {
	      *ss_ptr_ << logger.str();  // Use pointer
	      return *this;
	    }
	
	    template<typename K, typename V>
	    auto :ref:`operator\<\< <doxid-namespacetoolbox_1_1types_1a1249a479c6547ab56be6a1907421ccf2>`(const std::map<K, V>& :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`) -> thread_stream_logger_t&
	    {
	      if (level_ < logger_.level()) {
	        return *this;
	      }
	      *ss_ptr_ << "{";  // Use pointer
	      bool first = true;
	      for (const auto& [key, value] : :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`) {
	        if (!first) {
	          *ss_ptr_ << ", ";
	        }
	        *ss_ptr_ << key << ": " << value;  // Use pointer
	        first = false;
	      }
	      *ss_ptr_ << "}";  // Use pointer
	      return *this;
	    }
	
	    template<typename K, typename V>
	    auto :ref:`operator\<\< <doxid-namespacetoolbox_1_1types_1a1249a479c6547ab56be6a1907421ccf2>`(const std::unordered_map<K, V>& :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`)
	        -> thread_stream_logger_t&
	    {
	      if (level_ < logger_.level())
	        return *this;
	      *ss_ptr_ << "{";  // Use pointer
	      bool first = true;
	      for (const auto& [key, value] : :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`) {
	        if (!first)
	          *ss_ptr_ << ", ";
	        *ss_ptr_ << key << ": " << value;  // Use pointer
	        first = false;
	      }
	      *ss_ptr_ << "}";  // Use pointer
	      return *this;
	    }
	
	  private:
	    template<typename Tuple, size_t... Is>
	    auto print_tuple(const Tuple& t, std::index_sequence<Is...>) -> void
	    {
	      if (level_ < logger_.level()) {
	        return;
	      }
	      *ss_ptr_ << "(";  // Use pointer
	      ((*ss_ptr_ << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);
	      *ss_ptr_ << ")";  // Use pointer
	    }
	
	    thread_logger_t& logger_;  
	    Level level_;  
	    // Store the stringstream in a unique_ptr obtained from the pool
	    :ref:`toolbox::base::object_pool_t\<std::stringstream>::PooledObjectPtr <doxid-classtoolbox_1_1base_1_1object__pool__t_1ae721bc932699daa4c5e521583b54eb5f>` ss_ptr_;
	  };
	
	  auto trace_f() -> thread_format_logger_t
	  {
	    return {*this, Level::TRACE};
	  }
	
	  auto debug_f() -> thread_format_logger_t
	  {
	    return {*this, Level::DEBUG};
	  }
	
	  auto info_f() -> thread_format_logger_t
	  {
	    return {*this, Level::INFO};
	  }
	
	  auto warn_f() -> thread_format_logger_t
	  {
	    return {*this, Level::WARN};
	  }
	
	  auto error_f() -> thread_format_logger_t
	  {
	    return {*this, Level::ERROR};
	  }
	
	  auto critical_f() -> thread_format_logger_t
	  {
	    return {*this, Level::CRITICAL};
	  }
	
	  auto trace_s() -> thread_stream_logger_t
	  {
	    return {*this, Level::TRACE, stringstream_pool_};
	  }
	
	  auto debug_s() -> thread_stream_logger_t
	  {
	    return {*this, Level::DEBUG, stringstream_pool_};
	  }
	
	  auto info_s() -> thread_stream_logger_t
	  {
	    return {*this, Level::INFO, stringstream_pool_};
	  }
	
	  auto warn_s() -> thread_stream_logger_t
	  {
	    return {*this, Level::WARN, stringstream_pool_};
	  }
	
	  auto error_s() -> thread_stream_logger_t
	  {
	    return {*this, Level::ERROR, stringstream_pool_};
	  }
	
	  auto critical_s() -> thread_stream_logger_t
	  {
	    return {*this, Level::CRITICAL, stringstream_pool_};
	  }
	
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
	  :ref:`toolbox::container::concurrent_queue_t\<std::pair\<Level, std::string> <doxid-classtoolbox_1_1container_1_1concurrent__queue__t>`> queue_;
	  // Worker thread member remains
	  std::thread worker_;
	  std::atomic<bool> running_ {false};
	  Level level_ = Level::INFO;
	
	  // Static members for singleton pattern
	  static std::atomic<thread_logger_t*> instance_ptr_;
	  static std::mutex instance_mutex_;
	  static std::atomic<bool> shutdown_called_;
	
	  // Add the stringstream pool
	  :ref:`toolbox::base::object_pool_t\<std::stringstream> <doxid-classtoolbox_1_1base_1_1object__pool__t>` stringstream_pool_;
	};
	
	}  // namespace toolbox::logger
	
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



.. rubric:: Returns:

A thread_stream_logger_t instance configured for INFO level
