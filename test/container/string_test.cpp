#include "cpp-toolbox/container/string.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <vector>
#include <string>
#include <string_view>
#include <limits>
#include <cmath>

// Import the namespace for convenience
using namespace toolbox::container::string;
using Catch::Matchers::Equals;

TEST_CASE("String Splitting Functions", "[string][split]")
{
    SECTION("Split by string delimiter")
    {
        REQUIRE_THAT(split("a,b,c", ","), Equals<std::string>({ "a", "b", "c" }));
        REQUIRE_THAT(split("a,,c", ","), Equals<std::string>({ "a", "", "c" }));
        REQUIRE_THAT(split(",b,c", ","), Equals<std::string>({ "", "b", "c" }));
        REQUIRE_THAT(split("a,b,", ","), Equals<std::string>({ "a", "b", "" }));
        REQUIRE_THAT(split("abc", ","), Equals<std::string>({ "abc" }));
        REQUIRE_THAT(split("", ","), Equals<std::string>({ "" }));
        REQUIRE_THAT(split(",,", ","), Equals<std::string>({ "", "", "" }));
        REQUIRE_THAT(split("a delim b delim c", " delim "), Equals<std::string>({ "a", "b", "c" }));
        REQUIRE_THAT(split("start end", " "), Equals<std::string>({ "start", "end" }));
        // Empty delimiter splits into characters
        REQUIRE_THAT(split("abc", ""), Equals<std::string>({ "a", "b", "c" }));
        REQUIRE_THAT(split("", ""), Equals<std::string>({}));
    }

    SECTION("Split by char delimiter")
    {
        REQUIRE_THAT(split("a:b:c", ':'), Equals<std::string>({ "a", "b", "c" }));
        REQUIRE_THAT(split("a::c", ':'), Equals<std::string>({ "a", "", "c" }));
        REQUIRE_THAT(split(":b:c", ':'), Equals<std::string>({ "", "b", "c" }));
        REQUIRE_THAT(split("a:b:", ':'), Equals<std::string>({ "a", "b", "" }));
        REQUIRE_THAT(split("abc", ':'), Equals<std::string>({ "abc" }));
        REQUIRE_THAT(split("", ':'), Equals<std::string>({ "" }));
        REQUIRE_THAT(split("::", ':'), Equals<std::string>({ "", "", "" }));
    }
}

TEST_CASE("String Joining Functions", "[string][join]")
{
    SECTION("Join vector of strings")
    {
        REQUIRE(join(std::vector<std::string>{ "a", "b", "c" }, ",") == "a,b,c");
        REQUIRE(join(std::vector<std::string>{ "a", "", "c" }, ",") == "a,,c");
        REQUIRE(join(std::vector<std::string>{ "", "b", "c" }, ",") == ",b,c");
        REQUIRE(join(std::vector<std::string>{ "a", "b", "" }, ",") == "a,b,");
        REQUIRE(join(std::vector<std::string>{ "abc" }, ",") == "abc");
        REQUIRE(join(std::vector<std::string>{}, ",") == "");
        REQUIRE(join(std::vector<std::string>{ "a", "b", "c" }, " -- ") == "a -- b -- c");
    }

    SECTION("Join vector of string_views")
    {
        using namespace std::string_view_literals;
        std::vector<std::string_view> views = { "a"sv, "b"sv, "c"sv };
        REQUIRE(join(views, ",") == "a,b,c");

        std::vector<std::string_view> views_with_empty = { "a"sv, ""sv, "c"sv };
        REQUIRE(join(views_with_empty, ",") == "a,,c");

        std::vector<std::string_view> single_view = { "abc"sv };
        REQUIRE(join(single_view, ",") == "abc");

        std::vector<std::string_view> empty_views = {};
        REQUIRE(join(empty_views, ",") == "");

        REQUIRE(join(views, " -- ") == "a -- b -- c");
    }
}

