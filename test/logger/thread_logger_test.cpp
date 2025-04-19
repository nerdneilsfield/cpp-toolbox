#include <iostream>
#include <map>
#include <sstream>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include "cpp-toolbox/logger/thread_logger.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace toolbox::logger;

// RAII helper to restore logger level
class LoggerLevelGuard
{
public:
  LoggerLevelGuard()
      : logger_(thread_logger_t::instance())
      , original_level_(logger_.level())
  {
  }

  ~LoggerLevelGuard()
  {
    // Restore original level when guard goes out of scope
    logger_.set_level(original_level_);
  }

  // Prevent copying/moving
  LoggerLevelGuard(const LoggerLevelGuard&) = delete;
  LoggerLevelGuard& operator=(const LoggerLevelGuard&) = delete;
  LoggerLevelGuard(LoggerLevelGuard&&) = delete;
  LoggerLevelGuard& operator=(LoggerLevelGuard&&) = delete;

private:
  thread_logger_t& logger_;
  thread_logger_t::Level original_level_;
};

struct TestStruct
{
  int a;
  std::string b;

  auto operator<<(std::ostream& os) const -> std::ostream&
  {
    os << "TestStruct(" << a << ", " << b << ")";
    return os;
  }
};

struct StandardStruct
{
  int x;
  std::string y;

  friend auto operator<<(std::ostream& os, const StandardStruct& s)
      -> std::ostream&
  {
    os << "StandardStruct(" << s.x << ", " << s.y << ")";
    return os;
  }
};

TEST_CASE("ThreadLogger singleton test", "[ThreadLogger]")
{
  SECTION("get instance")
  {
    auto& logger1 = thread_logger_t::instance();
    auto& logger2 = thread_logger_t::instance();

    // check two times get the same instance
    REQUIRE(&logger1 == &logger2);
  }
}

TEST_CASE("ThreadLogger log level test", "[ThreadLogger]")
{
  LoggerLevelGuard level_guard;
  auto& logger = thread_logger_t::instance();

  // SECTION("default log level")
  // {
  //   REQUIRE(logger.level() == thread_logger_t::Level::INFO);
  //   REQUIRE(logger.level_str() == "[INFO]");
  // }

  SECTION("set log level")
  {
    logger.set_level(thread_logger_t::Level::DEBUG);
    REQUIRE(logger.level() == thread_logger_t::Level::DEBUG);
    REQUIRE(logger.level_str() == "[DEBUG]");
  }

  LOG_CRITICAL_S << "test critical log";
}

TEST_CASE("ThreadLogger format log test", "[ThreadLogger]")
{
  auto& logger = thread_logger_t::instance();
  logger.set_level(thread_logger_t::Level::TRACE);

  SECTION("basic format log")
  {
    REQUIRE_NOTHROW(LOG_INFO_F("test message {} {}", 123, "abc"));
  }

  SECTION("multiple parameters format log")
  {
    REQUIRE_NOTHROW(LOG_DEBUG_F("complex test {} {} {}", 1, 2.5, "test"));
  }
}

// !TODO: 支持 tuple, container
// TEST_CASE("ThreadLogger format log with custom struct", "[ThreadLogger]") {
//     auto& logger = ThreadLogger::instance();
//     logger.set_level(ThreadLogger::Level::TRACE);
//     REQUIRE_NOTHROW(LOG_DEBUG_F("custom struct: {}", TestStruct{42,
//     "test"})); REQUIRE_NOTHROW(LOG_DEBUG_F("standard struct: {}",
//     StandardStruct{100, "standard"}));

//     std::tuple<int, std::string, double> tuple = {1, "hello", 3.14};
//     REQUIRE_NOTHROW(LOG_DEBUG_F("tuple: {}", tuple));

//     std::vector<int> vec = {1, 2, 3, 4, 5};
//     REQUIRE_NOTHROW(LOG_DEBUG_F("vector: {}", vec));
// }

