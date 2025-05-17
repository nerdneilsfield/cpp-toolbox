#include <chrono>
#include <iostream>
#include <optional>
#include <stdexcept>  // For std::stoi etc. exceptions
#include <string>
#include <utility>  // For std::pair
#include <vector>

#include <cpp-toolbox/logger/thread_logger.hpp>  // Include logger for output
#include <cpp-toolbox/utils/click.hpp>
#include <cpp-toolbox/utils/ini_config.hpp>
#include <cpp-toolbox/utils/ini_struct.hpp>

// Use namespaces for brevity
using namespace toolbox::utils;
using namespace toolbox::logger;

struct basic_ini
{
  std::string key1;
  int key2 = 0;
};

TOOLBOX_INI_STRUCT(basic_ini,
    TOOLBOX_INI_FIELD(basic_ini, key1, "section1", "key1"),
    TOOLBOX_INI_FIELD(basic_ini, key2, "section1", "key2"))

/**
 * @brief Callback function for the 'process' subcommand.
 *
 * @param input_file The required input file path argument.
 * @param output_file The optional output file path option.
 * @param count The required count option.
 * @param threshold The optional threshold option with default value.
 * @param retries The optional retries option (std::optional<int>).
 * @param coords The optional coordinates option with custom parser.
 * @return int Exit code (0 for success).
 */
int process_callback(const argument_t<std::string>& input_file,
                     const option_t<std::string>& output_file,
                     const option_t<int>& count,
                     const option_t<double>& threshold,
                     const option_t<std::optional<int>>& retries,
                     const option_t<std::optional<std::pair<int, int>>>& coords)
{
  LOG_INFO_S << "--- Running 'process' command ---";

  // Access required arguments/options directly
  LOG_INFO_S << "Input file: " << input_file.get();
  LOG_INFO_S << "Count: " << count.get();

  // Access options with defaults (check if set if needed)
  if (output_file.is_set()) {
    LOG_INFO_S << "Output file: " << output_file.get();
  } else {
    LOG_INFO_S << "Output file: (Using default based on input)";
    // Default logic would go here, get() returns default if not set
  }
  LOG_INFO_S << "Threshold: " << threshold.get() << " (Default was 0.5)";

  // Access optional<T> options
  if (retries.get().has_value()) {
    LOG_INFO_S << "Retries specified: " << retries.get().value();
  } else {
    LOG_INFO_S << "Retries: Not specified.";
  }

  if (coords.get().has_value()) {
    LOG_INFO_S << "Coordinates specified: (" << coords.get().value().first
               << ", " << coords.get().value().second << ")";
  } else {
    LOG_INFO_S << "Coordinates: Not specified.";
  }

  LOG_INFO_S << "Processing data...";
  // ... Add actual processing logic here ...
  LOG_INFO_S << "--- 'process' command finished ---";
  return 0;
}

void void_function()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

/**
 * @brief Callback function for the 'info' subcommand.
 * @return int Exit code (0 for success).
 */
int info_callback()
{
  // Combine log messages into a single chained call for robustness
  LOG_INFO_S << "--- Running 'info' command ---";
  LOG_INFO_S << "\nThis is the example application v1.0.";
  LOG_INFO_S << "\nUse --help for more details.";
  LOG_INFO_S << "\n--- 'info' command finished ---";
  return 0;
}

/**
 * @brief Main entry point for the example application.
 *
 * Demonstrates setting up commands, options, arguments, and subcommands
 * using the click.hpp library.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int Exit code from the command execution.
 */