TEST_CASE("String Trimming Functions", "[string][trim]")
{
    REQUIRE(trim_left("  abc") == "abc");
    REQUIRE(trim_left("abc  ") == "abc  ");
    REQUIRE(trim_left("  abc  ") == "abc  ");
    REQUIRE(trim_left("abc") == "abc");
    REQUIRE(trim_left("  ") == "");
    REQUIRE(trim_left("") == "");
    REQUIRE(trim_left("\t\n abc") == "abc");

    REQUIRE(trim_right("abc  ") == "abc");
    REQUIRE(trim_right("  abc") == "  abc");
    REQUIRE(trim_right("  abc  ") == "  abc");
    REQUIRE(trim_right("abc") == "abc");
    REQUIRE(trim_right("  ") == "");
    REQUIRE(trim_right("") == "");
    REQUIRE(trim_right("abc \t\n") == "abc");

    REQUIRE(trim("  abc  ") == "abc");
    REQUIRE(trim("  abc") == "abc");
    REQUIRE(trim("abc  ") == "abc");
    REQUIRE(trim("abc") == "abc");
    REQUIRE(trim("  ") == "");
    REQUIRE(trim("") == "");
    REQUIRE(trim("\t\n abc \t\n") == "abc");
}

TEST_CASE("String Predicate Functions", "[string][predicates]")
{
    SECTION("starts_with")
    {
        REQUIRE(starts_with("abcdef", "abc"));
        REQUIRE_FALSE(starts_with("abcdef", "def"));
        REQUIRE(starts_with("abc", "abc"));
        REQUIRE_FALSE(starts_with("ab", "abc"));
        REQUIRE(starts_with("abc", ""));
        REQUIRE(starts_with("", ""));
        REQUIRE_FALSE(starts_with("", "a"));
    }

    SECTION("ends_with")
    {
        REQUIRE(ends_with("abcdef", "def"));
        REQUIRE_FALSE(ends_with("abcdef", "abc"));
        REQUIRE(ends_with("abc", "abc"));
        REQUIRE_FALSE(ends_with("abc", "abcd"));
        REQUIRE(ends_with("abc", ""));
        REQUIRE(ends_with("", ""));
        REQUIRE_FALSE(ends_with("", "a"));
    }

    SECTION("contains")
    {
        REQUIRE(contains("abcdef", "bcd"));
        REQUIRE(contains("abcdef", "abc"));
        REQUIRE(contains("abcdef", "def"));
        REQUIRE_FALSE(contains("abcdef", "xyz"));
        REQUIRE(contains("abc", "abc"));
        REQUIRE(contains("abc", ""));
        REQUIRE(contains("", ""));
        REQUIRE_FALSE(contains("", "a"));
        REQUIRE_FALSE(contains("abc", "xyz"));
    }

    SECTION("is_empty_or_whitespace")
    {
        REQUIRE(is_empty_or_whitespace(""));
        REQUIRE(is_empty_or_whitespace(" "));
        REQUIRE(is_empty_or_whitespace("\t\n "));
        REQUIRE_FALSE(is_empty_or_whitespace(" a "));
        REQUIRE_FALSE(is_empty_or_whitespace("abc"));
    }
}

TEST_CASE("String Type Check Functions", "[string][typecheck]")
{
    // Note: These checks assume NO leading/trailing whitespace
    SECTION("is_integer")
    {
        REQUIRE(is_integer("123"));
        REQUIRE(is_integer("-123"));
        REQUIRE(is_integer("+123"));
        REQUIRE(is_integer("0"));
        REQUIRE_FALSE(is_integer("123.4"));
        REQUIRE_FALSE(is_integer("123a"));
        REQUIRE_FALSE(is_integer("abc"));
        REQUIRE_FALSE(is_integer(""));
        REQUIRE_FALSE(is_integer("+"));
        REQUIRE_FALSE(is_integer("-"));
        REQUIRE_FALSE(is_integer(" 123")); // whitespace disallowed by assumption
    }

    SECTION("is_float") // Requires decimal or exponent
    {
        REQUIRE(is_float("123.45"));
        REQUIRE(is_float("-123.45"));
        REQUIRE(is_float("+123.45"));
        REQUIRE(is_float("0.0"));
        REQUIRE(is_float(".5"));
        REQUIRE(is_float("-.5"));
        REQUIRE(is_float("5."));
        REQUIRE(is_float("1e5"));
        REQUIRE(is_float("1.23e+4"));
        REQUIRE(is_float("-1.23E-4"));
        REQUIRE_FALSE(is_float("123")); // No decimal/exponent
        REQUIRE_FALSE(is_float("-123"));
        REQUIRE_FALSE(is_float("123a"));
        REQUIRE_FALSE(is_float("abc"));
        REQUIRE_FALSE(is_float(""));
        REQUIRE_FALSE(is_float("."));
        REQUIRE_FALSE(is_float("e5"));
        REQUIRE_FALSE(is_float("1e"));
        REQUIRE_FALSE(is_float("1e+"));
        REQUIRE_FALSE(is_float("1.2.3"));
        REQUIRE_FALSE(is_float("1e5.5"));
        REQUIRE_FALSE(is_float(" 1.0")); // whitespace disallowed by assumption
    }

    SECTION("is_numeric") // Integer or Float
    {
        // Integers
        REQUIRE(is_numeric("123"));
        REQUIRE(is_numeric("-123"));
        REQUIRE(is_numeric("+123"));
        REQUIRE(is_numeric("0"));
        // Floats
        REQUIRE(is_numeric("123.45"));
        REQUIRE(is_numeric("-123.45"));
        REQUIRE(is_numeric("+123.45"));
        REQUIRE(is_numeric("0.0"));
        REQUIRE(is_numeric(".5"));
        REQUIRE(is_numeric("-.5"));
        REQUIRE(is_numeric("5."));
        REQUIRE(is_numeric("1e5"));
        REQUIRE(is_numeric("1.23e+4"));
        REQUIRE(is_numeric("-1.23E-4"));
        // Invalid
        REQUIRE_FALSE(is_numeric("123a"));
        REQUIRE_FALSE(is_numeric("abc"));
        REQUIRE_FALSE(is_numeric(""));
        REQUIRE_FALSE(is_numeric("+"));
        REQUIRE_FALSE(is_numeric("-"));
        REQUIRE_FALSE(is_numeric("."));
        REQUIRE_FALSE(is_numeric("e5"));
        REQUIRE_FALSE(is_numeric("1e"));
        REQUIRE_FALSE(is_numeric("1e+"));
        REQUIRE_FALSE(is_numeric("1.2.3"));
        REQUIRE_FALSE(is_numeric("1e5.5"));
        REQUIRE_FALSE(is_numeric(" 123")); // whitespace disallowed by assumption
    }
}

