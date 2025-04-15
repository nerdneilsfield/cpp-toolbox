#include <algorithm>  // For std::max
#include <iostream>  // For std::cout
#include <sstream>  // For std::stringstream
#include <stdexcept>  // For std::invalid_argument
#include <unordered_map>
#include <vector>

#include <cpp-toolbox/logger/thread_logger.hpp>
#include <cpp-toolbox/utils/click.hpp>

namespace toolbox::utils
{

parameter_t::parameter_t(const std::string& name,
                         const std::string& description,
                         bool required)
    : name_(name)
    , description_(description)
    , required_(required)
    , is_set_(false)
{
  // Basic validation can be added here if needed, e.g., check if name is empty.
}

std::string parameter_t::get_name() const
{
  return name_;
}

std::string parameter_t::get_description() const
{
  return description_;
}

bool parameter_t::is_required() const
{
  return required_;
}

bool parameter_t::is_set() const
{
  return is_set_;
}

bool parameter_t::accepts_missing_value() const
{
  return false;  // Base implementation: parameters generally require values if
                 // not flags
}

std::string parameter_t::get_short_name() const
{
  return "";  // Base implementation: arguments and base don't have short names
}

flag_t::flag_t(const std::string& name,
               const std::string& short_name,
               const std::string& description)
    : option_t<bool>(name, short_name, description, false)
{
  // Flags are typically not required, default to false.
  // Set default value to false for the flag.
  set_default(false);
  // Flags don't usually take a value, their presence means true.
  // We override parse to handle this.
}

bool flag_t::parse(const std::string& /*value*/)
{
  // The presence of a flag typically means setting it to true.
  // We just need to mark it as set. The actual value (true)
  // can be inferred by the fact that is_set_ is true.
  is_set_ = true;  // Mark the flag as set.
  return true;  // Successfully parsed (presence detected).
}

std::string flag_t::get_short_name() const
{
  // flag_t inherits from option_t<bool>, which stores the short name.
  // We can call the base class implementation directly.
  return option_t<bool>::get_short_name();
}

bool flag_t::get() const
{
  // For flags, the value is true if it was set (present), false otherwise.
  return is_set_;
}

command_t::command_t(const std::string& name, const std::string& description)
    : name_(name)
    , description_(description)
    , help_flag_added_(false)
    , help_requested_(false)
{
  // Automatically add a help flag unless disabled
  add_help();
}

std::string command_t::get_name() const
{
  return name_;
}

std::string command_t::get_description() const
{
  return description_;
}

command_t& command_t::add_command(const std::string& name,
                                  const std::string& description)
{
  subcommands_.push_back(std::make_unique<command_t>(name, description));
  return *subcommands_.back();
}

command_t& command_t::set_callback(std::function<int()> callback)
{
  callback_ = std::move(callback);
  return *this;
}

void command_t::add_help()
{
  if (!help_flag_added_) {  // Add help flag if not already added
    add_flag("help", "h", "Show this message and exit.");
    help_flag_added_ = true;
  }
}

[[nodiscard]] bool command_t::handle_help_flag() const
{
  for (const auto& param : parameters_)
  {  // Check if the parameter is a flag named "help"
    if (auto* flag = dynamic_cast<flag_t*>(param.get());
        flag && flag->get_name() == "help" && flag->is_set())
    {  // If help flag is set, print help and return true
      LOG_INFO_S << format_help();
      return true;
    }
  }
  return false;  // Help flag not set
}

// Helper to format list of available commands for error messages
[[nodiscard]] std::string command_t::format_command_list() const
{
  if (subcommands_.empty()) {
    return "";
  }
  std::stringstream ss;
  ss << " Available commands:";
  for (const auto& cmd : subcommands_) {
    ss << " " << cmd->get_name();
  }
  return ss.str();
}

std::string command_t::format_help() const
{
  std::stringstream ss;
  ss << "Usage: " << name_ << " [OPTIONS] [ARGUMENTS]";
  if (!subcommands_.empty()) {
    ss << " COMMAND";
  }
  ss << "\n\n";

  if (!description_.empty()) {
    ss << description_ << "\n\n";
  }

  // Separate parameters into options and arguments using virtual functions
  std::vector<const parameter_t*> options;
  std::vector<const parameter_t*> arguments;
  for (const auto& param : parameters_) {
    if (param->is_option()) {
      options.push_back(param.get());
    } else if (param->is_argument()) {
      arguments.push_back(param.get());
    }
  }

  // Calculate max length for alignment
  size_t max_len = 0;
  for (const auto* option : options) {
    std::string header = "--" + option->get_name();
    std::string short_name = option->get_short_name();
    if (!short_name.empty()) {
      header += ", -" + short_name;
    }
    // Add placeholder for value if it's not a flag
    if (dynamic_cast<const flag_t*>(option) == nullptr
        && !option->accepts_missing_value())
    {
      // Check accepts_missing_value too? Or add is_flag virtual?
      // Let's assume non-flags might need a value placeholder
      // A dedicated is_flag() virtual function would be cleaner.
      header += " <value>";  // Approximate type hint
    }
    max_len = std::max(max_len, header.length());
  }
  for (const auto* argument : arguments) {
    max_len = std::max(max_len, argument->get_name().length());
  }
  for (const auto& cmd : subcommands_) {
    max_len = std::max(max_len, cmd->get_name().length());
  }

  const size_t indent = 4;
  const size_t spacing = 2;
  size_t description_col = max_len + indent + spacing;

  // Print Options
  if (!options.empty()) {
    ss << "Options:\n";
    for (const auto* option : options) {
      std::string header = "--" + option->get_name();
      std::string short_name = option->get_short_name();
      std::string type_hint;
      if (!short_name.empty()) {
        header += ", -" + short_name;
      }
      // Basic type hint - check if it's a flag
      if (dynamic_cast<const flag_t*>(option) == nullptr
          && !option->accepts_missing_value())
      {
        type_hint =
            " <value>";  // Placeholder for non-flag, non-optional options
      }

      ss << std::string(indent, ' ') << header << type_hint;
      // Calculate padding
      size_t current_len = indent + header.length() + type_hint.length();
      if (current_len < description_col) {
        ss << std::string(description_col - current_len, ' ');
      } else {
        ss << " ";  // Ensure at least one space
      }
      ss << option->get_description();
      if (option->is_required()) {  // is_required() is safe on base
        ss << " [required]";
      }
      // TODO: Add default value display if needed (would need virtual
      // get_default_as_string())
      ss << "\n";
    }
    ss << "\n";
  }

  // Print Arguments
  if (!arguments.empty()) {
    ss << "Arguments:\n";
    for (const auto* argument : arguments) {
      std::string header = argument->get_name();
      ss << std::string(indent, ' ') << header;
      // Calculate padding
      size_t current_len = indent + header.length();
      if (current_len < description_col) {
        ss << std::string(description_col - current_len, ' ');
      } else {
        ss << " ";  // Ensure at least one space
      }
      ss << argument->get_description();
      if (argument->is_required()) {
        ss << " [required]";
      }
      // TODO: Add default value display
      ss << "\n";
    }
    ss << "\n";
  }

  // Print Commands
  if (!subcommands_.empty()) {
    // ... (command printing logic remains the same) ...
    ss << "Commands:\n";
    for (const auto& cmd : subcommands_) {
      ss << std::string(indent, ' ') << cmd->get_name();
      // Calculate padding
      size_t current_len = indent + cmd->get_name().length();
      if (current_len < description_col) {
        ss << std::string(description_col - current_len, ' ');
      } else {
        ss << " ";  // Ensure at least one space
      }
      ss << cmd->get_description() << "\n";
    }
    ss << "\n";
  }

  return ss.str();
}

int command_t::parse_and_execute(const std::vector<std::string>& args)
{
  std::vector<std::string> remaining_args;
  std::vector<std::string> parsing_errors;
  bool help_explicitly_requested = false;

  // --- Revised Lookup Tables ---
  std::unordered_map<std::string, parameter_t*> long_option_map;
  std::unordered_map<std::string, parameter_t*> short_option_map;
  std::vector<parameter_t*> positional_args_base;  // Store base pointers
  std::unique_ptr<command_t>* target_subcommand = nullptr;

  // Helper function to check if an argument string looks like a value rather
  // than an option
  auto is_value_arg = [](const std::string& s) -> bool
  {
    if (s.empty())
      return false;
    // It's a value if it doesn't start with '-'...
    if (s[0] != '-')
      return true;
    // ...unless it's just "-" (often used for stdin/stdout)
    if (s == "-")
      return true;
    // ...or if it starts with '-' but is followed by digits (negative number)
    if (s.length() > 1 && std::all_of(s.begin() + 1, s.end(), ::isdigit)) {
      return true;
    }
    // Otherwise (starts with '-' followed by non-digits), it's likely an option
    return false;
  };

  // --- 1. Prepare Parameter Lookup (Revised) ---
  for (auto& param : parameters_) {
    if (param->is_option()) {  // Use virtual function
      long_option_map["--" + param->get_name()] = param.get();
      std::string s_name = param->get_short_name();  // Use virtual function
      if (!s_name.empty()) {
        short_option_map["-" + s_name] = param.get();
      }
    } else if (param->is_argument()) {  // Use virtual function
      positional_args_base.push_back(param.get());
    }
  }

  // Debug logging for lookup tables
  //   LOG_TRACE_S << "Long options map:";
  //   for (const auto& pair : long_option_map) {
  //     LOG_TRACE_S << "  " << pair.first;
  //   }
  //   LOG_TRACE_S << "Short options map:";
  //   for (const auto& pair : short_option_map) {
  //     LOG_TRACE_S << "  " << pair.first;
  //   }

  // --- 2. Parse Arguments (Revised Logic) ---
  size_t positional_arg_index = 0;
  bool subcommand_found = false;
  bool double_dash_encountered = false;

  for (size_t i = 0; i < args.size(); ++i) {
    const std::string& arg = args[i];

    // --- Check for Help Flag FIRST (but only set flag, don't exit yet) ---
    if (arg == "--help" || arg == "-h") {
      parameter_t* help_param = nullptr;
      auto it_long_help = long_option_map.find("--help");
      if (it_long_help != long_option_map.end()) {
        help_param = it_long_help->second;
      } else {
        auto it_short_help = short_option_map.find("-h");
        if (it_short_help != short_option_map.end()) {
          help_param = it_short_help->second;
        }
      }
      // If help flag exists for *this* command, mark it as requested.
      if (help_param) {
        // Mark the parameter itself as set (important for flag logic)
        // We assume help flags don't take values, so parse("") is safe.
        help_param->parse("");
        help_explicitly_requested = true;  // Mark that help was asked for
        // Do NOT return yet, continue parsing other arguments.
        continue;  // Move to the next argument
      }
      // If --help or -h was provided but not registered for this command,
      // it will be caught later as an "Unknown option".
    }

    // Handle --
    if (!double_dash_encountered && arg == "--") {
      double_dash_encountered = true;
      continue;
    }

    parameter_t* current_param = nullptr;
    std::string current_opt_name = arg;
    std::string current_opt_value;
    bool value_provided_inline = false;
    bool is_long_option = false;
    bool is_short_option_or_bundle = false;

    // Determine if it's an option and its potential format
    if (!double_dash_encountered) {
      if (arg.rfind("--", 0) == 0) {
        is_long_option = true;
        // Handle --option=value
        size_t equals_pos = arg.find('=');
        if (equals_pos != std::string::npos) {
          current_opt_name = arg.substr(0, equals_pos);
          current_opt_value = arg.substr(equals_pos + 1);
          value_provided_inline = true;
        }
      } else if (arg.rfind("-", 0) == 0 && arg.length() > 1 && arg[1] != '-') {
        is_short_option_or_bundle = true;
        // Short option value attachment (-ovalue) is handled later
      }
    }

    // --- Try Parsing as Subcommand FIRST ---
    if (!double_dash_encountered && !subcommands_.empty()) {
      for (auto& sub_cmd_ptr : subcommands_) {
        if (sub_cmd_ptr->get_name() == arg) {
          target_subcommand = &sub_cmd_ptr;
          remaining_args.assign(args.begin() + i + 1, args.end());
          subcommand_found = true;
          break;
        }
      }
      if (subcommand_found)
        break;  // Break outer loop, proceed to dispatch
    }

    // --- Try Parsing as Option ---
    if (is_long_option) {
      auto it = long_option_map.find(current_opt_name);
      if (it != long_option_map.end()) {
        current_param = it->second;
        // Handle Long Option (Flag or Value)
        if (auto* flag = dynamic_cast<flag_t*>(current_param)) {
          if (value_provided_inline) {
            // Error: Flag with value
            parsing_errors.push_back("Flag '" + current_opt_name
                                     + "' does not take a value.");
            // Continue parsing next argument
          } else {
            // Correctly parse the flag (marks it as set)
            if (!flag->parse("")) {
              // Should not happen for flags, but handle defensively
              parsing_errors.push_back("Internal error parsing flag '"
                                       + current_opt_name + "'.");
            }
          }
        } else {  // Option needs a value
          bool value_found = false;
          if (value_provided_inline) {
            value_found = true;
          } else {
            // Check next argument using helper
            if (i + 1 < args.size() && is_value_arg(args[i + 1])) {
              bool next_is_subcommand = false;  // Check if next is subcommand
              // ... (subcommand check logic) ...
              if (!next_is_subcommand) {
                current_opt_value = args[i + 1];
                i++;
                value_found = true;
              }
            }
          }

          if (!value_found && !current_param->accepts_missing_value()) {
            // Error: Missing required value
            parsing_errors.push_back("Option '" + current_opt_name
                                     + "' requires a value.");
            // Continue parsing, but the option remains unset/invalid
          } else if (value_found) {
            try {  // Catch exceptions from custom parsers
              if (!current_param->parse(current_opt_value)) {
                // Error: Invalid value format (standard parser returned false)
                parsing_errors.push_back("Invalid value '" + current_opt_value
                                         + "' for option '" + current_opt_name
                                         + "'.");
                // Option remains unset/invalid
              }
            } catch (const ClickException& e) {
              parsing_errors.push_back(
                  e.what());  // Add error from custom parser exception
            }
            // Successfully parsed value or error recorded
          } else {
            // Optional value was missing, parse with empty string? Or specific
            // logic? Current base accepts_missing_value() is false, derived
            // might override. Let's assume parse needs to be called if it
            // accepts missing.
            if (current_param->accepts_missing_value()) {
              try {  // Catch exceptions from custom parsers
                if (!current_param->parse(""))
                {  // Or handle missing value case specifically
                  parsing_errors.push_back(
                      "Error parsing missing optional value for '"
                      + current_opt_name + "'.");
                }
              } catch (const ClickException& e) {
                parsing_errors.push_back(e.what());
              }
            }
            // If it doesn't accept missing, the error was already added above.
          }
          // else: Optional missing value handled or error recorded
        }
      } else {  // Unknown long option
        parsing_errors.push_back("Unknown option: " + current_opt_name);
      }
      continue;  // Move to the next argument
    } else if (is_short_option_or_bundle) {
      // Handle short option bundling (-abc) or single short option (-o)
      for (size_t j = 1; j < arg.length(); ++j) {
        std::string short_opt_name = "-" + std::string(1, arg[j]);
        auto it_short = short_option_map.find(short_opt_name);

        if (it_short != short_option_map.end()) {
          current_param = it_short->second;
          // Use is_flag() virtual function
          if (current_param->is_flag()) {
            // It's a flag, parse it.
            current_param->parse("");  // Call parse on base pointer is fine
                                       // If value is attached to a flag in a
                                       // bundle (-fvValue), it's an error
            if (j + 1 < arg.length()) {
              // Error: Value attached to flag in bundle
              parsing_errors.push_back("Flag '" + short_opt_name
                                       + "' does not take a value (in " + arg
                                       + ").");
              // Skip the rest of this bundle and move to next argument
              break;  // Break inner loop (j)
            }
          } else {  // Short option needs a value
            bool value_found_short = false;
            if (j + 1 < arg.length()) {  // Attached value: -ovalue
              current_opt_value = arg.substr(j + 1);
              value_found_short = true;
              j = arg.length();  // Consume rest of bundle as value
            } else {  // Value must be next argument
              // Check next argument using helper
              if (i + 1 < args.size() && is_value_arg(args[i + 1])) {
                bool next_is_subcommand = false;  // Check if next is subcommand
                // ... (subcommand check logic should be added here if needed)
                // ...
                if (!next_is_subcommand) {
                  current_opt_value = args[i + 1];
                  i++;
                  value_found_short = true;
                }
              }
            }

            if (!value_found_short && !current_param->accepts_missing_value()) {
              // Error: Missing required value
              parsing_errors.push_back("Option '" + short_opt_name
                                       + "' requires a value.");
              // Continue parsing bundle/next argument
            } else if (value_found_short) {
              try {  // Catch exceptions from custom parsers
                if (!current_param->parse(current_opt_value)) {
                  // Error: Invalid value format (standard parser returned
                  // false)
                  parsing_errors.push_back("Invalid value '" + current_opt_value
                                           + "' for option '" + short_opt_name
                                           + "'.");
                  // Option remains unset/invalid
                }
              } catch (const ClickException& e) {
                parsing_errors.push_back(
                    e.what());  // Add error from custom parser exception
              }
              // Successfully parsed value or error recorded
            } else {
              // Optional value missing
              if (current_param->accepts_missing_value()) {
                try {  // Catch exceptions from custom parsers
                  if (!current_param->parse(""))
                  {  // Or handle missing value case specifically
                    parsing_errors.push_back(
                        "Error parsing missing optional value for '"
                        + short_opt_name + "'.");
                  }
                } catch (const ClickException& e) {
                  parsing_errors.push_back(e.what());
                }
              }
              // Error already added if it was required
            }
            // If value was attached or consumed next arg, break inner loop
            if (value_found_short)
              break;  // Break inner loop (j)
          }
        } else {  // Unknown short option
          parsing_errors.push_back("Unknown option: '" + short_opt_name
                                   + "' in " + arg);
          // Continue checking rest of bundle? Or stop for this bundle?
          // Let's stop processing this bundle on first unknown short opt.
          break;  // Break inner loop (j)
        }
      }  // End loop through short options (j)
      continue;  // Move to the next argument (i)
    }

    // --- Treat as Positional Argument ---
    if (positional_arg_index < positional_args_base.size()) {
      current_param = positional_args_base[positional_arg_index];
      try {  // Catch exceptions from custom parsers for arguments
        if (!current_param->parse(arg)) {
          // Error: Invalid value for positional argument (standard parser
          // returned false)
          parsing_errors.push_back("Invalid value '" + arg + "' for argument '"
                                   + current_param->get_name() + "'.");
          // Argument remains unset/invalid
        }
      } catch (const ClickException& e) {
        parsing_errors.push_back(
            e.what());  // Add error from custom parser exception
      }
      positional_arg_index++;
    } else {
      // If a subcommand hasn't been found yet, this is unexpected.
      if (!target_subcommand) {
        // Error: Unexpected positional argument
        parsing_errors.push_back("Unexpected positional argument: " + arg
                                 + format_command_list());
        // Continue parsing next argument, though it's likely also an error
      } else {
        // If a subcommand *was* found earlier, this argument (and subsequent
        // ones) belong to it. Stop parsing for *this* command. The
        // remaining_args vector should have been populated correctly when the
        // subcommand was found.
        break;  // Break outer loop (i)
      }
    }
  }  // End main argument loop (i)

  // --- 4. Final Checks and Execution ---

  // Priority 1: Handle explicit help request
  if (help_explicitly_requested) {
    LOG_INFO_S << format_help();
    // Suppress parsing errors when help is explicitly requested.
    return 0;  // Successful exit after showing help
  }

  // Priority 2: Handle accumulated parsing errors
  if (!parsing_errors.empty()) {
    for (const auto& err : parsing_errors) {
      LOG_WARN_S << "Usage Error: " << err;  // Log accumulated errors
    }
    LOG_INFO_S << "\n" << format_help();  // Show help after errors
    return 1;  // Indicate error
  }

  // If we reach here, no help was requested and no parsing errors occurred.

  // --- 5. Execute Subcommand or Validate/Execute Current Command ---
  if (target_subcommand) {
    // Validation for required args of *this* command before dispatching
    // This validation should still run, as arguments before the subcommand are
    // parsed by this command.
    for (const auto* param : positional_args_base) {
      if (param->is_required() && !param->is_set()) {
        // Log error and return, don't throw
        LOG_ERROR_S << "Requirement Error: Argument '" << param->get_name()
                    << "' is required but not provided before subcommand '"
                    << (*target_subcommand)->get_name() << "'.";
        LOG_INFO_S << "\n" << format_help();  // Show help
        return 1;
        // throw UsageError(...) // Original
      }
    }
    // TODO: Consider handling required *options* of the main command as well?
    // Dispatch to subcommand. It will handle its own parsing, errors, help
    // requests.
    return (*target_subcommand)->parse_and_execute(remaining_args);
  } else {
    // No subcommand found or executed. Validate and execute current command.

    // --- 5a. Validate Required Parameters (Current Command) ---
    bool missing_required = false;
    for (const auto& param : parameters_) {
      if (param->is_required() && !param->is_set()) {
        missing_required = true;
        std::string param_id = (param->is_option() ? "Option" : "Argument");
        param_id += " '"
            + (param->is_option() ? ("--" + param->get_name())
                                  : param->get_name())
            + "'";
        LOG_ERROR_S << "Requirement Error: " << param_id
                    << " is required but not provided.";
        // Don't throw, collect all missing ones first.
      }
    }
    // This check might be redundant if positional_args_base covers all
    // arguments, but keep for safety, ensuring all required args were
    // processed. However, the loop above using parameters_ should cover
    // arguments too. Let's simplify. if (positional_arg_index <
    // positional_args_base.size()
    //     && positional_args_base[positional_arg_index]->is_required())
    // { ... }

    if (missing_required) {
      LOG_INFO_S << "\n"
                 << format_help();  // Show help if required params are missing
      return 1;  // Indicate error
    }

    // --- 5b. Execute Callback (Current Command) ---
    if (callback_) {
      // All checks passed, execute the command's action.
      return callback_();
    } else if (!subcommands_.empty()) {
      // No callback defined, but subcommands exist. This means a subcommand was
      // expected.
      LOG_WARN_S << "Usage Error: Missing command." << format_command_list();
      LOG_INFO_S << "\n" << format_help();
      return 1;  // Indicate error: command expected but not provided
      // throw UsageError("Missing command." + format_command_list()); //
      // Original
    } else {
      // No errors, no required params missing, no callback, no subcommands.
      // This is a successful invocation, possibly just parsing options.
      return 0;  // Success (e.g., app definition without action)
    }
  }
}

flag_t& command_t::add_flag(const std::string& name,
                            const std::string& short_name,
                            const std::string& description)
{
  // Add checks
  if (name.rfind("--", 0) == 0) {
    throw std::invalid_argument("Flag name should not start with '--': "
                                + name);
  }
  if (!short_name.empty() && short_name.rfind("-", 0) == 0) {
    throw std::invalid_argument("Flag short name should not start with '-': "
                                + short_name);
  }
  if (!short_name.empty() && short_name.length() > 1) {
    throw std::invalid_argument("Flag short name must be a single character: "
                                + short_name);
  }

  // Ensure help flag logic remains consistent if "help" is added manually
  if (name == "help" || short_name == "h") {
    help_flag_added_ = true;
  }
  auto flag = std::make_unique<flag_t>(name, short_name, description);
  flag_t* flag_ptr = flag.get();
  parameters_.push_back(std::move(flag));
  return *flag_ptr;
}

// Exception Implementations
ClickException::ClickException(const std::string& message)
    : std::runtime_error(message)
{
}

// Restore the definition, even if not directly called by handle_exceptions
void ClickException::print() const
{
  // Default implementation, might be useful for direct exception handling
  // or is needed for vtable generation.
  LOG_ERROR_S << "ClickException: " << what();
}

ParameterError::ParameterError(const std::string& message)
    : ClickException(message)
{
}

UsageError::UsageError(const std::string& message)
    : ClickException(message)
{
}

// CommandLineApp Implementation
CommandLineApp::CommandLineApp(const std::string& name,
                               const std::string& description)
    : command_t(name, description)
{
}

int CommandLineApp::run(int argc, char** argv)
{
  // Convert C-style args to vector<string>, skipping the program name (argv[0])
  std::vector<std::string> args(argv + 1, argv + argc);
  try {
    // Errors are now handled primarily by return codes and logs within
    // parse_and_execute
    return parse_and_execute(args);
  } catch (const ClickException& e) {
    // Catch exceptions that might still be thrown (e.g., invalid_argument
    // during setup, or if validation logic still throws instead of
    // logging/returning).
    handle_exceptions(e);  // Use existing handler for unexpected throws
    return 1;  // Indicate error
  } catch (const std::exception& e) {
    // Handle other potential standard exceptions during parsing/execution
    handle_exceptions(e);
    return 1;
  } catch (...) {
    LOG_CRITICAL_S << "An unknown error occurred.";  // Use logger
    return 1;
  }
}

void CommandLineApp::handle_exceptions(const std::exception& e) const
{
  // This handler is now less likely to be called for standard parsing errors,
  // but can catch other issues or explicitly thrown exceptions.
  if (const auto* usage_err = dynamic_cast<const UsageError*>(&e)) {
    LOG_WARN_S << "Usage Error: " << usage_err->what();
    LOG_INFO_S << "\n" << format_help();  // Show help on usage errors
  } else if (const auto* param_err = dynamic_cast<const ParameterError*>(&e)) {
    LOG_ERROR_S << "Parameter Error: " << param_err->what();
  } else if (const auto* click_err = dynamic_cast<const ClickException*>(&e)) {
    // Catch other ClickExceptions (if any)
    LOG_ERROR_S << "Error: " << click_err->what();
  } else {
    // Generic handling for other std::exception types
    LOG_ERROR_S << "Error: " << e.what();
  }
}

// Explicit template instantiations

// option_t
template class CPP_TOOLBOX_EXPORT option_t<std::string>;
template class CPP_TOOLBOX_EXPORT option_t<int>;
template class CPP_TOOLBOX_EXPORT option_t<double>;
template class CPP_TOOLBOX_EXPORT option_t<
    bool>;  // Though flag_t exists, generic bool option might be useful

// argument_t
template class CPP_TOOLBOX_EXPORT argument_t<std::string>;
template class CPP_TOOLBOX_EXPORT argument_t<int>;
template class CPP_TOOLBOX_EXPORT argument_t<double>;
// bool arguments are less common but possible
template class CPP_TOOLBOX_EXPORT argument_t<bool>;

// Optional instantiations
template class CPP_TOOLBOX_EXPORT option_t<std::optional<std::string>>;
template class CPP_TOOLBOX_EXPORT option_t<std::optional<int>>;
template class CPP_TOOLBOX_EXPORT option_t<std::optional<double>>;
template class CPP_TOOLBOX_EXPORT option_t<std::optional<bool>>;

template class CPP_TOOLBOX_EXPORT argument_t<std::optional<std::string>>;
template class CPP_TOOLBOX_EXPORT argument_t<std::optional<int>>;
template class CPP_TOOLBOX_EXPORT argument_t<std::optional<double>>;
template class CPP_TOOLBOX_EXPORT argument_t<std::optional<bool>>;

// Instantiation for custom parser test type
template class CPP_TOOLBOX_EXPORT
    option_t<std::pair<int, int>>;  // Added for custom pair parser

}  // namespace toolbox::utils