int main(int argc, char** argv)
{
  // Initialize the logger (optional, if you want logging)
  auto& logger = thread_logger_t::instance();
  logger.set_level(thread_logger_t::Level::INFO);  // Set desired log level

  // --- Pre scan for --ini to load configuration before CLI parsing ---
  std::string ini_path = "example/example.ini";
  std::vector<std::string> remaining_args;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--ini") {
      if (i + 1 < argc) {
        ini_path = argv[++i];
      } else {
        LOG_WARN_S << "--ini option requires a file path";
      }
    } else {
      remaining_args.push_back(arg);
    }
  }

  ini_config_t ini_cfg;
  ini_cfg.load(ini_path);


  basic_ini cfg_struct{};
  load_struct_from_ini(ini_cfg, cfg_struct);
  LOG_INFO_S << "INI struct key1=" << cfg_struct.key1
             << " key2=" << cfg_struct.key2;
  /** @brief Main application object */
  CommandLineApp app(
      "example_cli",
      "An example CLI application demonstrating click.hpp features.");

  // --- Global Options ---
  /** @brief Verbosity flag */
  auto& verbose_flag =
      app.add_flag("verbose", "v", "Enable verbose output (global).");
  /** @brief Global configuration file option */
  auto& config_opt =
      app.add_option<std::string>(
             "config", "c", "Path to configuration file (global).")
          .set_default("config.json");

  // --- 'process' Subcommand ---
  /** @brief Process subcommand definition */
  auto& process_cmd = app.add_command("process", "Process input data.");

  // Options for 'process'
  /** @brief Required output file path */
  auto& process_output_opt =
      process_cmd.add_option<std::string>("output", "o", "Output file path.")
          .set_default("output.dat");  // Default if not provided
  /** @brief Required count value */
  auto& process_count_opt = process_cmd.add_option<int>(
      "count", "n", "Number of items to process.", true);  // Required
  /** @brief Optional threshold value */
  auto& process_threshold_opt =
      process_cmd.add_option<double>("threshold", "t", "Processing threshold.")
          .set_default(0.5);
  /** @brief Optional number of retries */
  auto& process_retries_opt = process_cmd.add_option<std::optional<int>>(
      "retries", "r", "Number of retries on failure.");

  /** @brief Optional coordinates with custom parser */
  auto& process_coords_opt =
      process_cmd
          .add_option<std::optional<std::pair<int, int>>>(
              "coords",
              "",
              "Coordinates in x,y format (e.g., 10,20).")  // No short name
          .set_parser(
              [](const std::string& value,
                 std::optional<std::pair<int, int>>& result)
              {
                size_t comma_pos = value.find(',');
                if (comma_pos == std::string::npos) {
                  LOG_WARN_S << "Custom parser: Invalid coords format (missing "
                                "comma): "
                             << value;
                  result = std::nullopt;  // Invalid format for optional results
                                          // in nullopt
                  return true;  // Parser itself succeeded, even if value was
                                // invalid for optional
                }
                try {
                  int x = std::stoi(value.substr(0, comma_pos));
                  if (comma_pos + 1 >= value.length()) {
                    LOG_WARN_S
                        << "Custom parser: Invalid coords format (missing y): "
                        << value;
                    result = std::nullopt;
                    return true;
                  }
                  int y = std::stoi(value.substr(comma_pos + 1));
                  result = std::make_pair(x, y);
                  return true;
                } catch (const std::invalid_argument&) {
                  LOG_WARN_S
                      << "Custom parser: Invalid number format in coords: "
                      << value;
                  result = std::nullopt;
                  return true;
                } catch (const std::out_of_range&) {
                  LOG_WARN_S << "Custom parser: Number out of range in coords: "
                             << value;
                  result = std::nullopt;
                  return true;
                }
              });

  // Argument for 'process'
  /** @brief Required input file path */
  auto& process_input_arg = process_cmd.add_argument<std::string>(
      "input_file", "Path to the input data file.", true);  // Required

  // Set callback for 'process'
  process_cmd.set_callback(
      [&]()
      {
        // Access global flags/options if needed (though typically handled
        // before dispatch)
        if (verbose_flag.get()) {
          logger.set_level(
              thread_logger_t::Level::TRACE);  // Example: increase verbosity
          LOG_TRACE_S << "Verbose mode enabled globally.";
        }
        LOG_INFO_S << "Using global config: " << config_opt.get();

        // Call the actual processing logic, passing the captured
        // options/arguments
        return process_callback(process_input_arg,
                                process_output_opt,
                                process_count_opt,
                                process_threshold_opt,
                                process_retries_opt,
                                process_coords_opt);
      });

  // --- 'info' Subcommand ---
  /** @brief Info subcommand definition */
  auto& info_cmd = app.add_command("info", "Display application information.");
  info_cmd.set_callback(info_callback);

  // Apply INI configuration after all options and commands are defined
  app.apply_ini_config(ini_cfg, "example_cli");

  // --- Run the Application ---
  int exit_code = 0;
  try {
    // Parse and execute using arguments with the optional --ini removed
    exit_code = app.parse_and_execute(remaining_args);
  } catch (const std::exception& e) {
    // Catch potential errors during setup (e.g., invalid_argument from checks)
    // Note: ClickExceptions during run() are caught internally by app.run()
    LOG_CRITICAL_S << "Unhandled exception during setup: " << e.what();
    exit_code = 1;
  }

  //   void_function();
  // Explicitly shutdown the logger before exiting
  thread_logger_t::shutdown();
  return exit_code;
}