TEST_CASE("String Replacement Functions", "[string][replace][remove]")
{
    SECTION("replace")
    {
        REQUIRE(replace("hello world", "world", "there") == "hello there");
        REQUIRE(replace("ababab", "ab", "x", 2) == "xxab");
        REQUIRE(replace("ababab", "ab", "x") == "xxx"); // Default count is max
        REQUIRE(replace("hello", "l", "L", 1) == "heLlo");
        REQUIRE(replace("hello", "l", "LL", 2) == "heLLLLo");
        REQUIRE(replace("hello", "x", "y") == "hello"); // Not found
        REQUIRE(replace("hello", "", "-", 3) == "-h-e-llo"); // Empty old_value
        REQUIRE(replace("", "a", "b") == "");
        REQUIRE(replace("aaa", "a", "aa") == "aaaaaa");
        REQUIRE(replace("aaaa", "aa", "a") == "aa");
    }

    SECTION("replace_all")
    {
        REQUIRE(replace_all("hello world world", "world", "there") == "hello there there");
        REQUIRE(replace_all("ababab", "ab", "x") == "xxx");
        REQUIRE(replace_all("hello", "l", "L") == "heLLo");
        REQUIRE(replace_all("hello", "x", "y") == "hello");
        REQUIRE(replace_all("aaa", "a", "aa") == "aaaaaa");
        REQUIRE(replace_all("aaaa", "aa", "a") == "aa");
    }

    SECTION("replace_by_nth")
    {
        REQUIRE(replace_by_nth("one two one three one", "one", "FIRST", 1) == "FIRST two one three one");
        REQUIRE(replace_by_nth("one two one three one", "one", "SECOND", 2) == "one two SECOND three one");
        REQUIRE(replace_by_nth("one two one three one", "one", "THIRD", 3) == "one two one three THIRD");
        REQUIRE(replace_by_nth("one two one three one", "one", "FOURTH", 4) == "one two one three one"); // N too large
        REQUIRE(replace_by_nth("one two one three one", "two", "SECOND", 1) == "one SECOND one three one");
        REQUIRE(replace_by_nth("hello", "l", "L", 1) == "heLlo");
        REQUIRE(replace_by_nth("hello", "l", "L", 2) == "helLo");
        REQUIRE(replace_by_nth("hello", "l", "L", 3) == "hello");
        REQUIRE(replace_by_nth("hello", "x", "y", 1) == "hello"); // Not found
        REQUIRE(replace_by_nth("hello", "", "y", 1) == "hello"); // Empty old_value
        REQUIRE(replace_by_nth("hello", "l", "L", 0) == "hello"); // Invalid n
    }

    SECTION("remove")
    {
        REQUIRE(remove("hello world", "l", 2) == "heo world");
        REQUIRE(remove("hello world", "l") == "heo word"); // Default count is max
        REQUIRE(remove("hello world", " ") == "helloworld");
        REQUIRE(remove("ababab", "ab", 1) == "abab");
        REQUIRE(remove("hello", "x") == "hello");
        REQUIRE(remove("", "a") == "");
    }

    SECTION("remove_all (string)")
    {
        REQUIRE(remove_all("hello world world", "world") == "hello  "); // Note space remains
        REQUIRE(remove_all("ababab", "ab") == "");
        REQUIRE(remove_all("hello", "l") == "heo");
        REQUIRE(remove_all("hello", "x") == "hello");
    }

    SECTION("remove_all (char)")
    {
        REQUIRE(remove_all("hello world", 'l') == "heo word");
        REQUIRE(remove_all("hello world", ' ') == "helloworld");
        REQUIRE(remove_all("aaaaa", 'a') == "");
        REQUIRE(remove_all("hello", 'x') == "hello");
    }

    SECTION("remove_nth")
    {
        REQUIRE(remove_nth("one two one three one", "one", 1) == " two one three one");
        REQUIRE(remove_nth("one two one three one", "one", 2) == "one two  three one");
        REQUIRE(remove_nth("one two one three one", "one", 3) == "one two one three ");
        REQUIRE(remove_nth("one two one three one", "one", 4) == "one two one three one");
        REQUIRE(remove_nth("hello", "l", 1) == "helo");
        REQUIRE(remove_nth("hello", "l", 2) == "helo");
        REQUIRE(remove_nth("hello", "l", 3) == "hello");
        REQUIRE(remove_nth("hello", "x", 1) == "hello");
        REQUIRE(remove_nth("hello", "", 1) == "hello");
        REQUIRE(remove_nth("hello", "l", 0) == "hello");
    }
}

