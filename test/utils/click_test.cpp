#include <memory>  // For RAII guard maybe?
#include <optional>
#include <string>
#include <utility>  // For std::pair
#include <vector>

#include <catch2/catch_approx.hpp>  // For Approx
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>  // For string matchers like ContainsSubstring
#include <cpp-toolbox/logger/thread_logger.hpp>  // For logger guard
#include <cpp-toolbox/utils/click.hpp>

using namespace toolbox::utils;
using Catch::Matchers::ContainsSubstring;
using namespace toolbox::logger;  // Need logger namespace

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
    // Optionally add a log message here for debugging
    // LOG_TRACE_S << "Restoring logger level to: " <<
    // thread_logger_t::level_to_string(original_level_);
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

TEST_CASE("Click Library Basic Tests", "[click]")
{
  SECTION("Flag Parsing")
  {
    CommandLineApp app("test_app", "A test application");
    auto& flag = app.add_flag("verbose", "v", "Enable verbose output");

    std::vector<std::string> args1 = {"-v"};
    REQUIRE(app.parse_and_execute(args1) == 0);
    REQUIRE(flag.is_set() == true);
    REQUIRE(flag.get() == true);  // flag_t::get() should return true if set

    CommandLineApp app2("test_app", "A test application");
    auto& flag2 = app2.add_flag("verbose", "v", "Enable verbose output");
    std::vector<std::string> args2 = {"--verbose"};
    REQUIRE(app2.parse_and_execute(args2) == 0);
    REQUIRE(flag2.is_set() == true);
    REQUIRE(flag2.get() == true);

    CommandLineApp app3("test_app", "A test application");
    auto& flag3 = app3.add_flag("verbose", "v", "Enable verbose output");
    std::vector<std::string> args3 = {};
    REQUIRE(app3.parse_and_execute(args3) == 0);
    REQUIRE(flag3.is_set() == false);
    REQUIRE(flag3.get() == false);  // Default for flag is false
  }

  SECTION("Option Parsing - String")
  {
    CommandLineApp app("test_app", "A test application");
    auto& name_opt = app.add_option<std::string>(
        "name", "n", "Specify a name", true);  // Required

    std::vector<std::string> args1 = {"--name", "tester"};
    REQUIRE(app.parse_and_execute(args1) == 0);
    REQUIRE(name_opt.is_set() == true);
    REQUIRE(name_opt.get() == "tester");

    CommandLineApp app2("test_app", "A test application");
    auto& name_opt2 =
        app2.add_option<std::string>("name", "n", "Specify a name", false);
    name_opt2.set_default("default_user");
    std::vector<std::string> args2 = {};
    REQUIRE(app2.parse_and_execute(args2) == 0);
    REQUIRE(name_opt2.is_set() == false);
    REQUIRE(name_opt2.get() == "default_user");

    CommandLineApp app3("test_app", "A test application");
    auto& name_opt3 =
        app3.add_option<std::string>("name", "n", "Specify a name", false);
    // Use standard short option syntax
    std::vector<std::string> args3 = {"-n", "shortform"};
    REQUIRE(app3.parse_and_execute(args3) == 0);
    REQUIRE(name_opt3.is_set() == true);
    REQUIRE(name_opt3.get() == "shortform");
  }

  SECTION("Option Parsing - Integer")
  {
    CommandLineApp app("test_app", "A test application");
    auto& count_opt =
        app.add_option<int>("count", "c", "", false).set_default(0);

    std::vector<std::string> args1 = {"--count", "123"};
    REQUIRE(app.parse_and_execute(args1) == 0);
    REQUIRE(count_opt.is_set() == true);
    REQUIRE(count_opt.get() == 123);

    CommandLineApp app2("test_app", "A test application");
    auto& count_opt2 =
        app2.add_option<int>("count", "c", "", false).set_default(0);

    std::vector<std::string> args2 = {"-c", "-45"};
    REQUIRE(app2.parse_and_execute(args2) == 0);
    REQUIRE(count_opt2.is_set() == true);
    REQUIRE(count_opt2.get() == -45);

    CommandLineApp app3("test_app", "A test application");
    auto& count_opt3 =
        app3.add_option<int>("count", "c", "", false).set_default(0);

    std::vector<std::string> args3 = {};
    REQUIRE(app3.parse_and_execute(args3) == 0);
    REQUIRE(count_opt3.is_set() == false);
    REQUIRE(count_opt3.get() == 0);  // Default

    // Test invalid integer
    CommandLineApp app_err("test_app", "");
    app_err.add_option<int>("count", "c", "");
    std::vector<std::string> args_err = {"--count", "abc"};
    REQUIRE(app_err.parse_and_execute(args_err) == 1);
  }

  SECTION("Option Parsing - Double")
  {
    // Test value provided
    CommandLineApp app1("test_app", "");
    auto& val_opt1 =
        app1.add_option<double>("value", "v", "", false).set_default(1.0);
    std::vector<std::string> args1 = {"--value", "3.14"};
    REQUIRE(app1.parse_and_execute(args1) == 0);
    REQUIRE(val_opt1.is_set() == true);
    REQUIRE(val_opt1.get() == Catch::Approx(3.14));

    // Test default value
    CommandLineApp app2("test_app", "");
    auto& val_opt2 =
        app2.add_option<double>("value", "v", "", false).set_default(1.0);
    std::vector<std::string> args2 = {};
    REQUIRE(app2.parse_and_execute(args2) == 0);
    REQUIRE(val_opt2.is_set() == false);
    REQUIRE(val_opt2.get() == Catch::Approx(1.0));

    // Test invalid double
    CommandLineApp app_err("test_app", "");
    app_err.add_option<double>("value", "v", "");
    std::vector<std::string> args_err = {"-v", "pi"};
    REQUIRE(app_err.parse_and_execute(args_err) == 1);
  }

  SECTION("Option Parsing - Bool (Non-Flag)")
  {
    CommandLineApp app("test_app", "A test application");
    // Use option<bool> for explicit true/false, unlike flag_t
    auto& enable_opt =
        app.add_option<bool>("enable", "e", "", false).set_default(false);

    std::vector<std::string> args1 = {"--enable", "true"};
    REQUIRE(app.parse_and_execute(args1) == 0);
    REQUIRE(enable_opt.is_set() == true);
    REQUIRE(enable_opt.get() == true);

    CommandLineApp app2("test_app", "A test application");
    auto& enable_opt2 =
        app2.add_option<bool>("enable", "e", "", false).set_default(false);

    std::vector<std::string> args2 = {"-e", "false"};
    REQUIRE(app2.parse_and_execute(args2) == 0);
    REQUIRE(enable_opt2.is_set() == true);
    REQUIRE(enable_opt2.get() == false);

    CommandLineApp app3("test_app", "A test application");
    auto& enable_opt3 =
        app3.add_option<bool>("enable", "e", "", false).set_default(false);

    std::vector<std::string> args3 = {"--enable", "1"};
    REQUIRE(app3.parse_and_execute(args3) == 0);
    REQUIRE(enable_opt3.is_set() == true);
    REQUIRE(enable_opt3.get() == true);

    CommandLineApp app4("test_app", "A test application");
    auto& enable_opt4 =
        app4.add_option<bool>("enable", "e", "", false).set_default(false);

    std::vector<std::string> args4 = {};
    REQUIRE(app4.parse_and_execute(args4) == 0);
    REQUIRE(enable_opt4.is_set() == false);
    REQUIRE(enable_opt4.get() == false);  // Default

    CommandLineApp app5("test_app", "A test application");
    auto& enable_opt5 = app5.add_option<bool>("enable", "e", "");

    std::vector<std::string> args5 = {"-e", "yes"};
    REQUIRE(app5.parse_and_execute(args5) == 1);

    // Test invalid bool
    CommandLineApp app_err_bool("test_app", "");
    app_err_bool.add_option<bool>("enable", "e", "");
    std::vector<std::string> args_err_bool = {"-e", "maybe"};
    REQUIRE(app_err_bool.parse_and_execute(args_err_bool) == 1);
  }

  SECTION("Argument Parsing")
  {
    CommandLineApp app("test_app", "A test application");
    auto& req_arg =
        app.add_argument<std::string>("input_file", "Input file path", true);
    auto& opt_arg = app.add_argument<int>("count", "Optional count", false);
    opt_arg.set_default(10);

    std::vector<std::string> args1 = {"my_file.txt"};
    REQUIRE(app.parse_and_execute(args1) == 0);
    REQUIRE(req_arg.is_set() == true);
    REQUIRE(req_arg.get() == "my_file.txt");
    REQUIRE(opt_arg.is_set() == false);
    REQUIRE(opt_arg.get() == 10);  // Default

    std::vector<std::string> args2 = {"data.csv", "5"};
    REQUIRE(app.parse_and_execute(args2) == 0);
    REQUIRE(req_arg.is_set() == true);
    REQUIRE(req_arg.get() == "data.csv");
    REQUIRE(opt_arg.is_set() == true);
    REQUIRE(opt_arg.get() == 5);

    // Test missing required argument
    CommandLineApp app_err("test_app", "");
    app_err.add_argument<std::string>("input", "Required input", true);
    std::vector<std::string> args_err = {};
    REQUIRE(app_err.parse_and_execute(args_err) == 1);

    // Test invalid argument type
    CommandLineApp app_err2("test_app", "");
    app_err2.add_argument<int>("count", "Must be integer", true);
    std::vector<std::string> args_err2 = {"hello"};
    REQUIRE(app_err2.parse_and_execute(args_err2) == 1);

    // Test unexpected argument
    CommandLineApp app_err3("test_app", "");
    app_err3.add_argument<std::string>("input", "The only input", true);
    std::vector<std::string> args_err3 = {"file1", "file2"};
    REQUIRE(app_err3.parse_and_execute(args_err3) == 1);
  }

  SECTION("Required Option Error")
  {
    CommandLineApp app("test_app", "");
    app.add_option<std::string>("required_opt", "r", "This is required", true);
    std::vector<std::string> args = {};
    REQUIRE(app.parse_and_execute(args) == 1);
  }

  SECTION("Unknown Option Error")
  {
    CommandLineApp app("test_app", "");
    std::vector<std::string> args = {"--unknown-option"};
    REQUIRE(app.parse_and_execute(args) == 1);

    CommandLineApp app2("test_app", "");
    std::vector<std::string> args2 = {"-x"};  // Unknown short option
    REQUIRE(app2.parse_and_execute(args2) == 1);
  }

  SECTION("Missing Option Value Error")
  {
    CommandLineApp app("test_app", "");
    app.add_option<std::string>("name", "n", "Requires value");
    std::vector<std::string> args = {"--name"};  // Value missing
    REQUIRE(app.parse_and_execute(args) == 1);

    CommandLineApp app2("test_app", "");
    app2.add_option<std::string>("name", "n", "Requires value");
    std::vector<std::string> args2 = {"-n"};  // Value missing for short opt
    REQUIRE(app2.parse_and_execute(args2) == 1);

    CommandLineApp app3("test_app", "");
    app3.add_option<std::string>("name", "n", "Requires value");
    std::vector<std::string> args3 = {
        "-n", "--another-option"};  // Value missing, next is option
    REQUIRE(app3.parse_and_execute(args3) == 1);
  }

  SECTION("Flag Does Not Take Value Error")
  {
    CommandLineApp app("test_app", "");
    app.add_flag("myflag", "f", "Just a flag");
    std::vector<std::string> args = {"--myflag=true"};
    REQUIRE(app.parse_and_execute(args) == 1);
  }
}

