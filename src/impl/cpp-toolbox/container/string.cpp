#include <algorithm>  // For trim, contains, etc.
#include <array>  // For array
#include <cctype>  // For isspace
#include <charconv>  // For std::from_chars (C++17)
#include <iterator>  // For join
#include <limits>  // For numeric_limits
#include <numeric>  // For join with reservation
#include <sstream>  // For join
#include <string>
#include <string_view>
#include <system_error>  // For std::errc
#include <vector>

#include "cpp-toolbox/container/string.hpp"

#include "cpp-toolbox/file/file.hpp"
#include "cpp-toolbox/macro.hpp"

namespace toolbox::container::string
{

// Implementation for splitting by string delimiter
auto split(std::string_view str, std::string_view delimiter)
    -> std::vector<std::string>
{
  std::vector<std::string> result;
  if (delimiter.empty()) {
    // If delimiter is empty, split into individual characters
    result.reserve(str.length());
    for (char c : str) {
      result.emplace_back(1, c);
    }
    return result;
  }

  std::string_view::size_type start = 0;
  std::string_view::size_type end = str.find(delimiter);

  while (end != std::string_view::npos) {
    result.emplace_back(str.substr(start, end - start));
    start = end + delimiter.length();
    end = str.find(delimiter, start);
  }

  // Add the last part
  result.emplace_back(str.substr(start));
  return result;
}

// Implementation for splitting by char delimiter
auto split(std::string_view str, char delimiter) -> std::vector<std::string>
{
  std::vector<std::string> result;
  std::string_view::size_type start = 0;
  std::string_view::size_type end = str.find(delimiter);

  while (end != std::string_view::npos) {
    result.emplace_back(str.substr(start, end - start));
    start = end + 1;  // Move past the single character delimiter
    end = str.find(delimiter, start);
  }

  // Add the last part
  result.emplace_back(str.substr(start));
  return result;
}

// Implementation for joining vector of strings
auto join(const std::vector<std::string>& parts, std::string_view glue)
    -> std::string
{
  if (parts.empty()) {
    return "";
  }

  // Pre-calculate the final string size for efficiency
  std::size_t total_size = (parts.size() - 1) * glue.length();
  for (const auto& part : parts) {
    total_size += part.length();
  }

  std::string result;
  result.reserve(total_size);

  result += parts[0];
  for (std::size_t i = 1; i < parts.size(); ++i) {
    result += glue;
    result += parts[i];
  }

  return result;
}

// Implementation for joining vector of string_views
auto join(const std::vector<std::string_view>& parts, std::string_view glue)
    -> std::string
{
  if (parts.empty()) {
    return "";
  }

  // Pre-calculate the final string size for efficiency
  std::size_t total_size = (parts.size() - 1) * glue.length();
  // Use std::accumulate to sum up lengths
  total_size = std::accumulate(parts.begin(),
                               parts.end(),
                               total_size,
                               [](std::size_t current_sum, std::string_view sv)
                               { return current_sum + sv.length(); });

  std::string result;
  result.reserve(total_size);

  result.append(parts[0]);  // Use append for string_view
  for (std::size_t i = 1; i < parts.size(); ++i) {
    result.append(glue);
    result.append(parts[i]);
  }

  return result;
}

// Implementation for trimming leading whitespace
auto trim_left(std::string_view str) -> std::string
{
  auto it = std::find_if_not(str.begin(), str.end(), ::isspace);
  return std::string(it, str.end());
}

// Implementation for trimming trailing whitespace
auto trim_right(std::string_view str) -> std::string
{
  // Use reverse iterators to find the first non-whitespace from the end
  auto it = std::find_if_not(str.rbegin(), str.rend(), ::isspace);
  // Calculate the distance from the beginning to the last non-whitespace
  // character it.base() points one element *past* the element found by
  // find_if_not in the original sequence
  return std::string(str.begin(), it.base());
}

// Implementation for trimming both leading and trailing whitespace
auto trim(std::string_view str) -> std::string
{
  auto start_it = std::find_if_not(str.begin(), str.end(), ::isspace);
  // If the string is all whitespace, start_it will be str.end()
  if (start_it == str.end()) {
    return "";
  }
  auto end_it = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
  return std::string(start_it, end_it);
}

// Implementation for checking prefix
auto starts_with(std::string_view s, std::string_view prefix) -> bool
{
  // C++20 has std::string_view::starts_with, but for compatibility:
  return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

// Implementation for checking suffix
auto ends_with(std::string_view s, std::string_view suffix) -> bool
{
  // C++20 has std::string_view::ends_with, but for compatibility:
  return s.size() >= suffix.size()
      && s.substr(s.size() - suffix.size()) == suffix;
}

// Implementation for checking substring containment
auto contains(std::string_view s, std::string_view substring) -> bool
{
  return s.find(substring) != std::string_view::npos;
}

// Implementation for checking if string is empty or only whitespace
auto is_empty_or_whitespace(std::string_view s) -> bool
{
  return std::all_of(s.begin(), s.end(), ::isspace);
}

// Implementation for checking if string represents an integer
auto is_integer(std::string_view s) -> bool
{
  if (s.empty()) {
    return false;
  }

  std::size_t pos = 0;
  // Check for optional sign
  if (s[0] == '+' || s[0] == '-') {
    pos = 1;
  }

  // If string is only a sign
  if (pos == s.length()) {
    return false;
  }

  // Check remaining characters are digits
  for (std::size_t i = pos; i < s.length(); ++i) {
    if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
      return false;
    }
  }

  return true;
}

// Implementation for checking if string represents a numeric value (int or
// float) Supports optional sign, decimal point, and scientific notation (e/E).
// Assumes no leading/trailing whitespace.
auto is_numeric(std::string_view s) -> bool
{
  if (s.empty()) {
    return false;
  }

  std::size_t pos = 0;
  bool has_sign = false;
  bool has_decimal_point = false;
  bool has_exponent = false;
  bool has_digit = false;

  // Check for optional sign
  if (s[0] == '+' || s[0] == '-') {
    has_sign = true;
    pos = 1;
  }

  // Need at least one character after sign if present
  if (has_sign && pos == s.length()) {
    return false;
  }

  // Check significand part (before E/e)
  while (pos < s.length()) {
    char c = s[pos];
    if (std::isdigit(static_cast<unsigned char>(c))) {
      has_digit = true;
      pos++;
    } else if (c == '.') {
      if (has_decimal_point || has_exponent) {
        return false;  // Already saw a decimal or exponent
      }
      has_decimal_point = true;
      pos++;
    } else if ((c == 'e' || c == 'E') && has_digit)
    {  // Exponent needs digits before it
      has_exponent = true;
      pos++;
      break;  // Move to exponent part check
    } else {
      return false;  // Invalid character in significand
    }
  }

  // If string ends after the significand part, it's valid if it had digits
  if (!has_exponent) {
    return has_digit;
  }

  // --- Check exponent part ---

  // Need characters after 'e'/'E'
  if (pos == s.length()) {
    return false;
  }

  // Check for optional sign in exponent
  if (s[pos] == '+' || s[pos] == '-') {
    pos++;
    // Need digits after exponent sign
    if (pos == s.length()) {
      return false;
    }
  }

  bool has_exponent_digit = false;
  while (pos < s.length()) {
    if (std::isdigit(static_cast<unsigned char>(s[pos]))) {
      has_exponent_digit = true;
      pos++;
    } else {
      return false;  // Invalid character in exponent
    }
  }

  // Valid only if exponent part had at least one digit
  return has_exponent_digit;
}

// Implementation for checking if string represents a floating-point value.
// Similar to is_numeric but requires either a decimal point or an exponent.
// Assumes no leading/trailing whitespace.
auto is_float(std::string_view s) -> bool
{
  if (s.empty()) {
    return false;
  }

  std::size_t pos = 0;
  bool has_sign = false;
  bool has_decimal_point = false;
  bool has_exponent = false;
  bool has_digit = false;

  // Check for optional sign
  if (s[0] == '+' || s[0] == '-') {
    has_sign = true;
    pos = 1;
  }

  // Need at least one character after sign if present
  if (has_sign && pos == s.length()) {
    return false;
  }

  // Check significand part
  while (pos < s.length()) {
    char c = s[pos];
    if (std::isdigit(static_cast<unsigned char>(c))) {
      has_digit = true;
      pos++;
    } else if (c == '.') {
      if (has_decimal_point || has_exponent) {
        return false;  // Already saw a decimal or exponent
      }
      has_decimal_point = true;
      pos++;
    } else if ((c == 'e' || c == 'E') && has_digit)
    {  // Exponent needs digits before it
      has_exponent = true;
      pos++;
      break;  // Move to exponent part check
    } else {
      return false;  // Invalid character in significand
    }
  }

  // If no exponent, must have had a decimal point and at least one digit
  if (!has_exponent) {
    return has_digit && has_decimal_point;
  }

  // --- Check exponent part ---

  // Need characters after 'e'/'E'
  if (pos == s.length()) {
    return false;
  }

  // Check for optional sign in exponent
  if (s[pos] == '+' || s[pos] == '-') {
    pos++;
    // Need digits after exponent sign
    if (pos == s.length()) {
      return false;
    }
  }

  bool has_exponent_digit = false;
  while (pos < s.length()) {
    if (std::isdigit(static_cast<unsigned char>(s[pos]))) {
      has_exponent_digit = true;
      pos++;
    } else {
      return false;  // Invalid character in exponent
    }
  }

  // Valid float if exponent part had digits (and significand had digits)
  return has_exponent_digit;
}

// Implementation for replacing substrings
auto replace(std::string_view s,
             std::string_view old_value,
             std::string_view new_value,
             std::size_t count) -> std::string
{
  std::string result;
  if (old_value.empty()) {
    // If old_value is empty, insert new_value between characters and at the
    // start (up to count times)
    result.reserve(s.length()
                   + std::min(count, s.length() + 1) * new_value.length());
    std::size_t insertions_done = 0;
    if (insertions_done < count) {
      result.append(new_value);
      insertions_done++;
    }
    for (std::size_t i = 0; i < s.length(); ++i) {
      result += s[i];
      if (insertions_done < count) {
        result.append(new_value);
        insertions_done++;
      }
    }
    return result;
  }

  std::string::size_type start_pos = 0;
  std::string::size_type pos;
  std::size_t replacements_done = 0;

  // Estimate final size to reserve memory, avoids multiple reallocations
  std::ptrdiff_t size_diff_per_replacement =
      static_cast<std::ptrdiff_t>(new_value.length())
      - static_cast<std::ptrdiff_t>(old_value.length());
  std::size_t estimated_size = s.length();
  if (size_diff_per_replacement > 0) {
    estimated_size += count * size_diff_per_replacement;
  }
  result.reserve(estimated_size);

  while (replacements_done < count
         && (pos = s.find(old_value, start_pos)) != std::string_view::npos)
  {
    result.append(s.data() + start_pos, pos - start_pos);
    result.append(new_value);
    start_pos = pos + old_value.length();
    replacements_done++;
  }

  result.append(s.data() + start_pos, s.length() - start_pos);
  return result;
}

// Implementation for replacing all occurrences
auto replace_all(std::string_view s,
                 std::string_view old_value,
                 std::string_view new_value) -> std::string
{
  // Call the main replace function with the maximum possible count
  return replace(
      s, old_value, new_value, std::numeric_limits<std::size_t>::max());
}

// Implementation for replacing the Nth occurrence
auto replace_by_nth(std::string_view s,
                    std::string_view old_value,
                    std::string_view new_value,
                    std::size_t n) -> std::string
{
  if (n == 0 || old_value.empty()) {
    return std::string(
        s);  // Invalid index (must be 1-based) or empty 'old_value'
  }

  std::string result;
  std::string::size_type start_pos = 0;
  std::string::size_type pos = std::string_view::npos;
  std::size_t current_occurrence = 0;

  while (start_pos < s.length()) {
    pos = s.find(old_value, start_pos);
    if (pos == std::string_view::npos) {
      break;  // No more occurrences
    }

    current_occurrence++;
    if (current_occurrence == n) {
      // Found the Nth occurrence, perform replacement and construct the result
      result.reserve(s.length() - old_value.length()
                     + new_value.length());  // Estimate size
      result.append(s.data(), pos);  // Append part before Nth occurrence
      result.append(new_value);  // Append the replacement
      // Append the part after the Nth occurrence
      result.append(s.data() + pos + old_value.length(),
                    s.length() - (pos + old_value.length()));
      return result;
    }

    // Not the Nth occurrence yet, move start position past the current match
    start_pos = pos + old_value.length();
  }

  // Nth occurrence not found or loop finished before finding it
  return std::string(s);
}

// Implementation for removing the Nth occurrence
auto remove_nth(std::string_view s, std::string_view from, std::size_t n)
    -> std::string
{
  // Removing is replacing with an empty string
  return replace_by_nth(s, from, "", n);
}

// Implementation for removing substrings up to a count
auto remove(std::string_view s, std::string_view value, std::size_t count)
    -> std::string
{
  if (value.empty()) {
    return std::string(s);
  }
  std::string result;
  std::string::size_type start_pos = 0;
  std::string::size_type pos;
  std::size_t removals_done = 0;

  result.reserve(s.length());  // Reserve at most original length

  while (removals_done < count
         && (pos = s.find(value, start_pos)) != std::string_view::npos)
  {
    result.append(s.data() + start_pos, pos - start_pos);
    start_pos = pos + value.length();
    removals_done++;
  }

  result.append(s.data() + start_pos, s.length() - start_pos);
  return result;
}

// Implementation for removing all occurrences of a substring
auto remove_all(std::string_view s, std::string_view value) -> std::string
{
  // Removing is replacing with an empty string
  return replace(s, value, "", std::numeric_limits<std::size_t>::max());
}

// Implementation for removing all occurrences of a character
auto remove_all(std::string_view s, char value) -> std::string
{
  std::string result;
  result.reserve(s.length());  // Reserve capacity, at most original length
  for (char c : s) {
    if (c != value) {
      result += c;
    }
  }
  return result;
}

// Implementation for converting to lowercase
auto to_lower(std::string_view s) -> std::string
{
  std::string result(s);
  std::transform(result.begin(),
                 result.end(),
                 result.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}

// Implementation for converting to uppercase
auto to_upper(std::string_view s) -> std::string
{
  std::string result(s);
  std::transform(result.begin(),
                 result.end(),
                 result.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return result;
}

// Implementation for converting string_view to string
auto to_string(std::string_view s) -> std::string
{
  return std::string(s);
}

// Implementation for left padding
auto left_pad(std::string_view s, std::size_t width, char pad_char)
    -> std::string
{
  if (s.length() >= width) {
    return std::string(s);
  }
  std::string result;
  result.reserve(width);
  result.append(width - s.length(), pad_char);  // Add padding characters
  result.append(s);  // Add original string
  return result;
}

// Implementation for right padding
auto right_pad(std::string_view s, std::size_t width, char pad_char)
    -> std::string
{
  if (s.length() >= width) {
    return std::string(s);
  }
  std::string result;
  result.reserve(width);
  result.append(s);  // Add original string
  result.append(width - s.length(), pad_char);  // Add padding characters
  return result;
}

// Implementation for general padding (defaults to left padding)
// Note: Position parameter implementation here is basic (0=left, otherwise
// right). Could be extended for center padding.
auto pad(std::string_view s,
         std::size_t width,
         char pad_char,
         std::size_t position) -> std::string
{
  if (position == 0) {  // 0 typically means left padding
    return left_pad(s, width, pad_char);
  } else {  // Any other value means right padding in this basic implementation
    return right_pad(s, width, pad_char);
  }
  // Future extension: handle center padding if needed
}

// Implementation for reversing a string
auto reverse(std::string_view s) -> std::string
{
  std::string result(s);
  std::reverse(result.begin(), result.end());
  return result;
}

// Implementation for trying to parse an integer
auto try_parse_int(std::string_view s, int& out) -> bool
{
  if (s.empty()) {
    return false;
  }
  const char* first = s.data();
  const char* last = s.data() + s.size();
  auto result = std::from_chars(first, last, out);
  return result.ec == std::errc() && result.ptr == last;
}

// Implementation for trying to parse a double
auto try_parse_double(std::string_view s, double& out) -> bool
{
  if (s.empty()) {
    return false;
  }
  const char* first = s.data();
  const char* last = s.data() + s.size();
  auto result = std::from_chars(first, last, out);
  return result.ec == std::errc() && result.ptr == last;
}

// Implementation for trying to parse a float
auto try_parse_float(std::string_view s, float& out) -> bool
{
  if (s.empty()) {
    return false;
  }
  const char* first = s.data();
  const char* last = s.data() + s.size();
  auto result = std::from_chars(first, last, out);
  return result.ec == std::errc() && result.ptr == last;
}

auto levenshtein_distance(std::string_view s1, std::string_view s2)
    -> std::size_t
{
  const std::size_t len1 = s1.length();
  const std::size_t len2 = s2.length();

  // Handle empty string edge cases
  if (len1 == 0)
    return len2;
  if (len2 == 0)
    return len1;

  // Use a single dynamic programming vector (one row) for space optimization
  // O(N), where N is the length of the shorter string. Typically, the dp array
  // size is based on the second string's length + 1.
  std::vector<std::size_t> dp(len2 + 1);

  // Initialize the first row: distance from empty string to prefix of s2 is j
  for (std::size_t j = 0; j <= len2; ++j) {
    dp[j] = j;
  }

  // Iterate through each character of s1
  for (std::size_t i = 1; i <= len1; ++i) {
    // Store the value of dp[i-1][j-1] (top-left value)
    std::size_t previous_top_left = dp[0];
    // Calculate dp[i][0]: distance from prefix of s1 to empty string is i
    dp[0] = i;

    // Iterate through each character of s2
    for (std::size_t j = 1; j <= len2; ++j) {
      // Store the value of dp[i-1][j] (value directly above)
      std::size_t current_top = dp[j];

      // Calculate substitution/match cost
      const std::size_t cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;

      // Calculate dp[i][j]
      // dp[j-1] is the value to the left: dp[i][j-1]
      dp[j] = std::min({
          current_top + 1,  // From above (deletion from s1)
          dp[j - 1] + 1,  // From left (insertion into s1)
          previous_top_left + cost  // From top-left (substitution/match)
      });

      // Update previous_top_left for the next iteration of the inner loop
      previous_top_left = current_top;
    }
  }

  // The final result is stored in the last element of the dp vector
  return dp[len2];
}

// Longest Common Subsequence (LCS) Length - Space Optimized O(min(N, M))
auto longest_common_subsequence_length(std::string_view s1, std::string_view s2)
    -> std::size_t
{
  const std::size_t len1 = s1.length();
  const std::size_t len2 = s2.length();

  if (len1 == 0 || len2 == 0) {
    return 0;
  }

  // Ensure dp vector corresponds to the shorter string length for better
  // optimization
  if (len1 < len2) {
    return longest_common_subsequence_length(
        s2, s1);  // Swap to ensure s2 is shorter
  }

  // Use two rows for DP table space optimization
  std::vector<std::size_t> dp_prev(len2 + 1, 0);
  std::vector<std::size_t> dp_curr(len2 + 1, 0);

  for (std::size_t i = 1; i <= len1; ++i) {
    for (std::size_t j = 1; j <= len2; ++j) {
      if (s1[i - 1] == s2[j - 1]) {
        dp_curr[j] = 1 + dp_prev[j - 1];
      } else {
        dp_curr[j] = std::max(dp_prev[j], dp_curr[j - 1]);
      }
    }
    // Move current row to previous row for next iteration
    dp_prev = dp_curr;
    // Optional: Reset dp_curr[0] if needed, though it's not used in the max
    // calc
    dp_curr[0] = 0;  // Though base case dp[i][0] is always 0
  }

  return dp_prev[len2];  // Result is in the last cell of the previous row
}

// Longest Common Substring Length - Space Optimized O(min(N, M))
auto longest_common_substring_length(std::string_view s1, std::string_view s2)
    -> std::size_t
{
  const std::size_t len1 = s1.length();
  const std::size_t len2 = s2.length();

  if (len1 == 0 || len2 == 0) {
    return 0;
  }

  // Ensure dp vector corresponds to the shorter string length for better
  // optimization
  if (len1 < len2) {
    return longest_common_substring_length(s2,
                                           s1);  // Swap to ensure s2 is shorter
  }

  std::vector<std::size_t> dp(len2 + 1, 0);  // Only need one row
  std::size_t max_length = 0;

  for (std::size_t i = 1; i <= len1; ++i) {
    std::size_t prev_diag = 0;  // Represents dp[i-1][j-1]
    for (std::size_t j = 1; j <= len2; ++j) {
      std::size_t temp = dp[j];  // Store dp[i-1][j] before overwriting
      if (s1[i - 1] == s2[j - 1]) {
        dp[j] = 1 + prev_diag;
        max_length = std::max(max_length, dp[j]);
      } else {
        dp[j] = 0;  // Reset length if characters don't match
      }
      prev_diag = temp;  // Update prev_diag for the next inner loop iteration
    }
  }

  return max_length;
}

// URL Encoding (Percent Encoding)
auto url_encode(std::string_view s) -> std::string
{
  std::ostringstream encoded;
  encoded.fill('0');  // Set fill character for hex representation
  encoded << std::hex << std::uppercase;  // Use uppercase hex digits

  for (unsigned char c : s) {
    // Keep alphanumeric and other unreserved characters
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded << c;
    } else {
      // Percent-encode others
      encoded << '%' << std::setw(2) << static_cast<int>(c);
    }
    // Note: Spaces (' ') are encoded as %20 by this logic.
    // If you specifically want '+' for spaces (common in query strings),
    // add an explicit check: else if (c == ' ') { encoded << '+'; }
  }

  return encoded.str();
}

// URL Decoding (Percent Decoding)
auto url_decode(std::string_view s) -> std::string
{
  std::string decoded;
  decoded.reserve(s.length());  // Reserve estimated size

  for (std::size_t i = 0; i < s.length(); ++i) {
    if (s[i] == '%') {
      // Need two hex digits following %
      if (i + 2 < s.length()
          && std::isxdigit(static_cast<unsigned char>(s[i + 1]))
          && std::isxdigit(static_cast<unsigned char>(s[i + 2])))
      {
        // Convert hex pair to integer
        int value = 0;
        std::stringstream ss;
        ss << std::hex << s.substr(i + 1, 2);
        ss >> value;

        decoded += static_cast<char>(value);
        i += 2;  // Skip the two hex digits
      } else {
        // Invalid sequence - handle as error? For now, maybe append '%'
        // literally or return error? Let's append '%' literally for now, though
        // this might not be ideal. Alternatively, return an empty string or
        // throw an exception. Return empty string on error for simplicity here.
        // Consider adding error handling (e.g., optional<string> or
        // exceptions). return ""; // Option 1: Return empty on error
        decoded +=
            '%';  // Option 2: Append '%' literally if sequence is invalid
      }
    } else if (s[i] == '+') {
      // Decode '+' as space (common practice)
      decoded += ' ';
    } else {
      // Append character as is
      decoded += s[i];
    }
  }
  return decoded;
}

namespace detail  // Keep Base64 helpers internal
{
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// Runtime-initialized, thread-safe lookup table using Meyers' Singleton
inline const std::array<unsigned char, 256>& get_base64_lookup_table()
{
  // This lambda will only be executed once, the first time the function is
  // called. The resulting table is stored in a static variable.
  static const auto lookup_table = []()
  {
    std::array<unsigned char, 256> table {};
    // Initialize using a normal runtime loop
    for (std::size_t i = 0; i < 256; ++i) {
      table[i] = 0xFF;  // Invalid marker
    }
    // Map the valid Base64 characters
    for (unsigned char i = 0; i < 64; ++i) {
      table[static_cast<unsigned char>(base64_chars[i])] = i;
    }
    return table;
  }();  // Immediately invoke the lambda to initialize the static variable
  return lookup_table;
}

// No longer need: create_base64_lookup_table()
// No longer need: static constexpr auto base64_lookup = ...

}  // namespace detail

auto base64_encode(std::string_view data) -> std::string
{
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];  // Stores 3 bytes of input
  unsigned char char_array_4[4];  // Stores 4 encoded chars

  std::size_t in_len = data.length();
  const char* bytes_to_encode = data.data();

  ret.reserve(((in_len + 2) / 3) * 4);  // Pre-allocate memory

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] =
          (char_array_3[0] & 0xfc) >> 2;  // First 6 bits of byte 0
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4)
          + ((char_array_3[1] & 0xf0)
             >> 4);  // Last 2 bits of 0 + first 4 bits of 1
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2)
          + ((char_array_3[2] & 0xc0)
             >> 6);  // Last 4 bits of 1 + first 2 bits of 2
      char_array_4[3] = char_array_3[2] & 0x3f;  // Last 6 bits of byte 2

      for (i = 0; (i < 4); i++)
        ret += detail::base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)  // Handle padding
  {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';  // Zero out remaining bytes

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] =
        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] =
        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    // char_array_4[3] = char_array_3[2] & 0x3f; // This index is not used
    // directly for padded output

    for (j = 0; (j < i + 1); j++)  // Output chars for existing bytes
      ret += detail::base64_chars[char_array_4[j]];

    while ((i++ < 3))  // Add padding '='
      ret += '=';
  }

  return ret;
}