TEST_CASE("String Case Conversion", "[string][case]")
{
    REQUIRE(to_lower("Hello World") == "hello world");
    REQUIRE(to_lower("HELLO") == "hello");
    REQUIRE(to_lower("hello") == "hello");
    REQUIRE(to_lower("123 ABC def") == "123 abc def");
    REQUIRE(to_lower("") == "");

    REQUIRE(to_upper("Hello World") == "HELLO WORLD");
    REQUIRE(to_upper("hello") == "HELLO");
    REQUIRE(to_upper("HELLO") == "HELLO");
    REQUIRE(to_upper("123 ABC def") == "123 ABC DEF");
    REQUIRE(to_upper("") == "");
}

TEST_CASE("String Conversion to_string", "[string][conversion]")
{
    std::string_view sv = "test string";
    std::string s = to_string(sv);
    REQUIRE(s == "test string");
    REQUIRE(to_string("") == "");
}

TEST_CASE("String Padding Functions", "[string][pad]")
{
    SECTION("left_pad")
    {
        REQUIRE(left_pad("abc", 5, ' ') == "  abc");
        REQUIRE(left_pad("abc", 5, '0') == "00abc");
        REQUIRE(left_pad("abc", 3, ' ') == "abc");
        REQUIRE(left_pad("abc", 2, ' ') == "abc");
        REQUIRE(left_pad("", 3, '-') == "---");
    }

    SECTION("right_pad")
    {
        REQUIRE(right_pad("abc", 5, ' ') == "abc  ");
        REQUIRE(right_pad("abc", 5, '0') == "abc00");
        REQUIRE(right_pad("abc", 3, ' ') == "abc");
        REQUIRE(right_pad("abc", 2, ' ') == "abc");
        REQUIRE(right_pad("", 3, '-') == "---");
    }

    SECTION("pad") // Default position 0 is left
    {
        REQUIRE(pad("abc", 5, ' ', 0) == "  abc");
        REQUIRE(pad("abc", 5, ' ') == "  abc"); // Default position
        REQUIRE(pad("abc", 5, ' ', 1) == "abc  "); // Position 1 is right
        REQUIRE(pad("abc", 5, '0', 0) == "00abc");
        REQUIRE(pad("abc", 5, '0', 1) == "abc00");
        // Cases where width is not greater
        REQUIRE(pad("abc", 3) == "abc");
        REQUIRE(pad("abc", 2) == "abc");
    }
}