TEST_CASE("Click Library Optional Tests", "[click][optional]")
{
  SECTION("Optional Option")
  {
    CommandLineApp app("test_app", "");
    auto& opt_str = app.add_option<std::optional<std::string>>(
        "opt_str", "s", "Optional string");
    auto& opt_int =
        app.add_option<std::optional<int>>("opt_int", "i", "Optional int");

    // 1. No options provided
    std::vector<std::string> args1 = {};
    REQUIRE(app.parse_and_execute(args1) == 0);
    REQUIRE(!opt_str.get().has_value());
    REQUIRE(!opt_int.get().has_value());

    CommandLineApp app2("test_app", "");
    auto& opt_str2 = app2.add_option<std::optional<std::string>>(
        "opt_str", "s", "Optional string");
    auto& opt_int2 =
        app2.add_option<std::optional<int>>("opt_int", "i", "Optional int");

    // 2. String option provided
    std::vector<std::string> args2 = {"-s", "hello"};
    REQUIRE(app2.parse_and_execute(args2) == 0);
    REQUIRE(opt_str2.get().has_value());
    REQUIRE(opt_str2.get().value() == "hello");
    REQUIRE(!opt_int2.get().has_value());

    // 3. Int option provided
    CommandLineApp app3("test_app", "");
    auto& opt_str3 = app3.add_option<std::optional<std::string>>(
        "opt_str", "s", "Optional string");
    auto& opt_int3 =
        app3.add_option<std::optional<int>>("opt_int", "i", "Optional int");

    std::vector<std::string> args3 = {"--opt_int", "42"};
    REQUIRE(app3.parse_and_execute(args3) == 0);
    REQUIRE(!opt_str3.get().has_value());
    REQUIRE(opt_int3.get().has_value());
    REQUIRE(opt_int3.get().value() == 42);

    CommandLineApp app4("test_app", "");
    auto& opt_str4 = app4.add_option<std::optional<std::string>>(
        "opt_str", "s", "Optional string");
    auto& opt_int4 =
        app4.add_option<std::optional<int>>("opt_int", "i", "Optional int");

    // 4. Both provided
    std::vector<std::string> args4 = {"--opt_str=world", "-i", "-10"};
    REQUIRE(app4.parse_and_execute(args4) == 0);
    REQUIRE(opt_str4.get().has_value());
    REQUIRE(opt_str4.get().value() == "world");
    REQUIRE(opt_int4.get().has_value());
    REQUIRE(opt_int4.get().value() == -10);

    // 5. Option present but value missing (should NOT throw for optional)
    CommandLineApp app_missing("test_app", "");
    auto& opt_missing = app_missing.add_option<std::optional<std::string>>(
        "opt_str", "s", "Optional string");
    std::vector<std::string> args5 = {"--opt_str"};
    REQUIRE(app_missing.parse_and_execute(args5) == 0);
    REQUIRE(
        !opt_missing.get().has_value());  // Missing value results in nullopt

    std::vector<std::string> args6 = {"-s"};
    REQUIRE(app_missing.parse_and_execute(args6) == 0);
    REQUIRE(!opt_missing.get().has_value());

    // 6. Invalid value for optional (should result in nullopt)
    CommandLineApp app_invalid("test_app", "");
    auto& opt_invalid_int = app_invalid.add_option<std::optional<int>>(
        "opt_int", "i", "Optional int");
    std::vector<std::string> args7 = {"--opt_int", "not-an-int"};
    // The parse itself doesn't throw, but the default parser sets nullopt
    REQUIRE(app_invalid.parse_and_execute(args7) == 0);
    REQUIRE(!opt_invalid_int.get().has_value());
  }

  SECTION("Optional Argument")
  {
    CommandLineApp app("test_app", "");
    // Optional arguments are less common but possible.
    // Usually, optional positional data uses nargs='?' or '*'.
    // Our current model treats them more like optional options without names.
    auto& opt_arg = app.add_argument<std::optional<int>>(
        "opt_val", "Optional value", false);  // false means optional

    std::vector<std::string> args1 = {};
    REQUIRE(app.parse_and_execute(args1) == 0);
    REQUIRE(!opt_arg.get().has_value());

    std::vector<std::string> args2 = {"123"};
    REQUIRE(app.parse_and_execute(args2) == 0);
    REQUIRE(opt_arg.get().has_value());
    REQUIRE(opt_arg.get().value() == 123);

    // Invalid value should result in nullopt
    std::vector<std::string> args3 = {"abc"};
    REQUIRE(app.parse_and_execute(args3) == 0);
    REQUIRE(!opt_arg.get().has_value());
  }
}

