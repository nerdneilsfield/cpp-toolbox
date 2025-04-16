#pragma once

#ifndef CPP_TOOLBOX_CONTAINER_STRING_HPP
#  define CPP_TOOLBOX_CONTAINER_STRING_HPP

#  include <limits>
#  include <string>
#  include <string_view>
#  include <vector>

#  include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::container::string
{
/**
 * @brief 使用字符串分隔符将字符串视图分割成字符串向量 / Splits a string view
 * into a vector of strings based on a string delimiter.
 *
 * 在分隔符字符串的每个出现位置分割输入字符串。分隔符之间的空部分也会包含在结果中。分隔符本身不会包含在输出中。
 * / Splits the input string at each occurrence of the delimiter string. Empty
 * parts between delimiters are included in the result. The delimiter itself is
 * not included in the output.
 *
 * @param str 要分割的字符串视图 / The string view to split
 * @param delimiter 用作分隔符的字符串视图 / The string view to use as a
 * delimiter
 * @return std::vector<std::string> 分割后得到的字符串向量 / A vector of strings
 * resulting from the split
 *
 * @code{.cpp}
 * // 使用字符串分隔符分割 / Split by string delimiter
 * auto result = split("apple,orange,,banana", ",");
 * // result 将是 {"apple", "orange", "", "banana"}
 *
 * // 使用多字符分隔符分割 / Split with multi-character delimiter
 * auto result2 = split("one->two->three", "->");
 * // result2 将是 {"one", "two", "three"}
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto split(std::string_view str, std::string_view delimiter)
    -> std::vector<std::string>;

/**
 * @brief 使用字符分隔符将字符串视图分割成字符串向量 / Splits a string view into
 * a vector of strings based on a character delimiter
 *
 * 在分隔符字符的每个出现位置分割输入字符串。分隔符之间的空部分也会包含在结果中。分隔符字符本身不会包含在输出中。
 * / Splits the input string at each occurrence of the delimiter character.
 * Empty parts between delimiters are included in the result. The delimiter
 * character itself is not included in the output.
 *
 * @param str 要分割的字符串视图 / The string view to split
 * @param delimiter 用作分隔符的字符 / The character to use as a delimiter
 * @return std::vector<std::string> 分割后得到的字符串向量 / A vector of strings
 * resulting from the split
 *
 * @code{.cpp}
 * // 使用字符分隔符分割 / Split by character delimiter
 * auto result = split("one|two||three", '|');
 * // result 将是 {"one", "two", "", "three"}
 *
 * // 使用空格分隔符分割 / Split with space delimiter
 * auto result2 = split("hello world  cpp", ' ');
 * // result2 将是 {"hello", "world", "", "cpp"}
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto split(std::string_view str, char delimiter)
    -> std::vector<std::string>;

/**
 * @brief 使用连接字符串将字符串向量连接成单个字符串 / Joins a vector of strings
 * into a single string, separated by a glue string
 *
 * 连接向量中的所有字符串，在每个元素之间插入连接字符串。如果输入向量为空，则返回空字符串。
 * / Concatenates all strings in the vector, inserting the glue string between
 * each element. If the input vector is empty, returns an empty string.
 *
 * @param parts 要连接的字符串向量 / The vector of strings to join
 * @param glue 用于在元素之间插入的字符串视图 / The string view to insert
 * between elements
 * @return std::string 连接后的结果字符串 / The resulting joined string
 *
 * @code{.cpp}
 * // 使用逗号分隔符连接 / Join with comma separator
 * auto result = join({"apple", "banana", "cherry"}, ", ");
 * // result 将是 "apple, banana, cherry"
 *
 * // 使用空连接符连接 / Join with empty glue
 * auto result2 = join({"a", "b", "c"}, "");
 * // result2 将是 "abc"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto join(const std::vector<std::string>& parts,
                             std::string_view glue) -> std::string;

/**
 * @brief 使用连接字符串将字符串视图向量连接成单个字符串 / Joins a vector of
 * string views into a single string, separated by a glue string
 *
 * 连接向量中的所有字符串视图，在每个元素之间插入连接字符串。如果输入向量为空，则返回空字符串。
 * / Concatenates all string views in the vector, inserting the glue string
 * between each element. If the input vector is empty, returns an empty string.
 *
 * @param parts 要连接的字符串视图向量 / The vector of string views to join
 * @param glue 用于在元素之间插入的字符串视图 / The string view to insert
 * between elements
 * @return std::string 连接后的结果字符串 / The resulting joined string
 *
 * @code{.cpp}
 * // 使用连字符连接日期 / Join string views with hyphen for date
 * auto result = join({"2023", "12", "31"}, "-");
 * // result 将是 "2023-12-31"
 *
 * // 使用换行符连接多行文本 / Join with newline for multiple lines
 * auto result2 = join({"line1", "line2", "line3"}, "\n");
 * // result2 将是 "line1\nline2\nline3"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto join(const std::vector<std::string_view>& parts,
                             std::string_view glue) -> std::string;

/**
 * @brief 删除字符串视图开头的空白字符 / Removes leading whitespace characters
 * from a string view
 *
 * 创建一个新字符串，删除所有开头的空白字符。空白字符由 std::isspace
 * 确定（空格、制表符、换行符等）。 / Creates a new string with all leading
 * whitespace characters removed. Whitespace characters are determined by
 * std::isspace (spaces, tabs, newlines, etc.).
 *
 * @param str 输入的字符串视图 / The input string view
 * @return std::string 删除开头空白字符后的新字符串 / A new string with leading
 * whitespace removed
 *
 * @code{.cpp}
 * // 删除开头空白字符示例 / Trim leading whitespace examples
 * auto result = trim_left("   hello");
 * // result 将是 "hello"
 *
 * auto result2 = trim_left("\t\n  world");
 * // result2 将是 "world"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto trim_left(std::string_view str) -> std::string;

/**
 * @brief 删除字符串视图末尾的空白字符 / Removes trailing whitespace characters
 * from a string view
 *
 * 创建一个新字符串，删除所有末尾的空白字符。空白字符由 std::isspace
 * 确定（空格、制表符、换行符等）。 / Creates a new string with all trailing
 * whitespace characters removed. Whitespace characters are determined by
 * std::isspace (spaces, tabs, newlines, etc.).
 *
 * @param str 输入的字符串视图 / The input string view
 * @return std::string 删除末尾空白字符后的新字符串 / A new string with trailing
 * whitespace removed
 *
 * @code{.cpp}
 * // 删除末尾空白字符示例 / Trim trailing whitespace examples
 * auto result = trim_right("hello   ");
 * // result 将是 "hello"
 *
 * auto result2 = trim_right("world\t\n  ");
 * // result2 将是 "world"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto trim_right(std::string_view str) -> std::string;

/**
 * @brief 删除字符串视图两端的空白字符 / Removes both leading and trailing
 * whitespace characters from a string view
 *
 * 创建一个新字符串，删除所有开头和末尾的空白字符。空白字符由 std::isspace
 * 确定。 / Creates a new string with all leading and trailing whitespace
 * characters removed. Whitespace characters are determined by std::isspace.
 *
 * @param str 输入的字符串视图 / The input string view
 * @return std::string 删除两端空白字符后的新字符串 / A new string with leading
 * and trailing whitespace removed
 *
 * @code{.cpp}
 * // 删除两端空白字符示例 / Trim both ends examples
 * auto result = trim("   hello world   ");
 * // result 将是 "hello world"
 *
 * auto result2 = trim("\t\n  test string \n\t");
 * // result2 将是 "test string"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto trim(std::string_view str) -> std::string;

/**
 * @brief 检查字符串视图是否以指定前缀开头 / Checks if a string view starts with
 * a specified prefix
 *
 * 执行区分大小写的比较，检查字符串是否以给定前缀开头。可以处理空字符串和空前缀。
 * / Performs a case-sensitive comparison to check if the string begins with the
 * given prefix. Both empty strings and empty prefixes are handled.
 *
 * @param s 要检查的字符串视图 / The string view to check
 * @param prefix 要查找的前缀字符串视图 / The prefix string view to look for
 * @return bool 如果字符串以前缀开头则返回true，否则返回false / True if the
 * string starts with the prefix, false otherwise
 *
 * @code{.cpp}
 * // 检查前缀示例 / Check prefix examples
 * bool result = starts_with("hello world", "hello");
 * // result 将是 true
 *
 * bool result2 = starts_with("hello world", "world");
 * // result2 将是 false
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto starts_with(std::string_view s, std::string_view prefix)
    -> bool;

/**
 * @brief 检查字符串视图是否以指定后缀结尾 / Checks if a string view ends with a
 * specified suffix
 *
 * 执行区分大小写的比较，检查字符串是否以给定后缀结尾。可以处理空字符串和空后缀。
 * / Performs a case-sensitive comparison to check if the string ends with the
 * given suffix. Both empty strings and empty suffixes are handled.
 *
 * @param s 要检查的字符串视图 / The string view to check
 * @param suffix 要查找的后缀字符串视图 / The suffix string view to look for
 * @return bool 如果字符串以后缀结尾则返回true，否则返回false / True if the
 * string ends with the suffix, false otherwise
 *
 * @code{.cpp}
 * // 检查后缀示例 / Check suffix examples
 * bool result = ends_with("hello world", "world");
 * // result 将是 true
 *
 * bool result2 = ends_with("hello world", "hello");
 * // result2 将是 false
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto ends_with(std::string_view s, std::string_view suffix)
    -> bool;

/**
 * @brief 检查字符串视图是否包含指定子字符串 / Checks if a string view contains
 * a specified substring
 *
 * 执行区分大小写的搜索，查找字符串中的子字符串。可以处理空字符串和空子字符串。
 * / Performs a case-sensitive search for the substring within the string. Both
 * empty strings and empty substrings are handled.
 *
 * @param s 要搜索的字符串视图 / The string view to search within
 * @param substring 要搜索的子字符串 / The substring to search for
 * @return bool 如果找到子字符串则返回true，否则返回false / True if the
 * substring is found within the string, false otherwise
 *
 * @code{.cpp}
 * // 检查子字符串示例 / Check substring examples
 * bool result = contains("hello world", "lo wo");
 * // result 将是 true
 *
 * bool result2 = contains("hello world", "goodbye");
 * // result2 将是 false
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto contains(std::string_view s, std::string_view substring)
    -> bool;

/**
 * @brief 检查字符串视图是否为空或仅包含空白字符 / Checks if a string view is
 * empty or consists only of whitespace characters
 *
 * 判断字符串是否为空或仅包含被 std::isspace 认为是空白的字符。 /
 * Determines if the string is either empty or contains only characters that are
 * considered whitespace by std::isspace.
 *
 * @param s 要检查的字符串视图 / The string view to check
 * @return bool 如果字符串为空或仅包含空白字符则返回true，否则返回false / True
 * if the string is empty or contains only whitespace, false otherwise
 *
 * @code{.cpp}
 * // 检查空或空白字符示例 / Check empty or whitespace examples
 * bool result = is_empty_or_whitespace("");
 * // result 将是 true
 *
 * bool result2 = is_empty_or_whitespace(" \t\n");
 * // result2 将是 true
 *
 * bool result3 = is_empty_or_whitespace("hello");
 * // result3 将是 false
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_empty_or_whitespace(std::string_view s) -> bool;

/**
 * @brief 检查字符串视图是否表示数值（整数或浮点数）/ Checks if a string view
 * represents a numeric value (integer or floating-point)
 *
 * 执行基本的数值格式检查，包括可选的符号、数字和可选的小数点。不处理特定于区域设置的格式或科学记数法。
 * / Performs a basic check for numeric format, including optional sign, digits,
 * and optional decimal point. Does not handle locale-specific formats or
 * scientific notation.
 *
 * @param s 要检查的字符串视图 / The string view to check
 * @return bool 如果字符串看起来是数值则返回true，否则返回false / True if the
 * string appears to be numeric, false otherwise
 *
 * @code{.cpp}
 * // 检查数值格式示例 / Check numeric format examples
 * bool result = is_numeric("12345");
 * // result 将是 true
 *
 * bool result2 = is_numeric("-123.45");
 * // result2 将是 true
 *
 * bool result3 = is_numeric("123abc");
 * // result3 将是 false
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_numeric(std::string_view s) -> bool;

/**
 * @brief 检查字符串视图是否表示有效的整数值 / Checks if a string view
 * represents a valid integer value
 *
 * 此函数验证输入字符串是否由可选的前导符号(+/-)后跟一个或多个数字组成。不处理特定于区域设置的格式或科学记数法。
 * / This function verifies if the input string consists of optional leading
 * sign (+/-) followed by one or more digits. It does not handle locale-specific
 * formats or scientific notation.
 *
 * @param s 要检查的字符串视图 / The string view to check
 * @return bool 如果字符串是有效整数则返回true，否则返回false / True if the
 * string is a valid integer, false otherwise
 *
 * @code{.cpp}
 * // 检查整数格式示例 / Check integer format examples
 * bool result1 = is_integer("12345");
 * // result1 将是 true / result1 will be true
 *
 * bool result2 = is_integer("-42");
 * // result2 将是 true / result2 will be true
 *
 * bool result3 = is_integer("12.34");
 * // result3 将是 false / result3 will be false
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_integer(std::string_view s) -> bool;

/**
 * @brief 检查字符串视图是否表示有效的浮点数值 / Checks if a string view
 * represents a valid floating-point value
 *
 * 此函数验证输入字符串是否由可选的前导符号(+/-)、数字和可选的小数点组成。不处理特定于区域设置的格式、科学记数法或特殊浮点值(NaN、无穷大)。
 * / This function verifies if the input string consists of optional leading
 * sign (+/-), digits, and optional decimal point. It does not handle
 * locale-specific formats, scientific notation, or special floating-point
 * values (NaN, infinity).
 *
 * @param s 要检查的字符串视图 / The string view to check
 * @return bool 如果字符串是有效浮点数则返回true，否则返回false / True if the
 * string is a valid floating-point number, false otherwise
 *
 * @code{.cpp}
 * // 检查浮点数格式示例 / Check float format examples
 * bool result1 = is_float("123.45");
 * // result1 将是 true / result1 will be true
 *
 * bool result2 = is_float("-0.123");
 * // result2 将是 true / result2 will be true
 *
 * bool result3 = is_float("12.34.56");
 * // result3 将是 false / result3 will be false
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_float(std::string_view s) -> bool;

/**
 * @brief 在字符串视图中将一个子字符串替换为另一个子字符串，最多替换指定次数 /
 * Replaces occurrences of a substring within a string view with another
 * substring, up to a specified count
 *
 * 此函数在输入字符串中执行最多'count'次将'old_value'替换为'new_value'的操作。如果'old_value'为空，则在字符之间和字符串开头插入'new_value'。
 * / This function performs up to 'count' replacements of 'old_value' with
 * 'new_value' in the input string. If 'old_value' is empty, it inserts
 * 'new_value' between characters and at the start of the string.
 *
 * @param s 原始字符串视图 / The original string view
 * @param old_value 要被替换的子字符串 / The substring to be replaced
 * @param new_value 用于替换的新子字符串 / The substring to replace with
 * @param count 要执行的最大替换次数，默认替换所有出现 / The maximum number of
 * replacements to perform. Defaults to replacing all occurrences
 * @return 完成替换后的新字符串 / A new string with the replacements made
 *
 * @code{.cpp}
 * // 替换子字符串示例 / Replace substring examples
 * auto result1 = replace("hello world", "world", "cpp");
 * // result1 将是 "hello cpp" / result1 will be "hello cpp"
 *
 * auto result2 = replace("banana", "na", "no", 2);
 * // result2 将是 "bonona" / result2 will be "bonona"
 *
 * auto result3 = replace("abc", "", "-", 2);
 * // result3 将是 "-a-bc" / result3 will be "-a-bc"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto replace(
    std::string_view s,
    std::string_view old_value,
    std::string_view new_value,
    std::size_t count = std::numeric_limits<std::size_t>::max()) -> std::string;

/**
 * @brief 替换字符串视图中所有出现的子字符串 / Replaces all occurrences of a
 * substring within a string view
 *
 * 此函数替换输入字符串中所有'old_value'的实例为'new_value'。等同于使用默认count调用replace。
 * / This function replaces all instances of 'old_value' with 'new_value' in the
 * input string. It is equivalent to calling `replace` with the default count.
 *
 * @param s 原始字符串视图 / The original string view
 * @param old_value 要被替换的子字符串 / The substring to be replaced
 * @param new_value 用于替换的新子字符串 / The substring to replace with
 * @return 替换所有出现后的新字符串 / A new string with all occurrences replaced
 *
 * @code{.cpp}
 * // 替换所有出现示例 / Replace all occurrences examples
 * auto result = replace_all("banana", "na", "no");
 * // result 将是 "bonono" / result will be "bonono"
 *
 * auto result2 = replace_all("hello", "x", "y");
 * // result2 将是 "hello" / result2 will be "hello"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto replace_all(std::string_view s,
                                    std::string_view old_value,
                                    std::string_view new_value) -> std::string;

/**
 * @brief 替换字符串中第N次出现的子字符串 / Replaces the Nth occurrence of a
 * substring within a string
 *
 * 此函数仅替换指定的第N次出现(基于1的索引)的'old_value'为'new_value'。如果未找到指定的出现次数或'old_value'为空，则返回原始字符串的副本。
 * / This function replaces only the specified occurrence (1-based index) of
 * 'old_value' with 'new_value'. If the specified occurrence is not found or
 * 'old_value' is empty, returns a copy of the original string.
 *
 * @param s 原始字符串视图 / The original string view
 * @param old_value 要查找的子字符串，不能为空 / The substring to find. Cannot
 * be empty
 * @param new_value 用于替换的新子字符串 / The substring to replace with
 * @param n 要替换的出现次数的1基索引(1表示第一次，2表示第二次，等等)，必须>0 /
 * The 1-based index of the occurrence to replace (1 for first, 2 for second,
 * etc.). Must be > 0
 * @return
 * 替换第N次出现后的新字符串，如果n无效、from为空或未找到第N次出现则返回原始字符串的副本
 * / A new string with the Nth occurrence replaced, or a copy of the original if
 * invalid
 *
 * @code{.cpp}
 * // 替换第N次出现示例 / Replace nth occurrence examples
 * auto result1 = replace_by_nth("banana", "na", "no", 2);
 * // result1 将是 "banona" / result1 will be "banona"
 *
 * auto result2 = replace_by_nth("hello", "l", "x", 3);
 * // result2 将是 "hello" / result2 will be "hello"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto replace_by_nth(std::string_view s,
                                       std::string_view old_value,
                                       std::string_view new_value,
                                       std::size_t n) -> std::string;

/**
 * @brief 删除字符串中第N次出现的子字符串 / Removes the Nth occurrence of a
 * substring within a string
 *
 * 此函数仅删除指定的第N次出现(基于1的索引)的'from'。如果未找到指定的出现次数或'from'为空，则返回原始字符串的副本。
 * / This function removes only the specified occurrence (1-based index) of
 * 'from'. If the specified occurrence is not found or 'from' is empty, returns
 * a copy of the original string.
 *
 * @param s 原始字符串视图 / The original string view
 * @param from 要查找并删除的子字符串，不能为空 / The substring to find and
 * remove. Cannot be empty
 * @param n 要删除的出现次数的1基索引(1表示第一次，2表示第二次，等等)，必须>0 /
 * The 1-based index of the occurrence to remove (1 for first, 2 for second,
 * etc.). Must be > 0
 * @return
 * 删除第N次出现后的新字符串，如果n无效、from为空或未找到第N次出现则返回原始字符串的副本
 * / A new string with the Nth occurrence removed, or a copy of the original if
 * invalid
 *
 * @code{.cpp}
 * // 删除第N次出现示例 / Remove nth occurrence examples
 * auto result1 = remove_nth("banana", "na", 2);
 * // result1 将是 "bana" / result1 will be "bana"
 *
 * auto result2 = remove_nth("hello", "l", 3);
 * // result2 将是 "hello" / result2 will be "hello"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto remove_nth(std::string_view s,
                                   std::string_view from,
                                   std::size_t n) -> std::string;

/**
 * @brief 从字符串视图中删除子字符串的出现，最多删除指定次数 / Removes
 * occurrences of a substring from a string view, up to a specified count
 *
 * 此函数从输入字符串中删除最多'count'次出现的'value'。等同于用空字符串替换'value'。
 * / This function removes up to 'count' occurrences of 'value' from the input
 * string. It is equivalent to replacing 'value' with an empty string.
 *
 * @param s 原始字符串视图 / The original string view
 * @param value 要删除的子字符串 / The substring to remove
 * @param count 要删除的最大出现次数，默认删除所有出现 / The maximum number of
 * occurrences to remove. Defaults to removing all occurrences
 * @return 删除指定子字符串后的新字符串 / A new string with the specified
 * substring removed
 *
 * @code{.cpp}
 * // 删除子字符串示例 / Remove substring examples
 * auto result1 = remove("banana", "na", 2);
 * // result1 将是 "ba" / result1 will be "ba"
 *
 * auto result2 = remove("hello world", "l");
 * // result2 将是 "heo word" / result2 will be "heo word"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto remove(
    std::string_view s,
    std::string_view value,
    std::size_t count = std::numeric_limits<std::size_t>::max()) -> std::string;

/**
 * @brief 从字符串视图中删除所有出现的子字符串 / Removes all occurrences of a
 * substring from a string view
 *
 * 此函数从输入字符串中删除指定子字符串的每个实例。等同于使用默认count调用remove(删除所有出现)。
 * / This function removes every instance of the specified substring from the
 * input string. It is equivalent to calling `remove` with the default count.
 *
 * @param s 要处理的原始字符串视图 / The original string view to process
 * @param value 要删除的子字符串，不能为空 / The substring to remove. Cannot be
 * empty
 * @return 删除所有子字符串出现后的新字符串 / A new string with all occurrences
 * of the substring removed
 *
 * @code{.cpp}
 * // 删除所有出现示例 / Remove all occurrences examples
 * auto result = remove_all("banana", "na");
 * // result 将是 "ba" / result will be "ba"
 *
 * auto result2 = remove_all("hello world hello", "hello");
 * // result2 将是 " world " / result2 will be " world "
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto remove_all(std::string_view s, std::string_view value)
    -> std::string;

/**
 * @brief 从字符串视图中删除所有出现的特定字符 / Removes all occurrences of a
 * specific character from a string view
 *
 * 此函数从输入字符串中删除指定字符的每个实例。 /
 * This function removes every instance of the specified character from the
 * input string.
 *
 * @param s 要处理的原始字符串视图 / The original string view to process
 * @param value 要删除的字符 / The character to remove
 * @return 删除所有字符出现后的新字符串 / A new string with all occurrences of
 * the character removed
 *
 * @code{.cpp}
 * // 删除所有字符示例 / Remove all character examples
 * auto result = remove_all("hello world", 'l');
 * // result 将是 "heo word" / result will be "heo word"
 *
 * auto result2 = remove_all("text with spaces", ' ');
 * // result2 将是 "textwithspaces" / result2 will be "textwithspaces"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto remove_all(std::string_view s, char value)
    -> std::string;

/**
 * @brief 将字符串视图中的所有字符转换为小写 / Converts all characters in a
 * string view to lowercase
 *
 * 此函数创建一个新字符串，其中所有字符都转换为其小写等效字符。转换依赖于区域设置。
 * / This function creates a new string where all characters are converted to
 * their lowercase equivalents. The conversion is locale-dependent.
 *
 * @param s 要转换的输入字符串视图 / The input string view to convert
 * @return 所有字符转换为小写后的新字符串 / A new string with all characters
 * converted to lowercase
 *
 * @code{.cpp}
 * // 转换为小写示例 / Convert to lowercase examples
 * auto result = to_lower("Hello World");
 * // result 将是 "hello world" / result will be "hello world"
 *
 * auto result2 = to_lower("CamelCaseString");
 * // result2 将是 "camelcasestring" / result2 will be "camelcasestring"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto to_lower(std::string_view s) -> std::string;

/**
 * @brief 将字符串视图中的所有字符转换为大写 / Converts all characters in a
 * string view to uppercase
 *
 * 此函数创建一个新字符串，其中所有字符都转换为其大写等效字符。转换依赖于区域设置。
 * / This function creates a new string where all characters are converted to
 * their uppercase equivalents. The conversion is locale-dependent.
 *
 * @param s 要转换的输入字符串视图 / The input string view to convert
 * @return 所有字符转换为大写后的新字符串 / A new string with all characters
 * converted to uppercase
 *
 * @code{.cpp}
 * // 转换为大写示例 / Convert to uppercase examples
 * auto result = to_upper("Hello World");
 * // result 将是 "HELLO WORLD" / result will be "HELLO WORLD"
 *
 * auto result2 = to_upper("CamelCaseString");
 * // result2 将是 "CAMELCASESTRING" / result2 will be "CAMELCASESTRING"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto to_upper(std::string_view s) -> std::string;

/**
 * @brief 将字符串视图转换为标准字符串 / Converts a string view to a standard
 * string
 *
 * 此函数创建一个新的std::string对象，包含与输入字符串视图相同的字符序列。用于获取字符串数据的拥有副本。
 * / This function creates a new std::string object containing the same
 * character sequence as the input string view. It is useful for obtaining an
 * owned copy of the string data.
 *
 * @param s 要转换的输入字符串视图 / The input string view to convert
 * @return 包含相同字符序列的std::string对象 / A std::string object containing
 * the same character sequence
 *
 * @code{.cpp}
 * // 将字符串视图转换为字符串示例 / Convert string view to string examples
 * auto result = to_string("hello");
 * // result 将是包含 "hello" 的std::string / result will be a std::string
 * containing "hello"
 *
 * std::string_view sv = "long string";
 * auto result2 = to_string(sv.substr(0, 4));
 * // result2 将是包含 "long" 的std::string / result2 will be a std::string
 * containing "long"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto to_string(std::string_view s) -> std::string;

/**
 * @brief 在字符串左侧填充指定字符以达到最小宽度 / Pads a string on the left
 * with a specified character to reach a minimum width
 *
 * 此函数在输入字符串的左侧添加填充字符，直到达到指定的宽度。如果字符串已经超过目标宽度，则返回原始字符串的副本。
 * / This function adds padding characters to the left of the input string until
 * it reaches the specified width. If the string is already longer than the
 * target width, it returns a copy of the original string.
 *
 * @param s 要填充的输入字符串视图 / The input string view to pad
 * @param width 结果字符串的最小期望宽度 / The minimum desired width of the
 * resulting string
 * @param pad_char 用于填充的字符，默认为空格(' ') / The character to use for
 * padding. Defaults to space (' ')
 * @return 必要时在左侧填充后的新字符串 / A new string, padded on the left if
 * necessary
 *
 * @code{.cpp}
 * // 左侧填充示例 / Left pad examples
 * auto result = left_pad("hello", 10);
 * // result 将是 "     hello" / result will be "     hello"
 *
 * auto result2 = left_pad("42", 5, '0');
 * // result2 将是 "00042" / result2 will be "00042"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto left_pad(std::string_view s,
                                 std::size_t width,
                                 char pad_char = ' ') -> std::string;

/**
 * @brief 在字符串右侧填充指定字符以达到最小宽度 / Pads a string on the right
 * with a specified character to reach a minimum width
 *
 * 此函数在输入字符串的右侧添加填充字符，直到达到指定的宽度。如果字符串已经超过目标宽度，则返回原始字符串的副本。
 * / This function adds padding characters to the right of the input string
 * until it reaches the specified width. If the string is already longer than
 * the target width, it returns a copy of the original string.
 *
 * @param s 要填充的输入字符串视图 / The input string view to pad
 * @param width 结果字符串的最小期望宽度 / The minimum desired width of the
 * resulting string
 * @param pad_char 用于填充的字符，默认为空格(' ') / The character to use for
 * padding. Defaults to space (' ')
 * @return 必要时在右侧填充后的新字符串 / A new string, padded on the right if
 * necessary
 *
 * @code{.cpp}
 * // 右侧填充示例 / Right pad examples
 * auto result = right_pad("hello", 10);
 * // result 将是 "hello     " / result will be "hello     "
 *
 * auto result2 = right_pad("text", 8, '-');
 * // result2 将是 "text----" / result2 will be "text----"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto right_pad(std::string_view s,
                                  std::size_t width,
                                  char pad_char = ' ') -> std::string;

/**
 * @brief 使用指定字符填充字符串以达到最小宽度，可控制填充位置 / Pads a string
 * with a specified character to reach a minimum width, controlling padding
 * position
 *
 * 此函数根据指定的位置参数在字符串中添加填充字符。位置参数控制填充的添加位置：0表示左填充，1表示右填充，其他值表示居中填充。
 * / This function adds padding characters to the string based on the specified
 * position parameter. The position parameter controls where the padding is
 * added: 0 for left padding, 1 for right padding, and other values for center
 * padding.
 *
 * @param s 要填充的输入字符串视图 / The input string view to pad
 * @param width 结果字符串的最小期望宽度 / The minimum desired width of the
 * resulting string
 * @param pad_char 用于填充的字符，默认为空格(' ') / The character to use for
 * padding. Defaults to space (' ')
 * @param position
 * 指示填充位置的值（0表示左侧，1表示右侧，其他表示居中），默认为0 / A value
 * indicating the padding position (0 for left, 1 for right, other for center).
 * Defaults to 0
 * @return 根据指定参数填充后的新字符串 / A new string, padded according to the
 * specified parameters
 *
 * @code{.cpp}
 * // 左侧填充（默认） / Left padding (default)
 * auto result = pad("hello", 10);
 * // result 将是 "     hello" / result will be "     hello"
 *
 * // 右侧填充 / Right padding
 * auto result2 = pad("world", 10, ' ', 1);
 * // result2 将是 "world     " / result2 will be "world     "
 *
 * // 居中填充 / Center padding
 * auto result3 = pad("center", 10, '-', 2);
 * // result3 将是 "--center--" / result3 will be "--center--"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto pad(std::string_view s,
                            std::size_t width,
                            char pad_char = ' ',
                            std::size_t position = 0) -> std::string;

/**
 * @brief 反转字符串视图中字符的顺序 / Reverses the order of characters in a
 * string view
 *
 * 此函数创建一个新字符串，其中字符的顺序与输入字符串视图相比是相反的 /
 * This function creates a new string where the characters are in reverse order
 * compared to the input string view
 *
 * @param s 要反转的输入字符串视图 / The input string view to reverse
 * @return 字符顺序相反的新字符串 / A new string with the characters in reverse
 * order
 *
 * @code{.cpp}
 * // 反转普通字符串 / Reverse a string
 * auto result = reverse("hello");
 * // result 将是 "olleh" / result will be "olleh"
 *
 * // 反转回文字符串 / Reverse a palindrome
 * auto result2 = reverse("racecar");
 * // result2 将是 "racecar" / result2 will be "racecar"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto reverse(std::string_view s) -> std::string;

/**
 * @brief 尝试从字符串视图解析整数 / Attempts to parse an integer from a string
 * view
 *
 * 此函数使用标准C++解析机制（如std::from_chars或std::stoi）将字符串转换为整数。它执行验证以确保字符串包含有效的整数格式，并检查潜在的溢出条件
 * / This function uses standard C++ parsing mechanisms to convert a string to
 * an integer. It performs validation to ensure the string contains a valid
 * integer format and checks for potential overflow conditions
 *
 * @param s 要解析的字符串视图 / The string view to parse
 * @param out 如果解析成功，存储解析值的整数引用 / Reference to an integer where
 * the parsed value will be stored if successful
 * @return
 * 如果解析成功则返回true，否则返回false。仅当函数返回true时才会修改输出参数out
 * / True if parsing was successful, false otherwise
 *
 * @code{.cpp}
 * // 成功解析 / Successful parsing
 * int value;
 * bool success = try_parse_int("12345", value);
 * // success 将是 true，value 将是 12345 / success will be true, value will be
 * 12345
 *
 * // 解析失败（无效格式） / Failed parsing (invalid format)
 * int value2;
 * bool success2 = try_parse_int("12abc", value2);
 * // success2 将是 false，value2 保持不变 / success2 will be false, value2
 * remains unchanged
 *
 * // 解析失败（溢出） / Failed parsing (overflow)
 * int value3;
 * bool success3 = try_parse_int("99999999999999999999", value3);
 * // success3 将是 false，value3 保持不变 / success3 will be false, value3
 * remains unchanged
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto try_parse_int(std::string_view s, int& out) -> bool;

/**
 * @brief 尝试从字符串视图解析双精度浮点数 / Attempts to parse a
 * double-precision floating-point number from a string view
 *
 * 此函数使用标准C++解析机制（如std::from_chars或std::stod）将字符串转换为double类型。它执行验证以确保字符串包含有效的浮点数格式，并检查潜在的上溢/下溢条件
 * / This function uses standard C++ parsing mechanisms to convert a string to a
 * double. It performs validation to ensure the string contains a valid
 * floating-point format and checks for potential overflow/underflow conditions
 *
 * @param s 要解析的字符串视图 / The string view to parse
 * @param out 如果解析成功，存储解析值的double引用 / Reference to a double where
 * the parsed value will be stored if successful
 * @return
 * 如果解析成功则返回true，否则返回false。仅当函数返回true时才会修改输出参数out
 * / True if parsing was successful, false otherwise
 *
 * @code{.cpp}
 * // 成功解析 / Successful parsing
 * double value;
 * bool success = try_parse_double("3.14159", value);
 * // success 将是 true，value 将是 3.14159 / success will be true, value will
 * be 3.14159
 *
 * // 解析失败（无效格式） / Failed parsing (invalid format)
 * double value2;
 * bool success2 = try_parse_double("3.14.15", value2);
 * // success2 将是 false，value2 保持不变 / success2 will be false, value2
 * remains unchanged
 *
 * // 解析失败（溢出） / Failed parsing (overflow)
 * double value3;
 * bool success3 = try_parse_double("1e999", value3);
 * // success3 将是 false，value3 保持不变 / success3 will be false, value3
 * remains unchanged
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto try_parse_double(std::string_view s, double& out)
    -> bool;

/**
 * @brief 尝试从字符串视图解析单精度浮点数 / Attempts to parse a
 * single-precision floating-point number from a string view
 *
 * 此函数使用标准C++解析机制（如std::from_chars或std::stof）将字符串转换为float类型。它执行验证以确保字符串包含有效的浮点数格式，并检查潜在的上溢/下溢条件
 * / This function uses standard C++ parsing mechanisms to convert a string to a
 * float. It performs validation to ensure the string contains a valid
 * floating-point format and checks for potential overflow/underflow conditions
 *
 * @param s 要解析的字符串视图 / The string view to parse
 * @param out 如果解析成功，存储解析值的float引用 / Reference to a float where
 * the parsed value will be stored if successful
 * @return
 * 如果解析成功则返回true，否则返回false。仅当函数返回true时才会修改输出参数out
 * / True if parsing was successful, false otherwise
 *
 * @code{.cpp}
 * // 成功解析 / Successful parsing
 * float value;
 * bool success = try_parse_float("2.71828", value);
 * // success 将是 true，value 将是 2.71828f / success will be true, value will
 * be 2.71828f
 *
 * // 解析失败（无效格式） / Failed parsing (invalid format)
 * float value2;
 * bool success2 = try_parse_float("2.71.82", value2);
 * // success2 将是 false，value2 保持不变 / success2 will be false, value2
 * remains unchanged
 *
 * // 解析失败（溢出） / Failed parsing (overflow)
 * float value3;
 * bool success3 = try_parse_float("1e99", value3);
 * // success3 将是 false，value3 保持不变 / success3 will be false, value3
 * remains unchanged
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto try_parse_float(std::string_view s, float& out) -> bool;

/**
 * @brief 计算两个字符串之间的Levenshtein距离 / Calculates the Levenshtein
 * distance between two strings
 *
 * Levenshtein距离是衡量两个序列差异的字符串度量。它被定义为将一个字符串转换为另一个字符串所需的最小单字符编辑次数（插入、删除或替换）
 * / The Levenshtein distance is a string metric for measuring the difference
 * between two sequences. It is defined as the minimum number of
 * single-character edits required to change one string into the other
 *
 * @param s1 第一个字符串视图 / The first string view
 * @param s2 第二个字符串视图 / The second string view
 * @return s1和s2之间的Levenshtein距离（非负整数） / The Levenshtein distance
 * between s1 and s2 (a non-negative integer)
 *
 * @code{.cpp}
 * // "kitten"和"sitting"之间的距离 / Distance between "kitten" and "sitting"
 * auto distance = levenshtein_distance("kitten", "sitting");
 * // distance 将是 3（k→s, e→i, +g） / distance will be 3 (k→s, e→i, +g)
 *
 * // "book"和"back"之间的距离 / Distance between "book" and "back"
 * auto distance2 = levenshtein_distance("book", "back");
 * // distance2 将是 2（o→a, o→c） / distance2 will be 2 (o→a, o→c)
 *
 * // 相同字符串之间的距离 / Distance between identical strings
 * auto distance3 = levenshtein_distance("same", "same");
 * // distance3 将是 0 / distance3 will be 0
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto levenshtein_distance(std::string_view s1,
                                             std::string_view s2)
    -> std::size_t;

/**
 * @brief 计算两个字符串的最长公共子序列（LCS）长度 / Calculates the length of
 * the Longest Common Subsequence (LCS) of two strings
 *
 * 子序列是以相同相对顺序出现但不一定连续的序列。此函数查找两个输入字符串共有的最长此类序列的长度
 * / A subsequence is a sequence that appears in the same relative order but not
 * necessarily contiguous. This function finds the length of the longest such
 * sequence common to both input strings
 *
 * @param s1 第一个字符串视图 / The first string view
 * @param s2 第二个字符串视图 / The second string view
 * @return 最长公共子序列的长度 / The length of the longest common subsequence
 *
 * @code{.cpp}
 * // "ABCDEF"和"AUBCDF"的LCS / LCS of "ABCDEF" and "AUBCDF"
 * auto lcs_len = longest_common_subsequence_length("ABCDEF", "AUBCDF");
 * // lcs_len 将是 4（ABCD） / lcs_len will be 4 (ABCD)
 *
 * // "AGGTAB"和"GXTXAYB"的LCS / LCS of "AGGTAB" and "GXTXAYB"
 * auto lcs_len2 = longest_common_subsequence_length("AGGTAB", "GXTXAYB");
 * // lcs_len2 将是 4（GTAB） / lcs_len2 will be 4 (GTAB)
 *
 * // 完全不同字符串的LCS / LCS of completely different strings
 * auto lcs_len3 = longest_common_subsequence_length("abc", "xyz");
 * // lcs_len3 将是 0 / lcs_len3 will be 0
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto longest_common_subsequence_length(std::string_view s1,
                                                          std::string_view s2)
    -> std::size_t;

/**
 * @brief 计算两个字符串的最长公共子串长度 / Calculates the length of the
 * Longest Common Substring of two strings
 *
 * 子串是字符串中连续字符的序列。此函数查找两个输入字符串共有的最长此类序列的长度
 * / A substring is a contiguous sequence of characters within a string. This
 * function finds the length of the longest such sequence common to both input
 * strings
 *
 * @param s1 第一个字符串视图 / The first string view
 * @param s2 第二个字符串视图 / The second string view
 * @return 最长公共子串的长度 / The length of the longest common substring
 *
 * @code{.cpp}
 * // "ABABC"和"BABCA"的最长公共子串 / Longest common substring of "ABABC" and
 * "BABCA" auto lcstr_len = longest_common_substring_length("ABABC", "BABCA");
 * // lcstr_len 将是 4（BABC） / lcstr_len will be 4 (BABC)
 *
 * // "abcdef"和"zcdemf"的最长公共子串 / Longest common substring of "abcdef"
 * and "zcdemf" auto lcstr_len2 = longest_common_substring_length("abcdef",
 * "zcdemf");
 * // lcstr_len2 将是 3（cde） / lcstr_len2 will be 3 (cde)
 *
 * // 完全不同字符串的最长公共子串 / Longest common substring of completely
 * different strings auto lcstr_len3 = longest_common_substring_length("abc",
 * "xyz");
 * // lcstr_len3 将是 0 / lcstr_len3 will be 0
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto longest_common_substring_length(std::string_view s1,
                                                        std::string_view s2)
    -> std::size_t;

/**
 * @brief 对字符串进行百分比编码以安全地包含在URL中 / Percent-encodes a string
 * for safe inclusion in a URL
 *
 * 此函数根据RFC
 * 3986实现URL编码（也称为百分比编码）。它对除了未保留字符（字母数字、'-'、'_'、'.'、'~'）之外的所有字符进行编码。空格被编码为"%20"，其他字符被编码为%XX，其中XX是字符字节值的十六进制表示
 * / This function implements URL encoding according to RFC 3986. It encodes all
 * characters except unreserved characters (alphanumeric, '-', '_', '.', '~').
 * Spaces are encoded as "%20", and other characters are encoded as %XX where XX
 * is the hexadecimal representation of the character's byte value
 *
 * @param s 要编码的字符串视图 / The string view to encode
 * @return URL编码后的字符串 / The URL-encoded string
 *
 * @code{.cpp}
 * // 编码简单字符串 / Encoding a simple string
 * auto encoded = url_encode("Hello World!");
 * // encoded 将是 "Hello%20World%21" / encoded will be "Hello%20World%21"
 *
 * // 编码特殊字符 / Encoding special characters
 * auto encoded2 = url_encode("100% true");
 * // encoded2 将是 "100%25%20true" / encoded2 will be "100%25%20true"
 *
 * // 编码带查询参数的URL / Encoding a URL with query parameters
 * auto encoded3 = url_encode("https://example.com/search?q=c++");
 * // encoded3 将是 "https%3A%2F%2Fexample.com%2Fsearch%3Fq%3Dc%2B%2B" /
 * encoded3 will be "https%3A%2F%2Fexample.com%2Fsearch%3Fq%3Dc%2B%2B"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto url_encode(std::string_view s) -> std::string;

/**
 * @brief 解码百分比编码的字符串（URL解码） / Decodes a percent-encoded string
 * (URL decoding)
 *
 * 此函数根据RFC
 * 3986实现URL解码（百分比解码）。它将%XX序列转换回原始字符，其中XX是十六进制值。为了与表单编码兼容，还处理'+'字符，将其转换为空格
 * / This function implements URL decoding according to RFC 3986. It converts
 * %XX sequences back to their original characters, where XX is a hexadecimal
 * value. Also handles '+' characters by converting them to spaces, for
 * compatibility with form encoding
 *
 * @param s 要解码的URL编码字符串视图 / The URL-encoded string view to decode
 * @return
 * 解码后的字符串。如果由于无效的%序列或其他编码错误导致解码失败，则返回空字符串
 * / The decoded string. Returns an empty string if decoding fails
 *
 * @code{.cpp}
 * // 解码简单的URL编码字符串 / Decode a simple URL-encoded string
 * auto decoded = url_decode("Hello%20World%21");
 * // decoded 将是 "Hello World!" / decoded will be "Hello World!"
 *
 * // 解码表单编码兼容性 / Decode with form encoding compatibility
 * auto decoded2 = url_decode("100%25+true");
 * // decoded2 将是 "100% true" / decoded2 will be "100% true"
 *
 * // 解码复杂URL / Decode a complex URL
 * auto decoded3 =
 * url_decode("https%3A%2F%2Fexample.com%2Fsearch%3Fq%3Dc%2B%2B");
 * // decoded3 将是 "https://example.com/search?q=c++" / decoded3 will be
 * "https://example.com/search?q=c++"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto url_decode(std::string_view s) -> std::string;

/**
 * @brief 将二进制数据（以字符串视图表示）编码为Base64格式 / Encodes binary data
 * (represented as a string_view) into Base64 format
 *
 * 此函数根据RFC
 * 4648实现Base64编码。它使用Base64字母表将二进制数据转换为ASCII字符串表示。如有必要，输出字符串将用'='字符填充，使其长度为4的倍数
 * / This function implements Base64 encoding according to RFC 4648. It converts
 * binary data into an ASCII string representation using the Base64 alphabet.
 * The output string will be padded with '=' characters if necessary
 *
 * @param data 要编码的原始二进制数据，以字符串视图表示 / The raw binary data to
 * encode, represented as a string_view
 * @return Base64编码的字符串 / The Base64 encoded string
 *
 * @code{.cpp}
 * // 编码简单文本 / Encode simple text
 * auto encoded = base64_encode("Hello World");
 * // encoded 将是 "SGVsbG8gV29ybGQ=" / encoded will be "SGVsbG8gV29ybGQ="
 *
 * // 编码二进制数据 / Encode binary data
 * std::string binary_data = {0x00, 0x01, 0x02, 0x03};
 * auto encoded2 = base64_encode(binary_data);
 * // encoded2 将是 "AAECAw==" / encoded2 will be "AAECAw=="
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto base64_encode(std::string_view data) -> std::string;

/**
 * @brief 将Base64编码的字符串解码回原始数据 / Decodes a Base64 encoded string
 * back into its original data
 *
 * 此函数根据RFC
 * 4648实现Base64解码。它处理填充字符（'='）并忽略输入字符串中的空白字符。函数以字符串形式返回解码后的二进制数据
 * / This function implements Base64 decoding according to RFC 4648. It handles
 * padding characters ('=') and ignores whitespace characters within the input
 * string. The function returns the decoded binary data as a string
 *
 * @param encoded_data 要解码的Base64编码字符串视图 / The Base64 encoded string
 * view to decode
 * @return
 * 解码后的数据字符串。如果由于无效输入（如无效字符、不正确的填充）导致解码失败，则返回空字符串
 * / The decoded data as a string. Returns an empty string if decoding fails
 *
 * @code{.cpp}
 * // 解码简单Base64字符串 / Decode simple Base64 string
 * auto decoded = base64_decode("SGVsbG8gV29ybGQ=");
 * // decoded 将是 "Hello World" / decoded will be "Hello World"
 *
 * // 解码带空白字符的字符串 / Decode with whitespace
 * auto decoded2 = base64_decode(" AA EC Aw == ");
 * // decoded2 将是 "\x00\x01\x02\x03" / decoded2 will be "\x00\x01\x02\x03"
 *
 * // 无效输入返回空字符串 / Invalid input returns empty string
 * auto decoded3 = base64_decode("Invalid!");
 * // decoded3 将是 "" / decoded3 will be ""
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto base64_decode(std::string_view encoded_data)
    -> std::string;

/**
 * @brief 将字符串转换为URL友好的"slug" / Converts a string into a URL-friendly
 * "slug"
 *
 * 此函数将字符串转换为适合在URL中使用的格式。它执行以下转换：
 * 1. 将字符串转换为小写
 * 2. 将空格和连续的非字母数字字符替换为单个连字符
 * 3. 删除其他非字母数字字符（连字符除外）
 * 4. 删除开头和结尾的连字符 /
 * This function transforms a string into a format suitable for use in URLs by:
 * 1. Converting the string to lowercase
 * 2. Replacing spaces and consecutive non-alphanumeric characters with a single
 * hyphen
 * 3. Removing other non-alphanumeric characters (except hyphens)
 * 4. Trimming leading and trailing hyphens
 *
 * @param s 要转换的输入字符串视图 / The input string view to convert
 * @return 生成的slug字符串 / The generated slug string
 *
 * @code{.cpp}
 * // 基本slug生成 / Basic slug generation
 * auto slug = slugify("Hello World!");
 * // slug 将是 "hello-world" / slug will be "hello-world"
 *
 * // 复杂字符串转换 / Complex string conversion
 * auto slug2 = slugify("This is a Test -- 123!");
 * // slug2 将是 "this-is-a-test-123" / slug2 will be "this-is-a-test-123"
 *
 * // 处理特殊字符 / Handling special characters
 * auto slug3 = slugify("C++ Programming 101");
 * // slug3 将是 "c-programming-101" / slug3 will be "c-programming-101"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto slugify(std::string_view s) -> std::string;

}  // namespace toolbox::container::string
#endif  // CPP_TOOLBOX_CONTAINER_STRING_HPP