// Returns empty string on error
auto base64_decode(std::string_view encoded_string) -> std::string
{
  std::string decoded_data;
  decoded_data.reserve(((encoded_string.length() + 3) / 4)
                       * 3);  // Estimate size

  // Get the lookup table (initialized on first call)
  const auto& lookup = detail::get_base64_lookup_table();

  std::vector<unsigned char> b64_block;  // Stores 6-bit values for a block
  b64_block.reserve(4);

  for (char c : encoded_string) {
    if (std::isspace(static_cast<unsigned char>(c))) {
      continue;  // Skip whitespace
    }

    if (c == '=') {
      // Padding found, stop processing input characters normally
      break;
    }

    // Use the runtime lookup table via the getter function
    unsigned char lookup_val = lookup.at(static_cast<unsigned char>(c));
    if (lookup_val == 0xFF) {  // Check for the invalid marker
      return "";  // Invalid character encountered
    }

    b64_block.push_back(lookup_val);

    if (b64_block.size() == 4) {
      // Process a full block... (rest of the logic is the same)
      decoded_data +=
          static_cast<char>((b64_block[0] << 2) + ((b64_block[1] & 0x30) >> 4));
      decoded_data += static_cast<char>(((b64_block[1] & 0x0F) << 4)
                                        + ((b64_block[2] & 0x3C) >> 2));
      decoded_data +=
          static_cast<char>(((b64_block[2] & 0x03) << 6) + b64_block[3]);
      b64_block.clear();
    }
  }

  // Handle the final block (which might involve padding)
  // (Logic from previous step remains the same here)
  if (b64_block.size() == 2) {
    decoded_data +=
        static_cast<char>((b64_block[0] << 2) + ((b64_block[1] & 0x30) >> 4));
  } else if (b64_block.size() == 3) {
    decoded_data +=
        static_cast<char>((b64_block[0] << 2) + ((b64_block[1] & 0x30) >> 4));
    decoded_data += static_cast<char>(((b64_block[1] & 0x0F) << 4)
                                      + ((b64_block[2] & 0x3C) >> 2));
  } else if (b64_block.size() == 1 || b64_block.size() > 3) {
    return "";  // Invalid number of characters before padding/end
  }

  return decoded_data;
}
auto slugify(std::string_view s) -> std::string
{
  std::string result;
  result.reserve(s.length());
  bool last_char_was_hyphen =
      true;  // Initialize to true to prevent leading hyphens

  for (char ch : s) {
    char lower_ch =
        static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

    if (std::isalnum(static_cast<unsigned char>(lower_ch))) {
      result += lower_ch;
      last_char_was_hyphen = false;
    } else if (lower_ch == ' ' || lower_ch == '-') {
      if (!last_char_was_hyphen) {
        result += '-';
        last_char_was_hyphen = true;
      }
    } else {
      // Ignore other characters
      // A more advanced version might transliterate common symbols or Unicode
      // here
    }
  }

  // Remove trailing hyphen if present
  if (!result.empty() && result.back() == '-') {
    result.pop_back();
  }

  // Although handled by initial state, double check leading hyphen (
  // belt-and-suspenders )
  if (!result.empty() && result.front() == '-') {
    result.erase(0, 1);
  }

  return result;
}

}  // namespace toolbox::container::string