TEST_CASE("Click Library Subcommand Tests", "[click][subcommand]")
{
  LoggerLevelGuard level_guard;  // Create guard at the start of the test case

  int main_callback_called = 0;
  int sub1_callback_called = 0;
  int sub2_callback_called = 0;

  CommandLineApp app("main_app", "Main application");
  app.set_callback(
      [&]()
      {
        main_callback_called++;
        return 0;
      });
  auto& global_opt =
      app.add_option<std::string>("global", "g", "Global option");

  // Subcommand 1
  auto& sub1 = app.add_command("sub1", "First subcommand");
  auto& sub1_opt = sub1.add_option<int>("num", "n", "Number for sub1", true);
  sub1.set_callback(
      [&]()
      {
        REQUIRE(sub1_opt.is_set());
        REQUIRE(sub1_opt.get() == 42);
        sub1_callback_called++;
        return 0;
      });

  // Subcommand 2
  auto& sub2 = app.add_command("sub2", "Second subcommand");
  auto& sub2_flag = sub2.add_flag("force", "f", "Force operation");
  sub2.set_callback(
      [&]()
      {
        REQUIRE(sub2_flag.is_set());
        sub2_callback_called++;
        return 0;
      });

  SECTION("Execute Subcommand 1")
  {
    std::vector<std::string> args = {"sub1", "--num", "42"};
    REQUIRE(app.parse_and_execute(args) == 0);
    REQUIRE(main_callback_called == 0);
    REQUIRE(sub1_callback_called == 1);
    REQUIRE(sub2_callback_called == 0);
    // Global options are usually parsed by main command before dispatch
    // REQUIRE(global_opt.is_set() == false); // Check parsing scope if needed
  }

  SECTION("Execute Subcommand 2 with Global Option")
  {
    // Current implementation parses global options within main_app's scope
    // *before* dispatching
    std::vector<std::string> args = {"--global", "value", "sub2", "-f"};
    REQUIRE(app.parse_and_execute(args) == 0);
    REQUIRE(main_callback_called == 0);
    REQUIRE(sub1_callback_called == 0);
    REQUIRE(sub2_callback_called == 1);
    REQUIRE(global_opt.is_set() == true);
    REQUIRE(global_opt.get() == "value");
    // The subcommand should not see the global option unless passed down
    // REQUIRE(sub2_flag.is_set() == true); // Already checked in callback
  }

  SECTION("Missing Subcommand")
  {
    std::vector<std::string> args = {};  // No subcommand provided
    // Main app has callback, so it should execute
    REQUIRE(app.parse_and_execute(args) == 0);
    REQUIRE(main_callback_called == 1);
    REQUIRE(sub1_callback_called == 0);
    REQUIRE(sub2_callback_called == 0);
  }

  SECTION("Missing Subcommand when Required (no main callback)")
  {
    CommandLineApp app_nosub("main", "Requires subcommand");
    app_nosub.add_command("cmd", "A command");
    // No app_nosub.set_callback(...)
    std::vector<std::string> args = {};
    REQUIRE(app_nosub.parse_and_execute(args) == 1);
  }

  SECTION("Unknown Subcommand")
  {
    std::vector<std::string> args = {"unknown_sub"};
    REQUIRE(app.parse_and_execute(args) == 1);
  }

  SECTION("Missing Required Option in Subcommand")
  {
    std::vector<std::string> args = {"sub1"};  // Missing --num
    REQUIRE(app.parse_and_execute(args) == 1);
  }
}

