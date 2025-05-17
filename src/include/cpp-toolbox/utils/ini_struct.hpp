#pragma once

#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>

#include <cpp-toolbox/logger/thread_logger.hpp>
#include <cpp-toolbox/utils/ini_config.hpp>

namespace toolbox::utils
{
namespace ini_detail
{
// Trait to detect std::optional
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

// Trait to detect if a type can be streamed from an istream
template<typename T, typename = void>
struct has_istream_operator : std::false_type
{
};

template<typename T>
struct has_istream_operator<
    T,
    std::void_t<decltype(std::declval<std::istream&>() >> std::declval<T&>())>>
    : std::true_type
{
};

template<typename T>
inline constexpr bool has_istream_operator_v =
    has_istream_operator<std::remove_cv_t<std::remove_reference_t<T>>>::value;

// Parsing helpers ---------------------------------------------------------

template<typename T>
bool parse_non_optional(const std::string& input, T& output)
{
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
        && (input[1] == 'x' || input[1] == 'X'))
    {
      ss >> std::hex >> output;
    } else {
      ss >> output;
    }
    return ss.eof() && !ss.fail();
  } else if constexpr (std::is_floating_point_v<T>) {
    ss >> output;
    return ss.eof() && !ss.fail();
  } else if constexpr (std::is_same_v<T, std::string>) {
    output = input;
    return true;
  } else {
    if constexpr (has_istream_operator_v<T>) {
      ss >> output;
      return ss.eof() && !ss.fail();
    } else {
      LOG_WARN_S << "Unsupported type for INI parser: " << typeid(T).name();
      return false;
    }
  }
}

template<typename T_Optional>
bool parse_non_optional(const std::string& input, std::optional<T_Optional>& output)
{
  T_Optional value;
  if (parse_non_optional(input, value)) {
    output = value;
    return true;
  }
  output = std::nullopt;
  return false;
}

template<typename T>
bool parse_value(const std::string& input, T& output)
{
  if constexpr (is_optional_v<T>) {
    if (input.empty()) {
      output = std::nullopt;
      return true;
    }
    using value_type = optional_value_type_t<T>;
    value_type parsed;
    bool ok = parse_non_optional(input, parsed);
    if (ok)
      output = parsed;
    else
      output = std::nullopt;
    return ok;
  } else {
    return parse_non_optional(input, output);
  }
}

}  // namespace ini_detail

// Field descriptor -------------------------------------------------------

template<typename Struct, typename Member>
struct ini_field_desc
{
  const char* section;
  const char* key;
  Member Struct::*member;
};

// Trait to associate a struct with field descriptors
template<typename T>
struct ini_struct_traits
{
  static constexpr auto fields = std::make_tuple();
};

// Load struct from ini_config_t -----------------------------------------

template<typename Struct>
bool load_struct_from_ini(const ini_config_t& cfg,
                          Struct& obj,
                          const std::string& base_section = "")
{
  bool success = true;
  constexpr auto fields = ini_struct_traits<Struct>::fields;
  std::apply(
      [&](auto&&... field)
      {
        ([&]() {
          using member_t = std::remove_reference_t<decltype(obj.*(field.member))>;
          std::string section = base_section;
          if (field.section && field.section[0] != '\0') {
            if (!base_section.empty()) {
              section += ".";
              section += field.section;
            } else {
              section = field.section;
            }
          }

          if (cfg.has(section, field.key)) {
            member_t value{};
            if (ini_detail::parse_value(cfg.get_string(section, field.key),
                                       value))
            {
              obj.*(field.member) = value;
            } else {
              success = false;
            }
          }
        }(), ...);
      },
      fields);
  return success;
}

}  // namespace toolbox::utils

// Helper macros to declare reflection metadata -------------------------

#define TOOLBOX_INI_FIELD(struct_type, member, section, key) \
  toolbox::utils::ini_field_desc<struct_type, decltype(struct_type::member)>{ \
      section, key, &struct_type::member }

#define TOOLBOX_INI_STRUCT(struct_type, ...) \
  template<> struct toolbox::utils::ini_struct_traits<struct_type> { \
    static constexpr auto fields = std::make_tuple(__VA_ARGS__); \
  };