TEST_CASE("ThreadLogger stream log test", "[ThreadLogger]")
{
  auto& logger = thread_logger_t::instance();
  logger.set_level(thread_logger_t::Level::TRACE);

  SECTION("basic stream log")
  {
    REQUIRE_NOTHROW(LOG_INFO_S << "simple log message");
  }

  SECTION("multiple types stream log")
  {
    REQUIRE_NOTHROW(LOG_DEBUG_S << "number: " << 42
                                << ", float number: " << 3.14 << ", string: "
                                << "test");
  }

  SECTION("container log")
  {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    REQUIRE_NOTHROW(LOG_TRACE_S << "vector: " << vec);
  }

  SECTION("tuple log")
  {
    auto tuple = std::make_tuple(1, "hello", 3.14);
    REQUIRE_NOTHROW(LOG_DEBUG_S << "tuple: " << tuple);
  }

  SECTION("map log")
  {
    std::map<int, std::string> map = {{1, "one"}, {2, "two"}, {3, "three"}};
    REQUIRE_NOTHROW(LOG_DEBUG_S << "map: " << map);
  }

  SECTION("unordered_map log")
  {
    std::unordered_map<int, std::string> umap = {
        {1, "one"}, {2, "two"}, {3, "three"}};
    REQUIRE_NOTHROW(LOG_DEBUG_S << "unordered_map: " << umap);
  }

  SECTION("custom struct log")
  {
    TestStruct ts = {42, "test"};
    REQUIRE_NOTHROW([&] { LOG_DEBUG_S << "custom struct: " << ts; }());
  }

  SECTION("standard struct log")
  {
    StandardStruct ss = {100, "standard"};
    REQUIRE_NOTHROW([&] { LOG_DEBUG_S << "standard struct: " << ss; }());
  }
}

TEST_CASE("ThreadLogger color log test", "[ThreadLogger]")
{
  auto& logger = thread_logger_t::instance();
  logger.set_level(thread_logger_t::Level::TRACE);

  SECTION("color log")
  {
    REQUIRE_NOTHROW(
        [&]
        {
          LOG_INFO_S.red("red text") << LOG_DEBUG_S.green("green text")
                                     << LOG_WARN_S.yellow("yellow text")
                                     << LOG_ERROR_S.bold("bold text");
        }());
  }
}

TEST_CASE("ThreadLogger debug log test", "[ThreadLogger]")
{
  auto& logger = thread_logger_t::instance();
  logger.set_level(thread_logger_t::Level::TRACE);

  REQUIRE_NOTHROW(LOG_DEBUG_D("debug log"));
  REQUIRE_NOTHROW(LOG_INFO_D("info log"));
  REQUIRE_NOTHROW(LOG_WARN_D("warn log"));
  REQUIRE_NOTHROW(LOG_ERROR_D("error log"));
  REQUIRE_NOTHROW(LOG_CRITICAL_D("critical log"));
}

TEST_CASE("ThreadLogger log level filter test", "[ThreadLogger]")
{
  LoggerLevelGuard level_guard;
  auto& logger = thread_logger_t::instance();

  SECTION("log level filter")
  {
    logger.set_level(thread_logger_t::Level::ERR);

    REQUIRE_NOTHROW(LOG_ERROR_S << "should be recorded");

    // these logs should not be recorded
    REQUIRE_NOTHROW(LOG_TRACE_F("should not be recorded"));
    REQUIRE_NOTHROW(LOG_DEBUG_F("should not be recorded"));
    REQUIRE_NOTHROW(LOG_INFO_F("should not be recorded"));
    REQUIRE_NOTHROW(LOG_WARN_F("should not be recorded"));
  }
}

TEST_CASE("ThreadLogger concurrency log test", "[ThreadLogger]")
{
  auto& logger = thread_logger_t::instance();
  logger.set_level(thread_logger_t::Level::TRACE);

  SECTION("multi-thread concurrency log")
  {
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
      threads.emplace_back(
          [i]()
          {
            LOG_INFO_F("thread {} log", i);
            LOG_DEBUG_S << "thread " << i << " stream log";
          });
    }

    for (auto& t : threads) {
      t.join();
    }

    REQUIRE(true);  // if no crash, test pass
  }
}