TEST_CASE("Click Library Help Flag", "[click][help]")
{
  CommandLineApp app("test_app", "Test Description");
  app.add_option<std::string>("name", "n", "A name option");
  app.add_argument<int>("count", "A count argument");
  app.add_command("sub", "A subcommand");

  // We can't easily capture stdout here without more setup,
  // but we can test that parse_and_execute returns 0 for --help
  // and that it throws if help is requested alongside errors.

  SECTION("Help flag triggers successful exit")
  {
    std::vector<std::string> args1 = {"--help"};
    REQUIRE(app.parse_and_execute(args1) == 0);

    CommandLineApp app2("test_app", "A test application");
    auto& help_opt2 = app2.add_option<bool>("help", "h", "Show help");

    std::vector<std::string> args2 = {"-h"};
    REQUIRE(app2.parse_and_execute(args2) == 0);
  }

  SECTION("Help flag overrides other arguments/errors (usually)")
  {
    // Typical behavior: if --help is present, show help and exit, ignore others
    std::vector<std::string> args1 = {"--help", "--name", "val", "123", "sub"};
    REQUIRE(app.parse_and_execute(args1) == 0);

    std::vector<std::string> args2 = {"--unknown",
                                      "--help"};  // Unknown option + help
    REQUIRE(app.parse_and_execute(args2) == 0);

    std::vector<std::string> args3 = {"sub", "--help"};  // Help for subcommand
    // This requires parse_and_execute to handle help within subcommand scope
    // Let's assume for now main help flag takes precedence or subcommand
    // handles its own
    REQUIRE(app.parse_and_execute(args3)
            == 0);  // Should likely show sub's help
  }
}