TEST_CASE("String Reverse Function", "[string][reverse]")
{
    REQUIRE(reverse("hello") == "olleh");
    REQUIRE(reverse("a") == "a");
    REQUIRE(reverse("") == "");
    REQUIRE(reverse("madam") == "madam");
}

TEST_CASE("String Parsing Functions", "[string][parse]")
{
    int i_val;
    double d_val;
    float f_val;

    SECTION("try_parse_int")
    {
        REQUIRE(try_parse_int("123", i_val));
        REQUIRE(i_val == 123);
        REQUIRE(try_parse_int("-456", i_val));
        REQUIRE(i_val == -456);
        REQUIRE(try_parse_int("789", i_val)); // Removed + prefix
        REQUIRE(i_val == 789);
        REQUIRE(try_parse_int("0", i_val));
        REQUIRE(i_val == 0);

        REQUIRE_FALSE(try_parse_int("123a", i_val));
        REQUIRE_FALSE(try_parse_int("12 3", i_val));
        REQUIRE_FALSE(try_parse_int("123.0", i_val));
        REQUIRE_FALSE(try_parse_int("", i_val));
        REQUIRE_FALSE(try_parse_int(" ", i_val)); // Whitespace not handled by from_chars
        REQUIRE_FALSE(try_parse_int("+", i_val));
        // Overflow/Underflow (might depend on int size, but generally large numbers)
        REQUIRE_FALSE(try_parse_int("99999999999999999999", i_val));
        REQUIRE_FALSE(try_parse_int("-99999999999999999999", i_val));
    }

    SECTION("try_parse_double")
    {
        REQUIRE(try_parse_double("123.45", d_val));
        REQUIRE(std::abs(d_val - 123.45) < std::numeric_limits<double>::epsilon());
        REQUIRE(try_parse_double("-0.5", d_val));
        REQUIRE(std::abs(d_val - (-0.5)) < std::numeric_limits<double>::epsilon());
        REQUIRE(try_parse_double("1e6", d_val)); // Removed + prefix
        REQUIRE(std::abs(d_val - 1e6) < std::numeric_limits<double>::epsilon());
        REQUIRE(try_parse_double("1.23E-4", d_val));
        // Be careful with exact float comparisons
        REQUIRE(d_val > 1.22e-4);
        REQUIRE(d_val < 1.24e-4);
        REQUIRE(try_parse_double("123", d_val)); // Integers are valid doubles
        REQUIRE(std::abs(d_val - 123.0) < std::numeric_limits<double>::epsilon());
        REQUIRE(try_parse_double("0", d_val));
        REQUIRE(std::abs(d_val - 0.0) < std::numeric_limits<double>::epsilon());
        // from_chars for float allows leading dot if digits follow
        REQUIRE(try_parse_double(".5", d_val));
        REQUIRE(std::abs(d_val - 0.5) < std::numeric_limits<double>::epsilon());

        REQUIRE_FALSE(try_parse_double("123.a", d_val));
        REQUIRE_FALSE(try_parse_double("1.2.3", d_val));
        REQUIRE_FALSE(try_parse_double("1e5.5", d_val));
        REQUIRE_FALSE(try_parse_double("", d_val));
        REQUIRE_FALSE(try_parse_double(" ", d_val));
        REQUIRE_FALSE(try_parse_double(".", d_val));
        REQUIRE_FALSE(try_parse_double("e5", d_val));
        // Note: std::from_chars might parse partially, but we require full match
        REQUIRE_FALSE(try_parse_double("123 suffix", d_val)); 
    }

     SECTION("try_parse_float")
    {
        REQUIRE(try_parse_float("12.5", f_val));
        REQUIRE(std::abs(f_val - 12.5f) < std::numeric_limits<float>::epsilon());
        REQUIRE(try_parse_float("-0.25", f_val));
        REQUIRE(std::abs(f_val - (-0.25f)) < std::numeric_limits<float>::epsilon());
        REQUIRE(try_parse_float("1e3", f_val)); // Removed + prefix
        REQUIRE(std::abs(f_val - 1e3f) < std::numeric_limits<float>::epsilon());
        REQUIRE(try_parse_float("456", f_val)); // Integer
        REQUIRE(std::abs(f_val - 456.0f) < std::numeric_limits<float>::epsilon());

        REQUIRE_FALSE(try_parse_float("12.a", f_val));
        REQUIRE_FALSE(try_parse_float("", f_val));
        REQUIRE_FALSE(try_parse_float(" ", f_val));
        REQUIRE_FALSE(try_parse_float("123 suffix", f_val));
    }
}
