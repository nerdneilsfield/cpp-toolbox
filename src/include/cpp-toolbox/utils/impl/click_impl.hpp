#pragma once

#include <algorithm>
#include <optional>  // Add optional include
#include <sstream>  // For std::stringstream
#include <stdexcept>  // For std::invalid_argument, std::runtime_error
#include <string>
#include <type_traits>  // For std::is_integral, std::is_floating_point

#include <cpp-toolbox/logger/thread_logger.hpp>
#include <cpp-toolbox/utils/click.hpp>

namespace toolbox::utils::impl
{

}  // namespace toolbox::utils::impl

namespace toolbox::utils
{
namespace detail
{
// Type trait to check if T is std::optional<U>
template<typename T>
struct is_optional : std::false_type
{
};
template<typename U>
struct is_optional<std::optional<U>> : std::true_type
{
};
template<typename T>
inline constexpr bool is_optional_v =
    is_optional<std::remove_cv_t<std::remove_reference_t<T>>>::value;

// Helper to get the value_type of std::optional
template<typename T>
struct optional_value_type
{
};
template<typename U>
struct optional_value_type<std::optional<U>>
{
  using type = U;
};
template<typename T>
using optional_value_type_t = typename optional_value_type<
    std::remove_cv_t<std::remove_reference_t<T>>>::type;

// Type trait to check if a type has stream extraction operator (>>)
template<typename T, typename = void>
struct has_istream_operator : std::false_type
{
};

template<typename T>
struct has_istream_operator<
    T,
    std::void_t<decltype(std::declval<std::istream&>() >> std::declval<T&>())>>
    : std::is_base_of<
          std::istream,
          std::remove_reference_t<decltype(std::declval<std::istream&>()
                                           >> std::declval<T&>())>>
{
};
// Checks if the result of operator>> is actually a stream reference

template<typename T>
inline constexpr bool has_istream_operator_v =
    has_istream_operator<std::remove_cv_t<std::remove_reference_t<T>>>::value;

}  // namespace detail

// option_t Implementation
template<typename T>
option_t<T>::option_t(const std::string& name,
                      const std::string& short_name,
                      const std::string& description,
                      bool required)
    : parameter_t(
        name, description, detail::is_optional_v<T> ? false : required)
    , short_name_(short_name)
{
  set_default_parser();  // Setup default parser based on type T
  if constexpr (detail::is_optional_v<T>) {
    value_ = std::nullopt;
    default_value_ = std::nullopt;
  }
}

template<typename T>
option_t<T>& option_t<T>::set_default(const T& default_value)
{
  default_value_ = default_value;
  value_ = default_value;  // Initialize value with default
  return *this;
}

template<typename T>
option_t<T>& option_t<T>::set_parser(
    std::function<bool(const std::string&, T&)> parser)
{
  parser_ = std::move(parser);
  return *this;
}

template<typename T>
[[nodiscard]] std::string option_t<T>::get_short_name() const
{
  return short_name_;
}

template<typename T>
T option_t<T>::get() const
{
  if constexpr (detail::is_optional_v<T>) {
    return value_;
  } else {
    return is_set_ ? value_ : default_value_;
  }
}

template<typename T>
bool option_t<T>::parse(const std::string& value)
{
  if (!parser_) {
    LOG_CRITICAL_S << "Internal Error: Parser function not set for option '"
                   << name_ << "'.";
    throw std::runtime_error("Parser function not set for option: " + name_);
  }

  if constexpr (detail::is_optional_v<T>) {
    is_set_ = true;
    if (value.empty()) {
      value_ = std::nullopt;
      return true;
    }
    return parser_(value, value_);
  } else {
    T parsed_value;
    if (parser_(value, parsed_value)) {
      value_ = parsed_value;
      is_set_ = true;
      return true;
    } else {
      return false;
    }
  }
}

template<typename T>
void option_t<T>::set_default_parser()
{
  parser_ = [](const std::string& input, T& output) -> bool
  {
    if constexpr (detail::is_optional_v<T>) {
      if (input.empty()) {
        output = std::nullopt;
        return true;
      }

      using optional_value_type = detail::optional_value_type_t<T>;
      optional_value_type parsed_value;
      std::stringstream ss(input);

      bool parse_success = false;
      if constexpr (std::is_same_v<optional_value_type, bool>) {
        std::string lower_input;
        std::transform(input.begin(),
                       input.end(),
                       std::back_inserter(lower_input),
                       ::tolower);
        if (lower_input == "true" || lower_input == "1") {
          parsed_value = true;
          parse_success = true;
        } else if (lower_input == "false" || lower_input == "0") {
          parsed_value = false;
          parse_success = true;
        } else {
          parse_success = false;
        }
      } else if constexpr (std::is_integral_v<optional_value_type>) {
        if (input.size() > 2 && input[0] == '0'
            && (input[1] == 'x' || input[1] == 'X')) {
          ss >> std::hex >> parsed_value;
        } else {
          ss >> parsed_value;
        }
        parse_success = (ss.eof() && !ss.fail());
      } else if constexpr (std::is_floating_point_v<optional_value_type>) {
        ss >> parsed_value;
        parse_success = (ss.eof() && !ss.fail());
      } else if constexpr (std::is_same_v<optional_value_type, std::string>) {
        parsed_value = input;
        parse_success = true;
      } else {
        if constexpr (detail::has_istream_operator_v<optional_value_type>) {
          ss >> parsed_value;
          parse_success = (ss.eof() && !ss.fail());
        } else {
          parse_success = false;
        }
      }

      if (parse_success) {
        output = parsed_value;
      } else {
        output = std::nullopt;
      }

      return true;

    } else {
      std::stringstream ss(input);
      if constexpr (std::is_same_v<T, bool>) {
        std::string lower_input;
        std::transform(input.begin(),
                       input.end(),
                       std::back_inserter(lower_input),
                       ::tolower);
        if (lower_input == "true" || lower_input == "1") {
          output = true;
          return true;
        } else if (lower_input == "false" || lower_input == "0") {
          output = false;
          return true;
        } else {
          return false;
        }
      } else if constexpr (std::is_integral_v<T>) {
        if (input.size() > 2 && input[0] == '0'
            && (input[1] == 'x' || input[1] == 'X')) {
          ss >> std::hex >> output;
        } else {
          ss >> output;
        }
      } else if constexpr (std::is_floating_point_v<T>) {
        ss >> output;
      } else if constexpr (std::is_same_v<T, std::string>) {
        output = input;
        return true;
      } else {
        if constexpr (detail::has_istream_operator_v<T>) {
          ss >> output;
        } else {
          return false;
        }
      }
      if constexpr (detail::has_istream_operator_v<T>) {
        return ss.eof() && !ss.fail();
      } else {
        return false;
      }
    }
  };
}

template<typename T>
bool option_t<T>::accepts_missing_value() const
{
  return detail::is_optional_v<T>;
}

template<typename T>
bool option_t<T>::is_option() const
{
  return true;
}

// argument_t Implementation
template<typename T>
argument_t<T>::argument_t(const std::string& name,
                          const std::string& description,
                          bool required)
    : parameter_t(
        name, description, detail::is_optional_v<T> ? false : required)
{
  set_default_parser();  // Setup default parser based on type T
  if constexpr (detail::is_optional_v<T>) {
    value_ = std::nullopt;
    default_value_ = std::nullopt;
  }
}

template<typename T>
argument_t<T>& argument_t<T>::set_default(const T& default_value)
{
  default_value_ = default_value;
  value_ = default_value;  // Initialize value with default
  return *this;
}

template<typename T>
T argument_t<T>::get() const
{
  if constexpr (detail::is_optional_v<T>) {
    return value_;
  } else {
    return is_set_ ? value_ : default_value_;
  }
}

template<typename T>
bool argument_t<T>::parse(const std::string& value)
{
  if (!parser_) {
    LOG_CRITICAL_S << "Internal Error: Parser function not set for argument '"
                   << name_ << "'.";
    throw std::runtime_error("Parser function not set for argument: " + name_);
  }

  if constexpr (detail::is_optional_v<T>) {
    is_set_ = true;
    bool underlying_parse_success = parser_(value, value_);
    if (!underlying_parse_success) {
      value_ = std::nullopt;
    }
    return underlying_parse_success;
  } else {
    T parsed_value;
    if (parser_(value, parsed_value)) {
      value_ = parsed_value;
      is_set_ = true;
      return true;
    } else {
      return false;
    }
  }
}

template<typename T>
void argument_t<T>::set_default_parser()
{
  parser_ = [](const std::string& input, T& output) -> bool
  {
    if constexpr (detail::is_optional_v<T>) {
      using optional_value_type = detail::optional_value_type_t<T>;
      optional_value_type parsed_value;
      std::stringstream ss(input);
      bool parse_success = false;

      if constexpr (std::is_same_v<optional_value_type, bool>) {
        std::string lower_input;
        std::transform(input.begin(),
                       input.end(),
                       std::back_inserter(lower_input),
                       ::tolower);
        if (lower_input == "true" || lower_input == "1") {
          parsed_value = true;
          parse_success = true;
        } else if (lower_input == "false" || lower_input == "0") {
          parsed_value = false;
          parse_success = true;
        } else {
          parse_success = false;
        }
      } else if constexpr (std::is_integral_v<optional_value_type>) {
        if (input.size() > 2 && input[0] == '0'
            && (input[1] == 'x' || input[1] == 'X')) {
          ss >> std::hex >> parsed_value;
        } else {
          ss >> parsed_value;
        }
        parse_success = (ss.eof() && !ss.fail());
      } else if constexpr (std::is_floating_point_v<optional_value_type>) {
        ss >> parsed_value;
        parse_success = (ss.eof() && !ss.fail());
      } else if constexpr (std::is_same_v<optional_value_type, std::string>) {
        parsed_value = input;
        parse_success = true;
      } else {
        if constexpr (detail::has_istream_operator_v<optional_value_type>) {
          ss >> parsed_value;
          parse_success = (ss.eof() && !ss.fail());
        } else {
          parse_success = false;
        }
      }

      if (parse_success) {
        output = parsed_value;
      } else {
        output = std::nullopt;
      }
      return true;

    } else {
      std::stringstream ss(input);
      if constexpr (std::is_same_v<T, bool>) {
        std::string lower_input;
        std::transform(input.begin(),
                       input.end(),
                       std::back_inserter(lower_input),
                       ::tolower);
        if (lower_input == "true" || lower_input == "1") {
          output = true;
          return true;
        } else if (lower_input == "false" || lower_input == "0") {
          output = false;
          return true;
        } else {
          return false;
        }
      } else if constexpr (std::is_integral_v<T>) {
        if (input.size() > 2 && input[0] == '0'
            && (input[1] == 'x' || input[1] == 'X')) {
          ss >> std::hex >> output;
        } else {
          ss >> output;
        }
      } else if constexpr (std::is_floating_point_v<T>) {
        ss >> output;
      } else if constexpr (std::is_same_v<T, std::string>) {
        output = input;
        return true;
      } else {
        if constexpr (detail::has_istream_operator_v<T>) {
          ss >> output;
        } else {
          return false;
        }
      }
      if constexpr (detail::has_istream_operator_v<T>) {
        return ss.eof() && !ss.fail();
      } else {
        return false;
      }
    }
  };
}

template<typename T>
bool argument_t<T>::accepts_missing_value() const
{
  return false;
}

template<typename T>
bool argument_t<T>::is_argument() const
{
  return true;
}

template<typename T>
std::string argument_t<T>::get_short_name() const
{
  return "";
}

// command_t Template Method Implementations
template<typename T>
option_t<T>& command_t::add_option(const std::string& name,
                                   const std::string& short_name,
                                   const std::string& description,
                                   bool required /* = false */)
{
  if (name.rfind("--", 0) == 0) {
    throw std::invalid_argument("Option name should not start with '--': "
                                + name);
  }
  if (!short_name.empty() && short_name.rfind("-", 0) == 0) {
    throw std::invalid_argument("Option short name should not start with '-': "
                                + short_name);
  }
  if (!short_name.empty() && short_name.length() > 1) {
    throw std::invalid_argument("Option short name must be a single character: "
                                + short_name);
  }

  auto option =
      std::make_unique<option_t<T>>(name, short_name, description, required);
  option_t<T>* option_ptr = option.get();
  parameters_.push_back(std::move(option));
  return *option_ptr;
}

template<typename T>
argument_t<T>& command_t::add_argument(const std::string& name,
                                       const std::string& description,
                                       bool required)
{
  auto argument = std::make_unique<argument_t<T>>(name, description, required);
  argument_t<T>* argument_ptr = argument.get();
  parameters_.push_back(std::move(argument));
  return *argument_ptr;
}

}  // namespace toolbox::utils