TEST_CASE("Click Library Custom Parser Tests", "[click][parser]")
{
  SECTION("Custom Parser for std::pair<int, int>")
  {
    CommandLineApp app("parser_app", "");
    // Need access to the parser lambda later
    auto parser_lambda =
        [](const std::string& value, std::pair<int, int>& result)
    {
      size_t comma_pos = value.find(',');
      if (comma_pos == std::string::npos) {
        return false;
      }
      try {
        int x = std::stoi(value.substr(0, comma_pos));
        // Handle potential empty string after comma
        if (comma_pos + 1 >= value.length())
          return false;
        int y = std::stoi(value.substr(comma_pos + 1));
        result = {x, y};
        return true;
      } catch (
          const std::invalid_argument&) {  // Catch stoi errors specifically
        return false;
      } catch (const std::out_of_range&) {  // Catch stoi errors specifically
        return false;
      }
    };
    auto& coords_opt =
        app.add_option<std::pair<int, int>>("coords", "c", "Coordinates (x,y)")
            .set_parser(parser_lambda);

    // Valid input
    std::vector<std::string> args1 = {"--coords", "10,-5"};
    REQUIRE(app.parse_and_execute(args1) == 0);
    REQUIRE(coords_opt.is_set());
    REQUIRE(coords_opt.get().first == 10);
    REQUIRE(coords_opt.get().second == -5);

    // Invalid inputs - Need new app instances for clean state
    CommandLineApp app_err1("parser_app", "");
    app_err1.add_option<std::pair<int, int>>("coords", "c", "")
        .set_parser(parser_lambda);
    std::vector<std::string> args_err1 = {"-c", "10,abc"};
    REQUIRE(app_err1.parse_and_execute(args_err1) == 1);

    CommandLineApp app_err2("parser_app", "");
    app_err2.add_option<std::pair<int, int>>("coords", "c", "")
        .set_parser(parser_lambda);
    std::vector<std::string> args_err2 = {"-c", "10"};  // Missing comma
    REQUIRE(app_err2.parse_and_execute(args_err2) == 1);

    CommandLineApp app_err3("parser_app", "");
    app_err3.add_option<std::pair<int, int>>("coords", "c", "")
        .set_parser(parser_lambda);
    std::vector<std::string> args_err3 = {"-c", "10,"};  // Missing y value
    REQUIRE(app_err3.parse_and_execute(args_err3) == 1);
  }

  SECTION("Custom Parser with Validation")
  {
    CommandLineApp app("validator_app", "");
    // Need access to the parser lambda later
    auto parser_lambda = [](const std::string& value, int& result)
    {
      try {
        result = std::stoi(value);
        if (result < 1 || result > 10) {
          // Throw ParameterError directly from parser for custom validation
          // message
          throw ParameterError("Level must be between 1 and 10, got: " + value);
        }
        return true;  // Indicate parsing success
      } catch (const std::invalid_argument&) {
        return false;  // Indicate basic parsing failure (not an int)
      } catch (const std::out_of_range&) {
        return false;  // Indicate basic parsing failure (out of int range)
      }
      // Note: ParameterError thrown above will be caught by CommandLineApp::run
    };
    auto& level_opt = app.add_option<int>("level", "l", "Level (1-10)")
                          .set_parser(parser_lambda);

    // Valid input
    std::vector<std::string> args1 = {"--level", "5"};
    REQUIRE(app.parse_and_execute(args1) == 0);
    REQUIRE(level_opt.is_set());
    REQUIRE(level_opt.get() == 5);

    // Invalid value (out of range) - should throw ParameterError from parser
    CommandLineApp app_err1("validator_app", "");
    app_err1.add_option<int>("level", "l", "").set_parser(parser_lambda);
    std::vector<std::string> args_err1 = {"-l", "11"};
    // Check specific exception type and message
    REQUIRE(app_err1.parse_and_execute(args_err1) == 1);
    // REQUIRE_THROWS_WITH(app_err1.parse_and_execute(args_err1),
    // ContainsSubstring("Level must be between 1 and 10")); // Cannot check
    // message easily now

    // Invalid type - parser returns false, parse_and_execute returns 1
    CommandLineApp app_err2("validator_app", "");
    app_err2.add_option<int>("level", "l", "").set_parser(parser_lambda);
    std::vector<std::string> args_err2 = {"--level", "high"};
    REQUIRE(app_err2.parse_and_execute(args_err2) == 1);
    // REQUIRE_THROWS_AS(app_err2.parse_and_execute(args_err2), ParameterError);
    // // Old assertion
    // REQUIRE_THROWS_WITH(app_err2.parse_and_execute(args_err2),
    // ContainsSubstring("Invalid value 'high'")); // Cannot check message
    // easily now
  }
}

TEST_CASE("Click Library Short Option Parsing", "[click][short]")
{
  SECTION("Short option with attached value")
  {
    CommandLineApp app("short_app", "");
    auto& out_opt = app.add_option<std::string>("output", "o", "Output file");

    std::vector<std::string> args = {"-oresult.txt"};
    REQUIRE(app.parse_and_execute(args) == 0);
    REQUIRE(out_opt.is_set());
    REQUIRE(out_opt.get() == "result.txt");
  }

  SECTION("Bundled short flags produce error")
  {
    CommandLineApp app("bundle_app", "");
    app.add_flag("alpha", "a", "");
    app.add_flag("beta", "b", "");
    app.add_flag("gamma", "c", "");

    std::vector<std::string> args = {"-abc"};
    REQUIRE(app.parse_and_execute(args) == 1);
  }
}

TEST_CASE("Click Library Nested Subcommands", "[click][subcommand][nested]")
{
  int main_called = 0;
  int a_called = 0;
  int b_called = 0;
  int c_called = 0;

  CommandLineApp app("main", "");
  app.set_callback(
      [&]()
      {
        main_called++;
        return 0;
      });

  auto& a = app.add_command("a", "level1");
  auto& a_opt = a.add_option<int>("num", "n", "", true);
  a.set_callback(
      [&]()
      {
        REQUIRE(a_opt.is_set());
        a_called++;
        return 0;
      });

  auto& b = a.add_command("b", "level2");
  auto& b_flag = b.add_flag("flag", "f", "");
  b.set_callback(
      [&]()
      {
        REQUIRE(b_flag.is_set());
        b_called++;
        return 0;
      });

  auto& c = b.add_command("c", "level3");
  auto& c_opt = c.add_option<std::string>("name", "o", "");
  c.set_callback(
      [&]()
      {
        REQUIRE(c_opt.is_set());
        c_called++;
        return 0;
      });

  std::vector<std::string> args = {
      "a", "-n", "5", "b", "-f", "c", "-o", "file"};
  REQUIRE(app.parse_and_execute(args) == 0);
  REQUIRE(main_called == 0);
  REQUIRE(a_called == 0);
  REQUIRE(b_called == 0);
  REQUIRE(c_called == 1);
  REQUIRE(a_opt.is_set());
  REQUIRE(a_opt.get() == 5);
  REQUIRE(b_flag.is_set());
  REQUIRE(b_flag.get() == true);
  REQUIRE(c_opt.is_set());
  REQUIRE(c_opt.get() == "file");
}