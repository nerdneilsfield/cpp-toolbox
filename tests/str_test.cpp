#include "cpp_toolbox.hpp"

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in
#include "catch.hpp"

using namespace cpp_toolbox;

TEST_CASE("test_cpp_toolbox", "[cpp_toolbox]") {
  SECTION("ToString and FromString") {
    REQUIRE(str::ToString(1) == "1");
    REQUIRE(str::ToString(1.0) == "1");
    REQUIRE(str::ToString(1.0f) == "1");
    REQUIRE(str::ToString(1.0l) == "1");
    REQUIRE(str::ToString(1ll) == "1");
    REQUIRE(str::ToString(1ul) == "1");
    REQUIRE(str::ToString(1ull) == "1");
    REQUIRE(str::ToString(1u) == "1");
    REQUIRE(str::FromString<float>("1.0") == 1.0f);
    REQUIRE(str::FromString<double>("1.0") == 1.0);
    REQUIRE(str::FromString<int>("1") == 1);
    REQUIRE(str::FromString<long>("12345678") == 12345678);
    REQUIRE(str::FromString<long long>("1234567890") == 1234567890);
    REQUIRE(str::FromString<std::string>("s s") == "s s");
  }

  SECTION("ToLower, ToUpper") {
    REQUIRE(str::StrToLower("ABCDEFG") == "abcdefg");
    REQUIRE(str::StrToUpper("abcdefg") == "ABCDEFG");
  }

  SECTION("Split") {
    auto result = str::StrSplit("a,b,c", ",");
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == "a");
    REQUIRE(result[1] == "b");
    REQUIRE(result[2] == "c");

    auto result2 = str::Split<u32>(13014015u, 0);
    REQUIRE(result2.size() == 3);
    REQUIRE(result2[0] == 13);
    REQUIRE(result2[1] == 14);
    REQUIRE(result2[2] == 15);
  }

  SECTION("CONTAINS") {
    REQUIRE(str::StrContains("abcdefg", "abc"));
    REQUIRE(str::StrContains("abcdefg", "def"));
    REQUIRE(str::StrContains("abcdefg", "efg"));
    REQUIRE(str::StrContains("abcdefg", "abcdefg"));
    REQUIRE(!str::StrContains("abcdefg", "abcdefg1"));
    REQUIRE(!str::StrContains("abcdefg", "1abcdefg"));

    REQUIRE(str::Contains<float>(1.23456f, "23456"));
    REQUIRE(str::Contains<float>(1.23456f, "1.23456"));
    REQUIRE(!str::Contains<float>(1.23456f, "1.23456f"));
  }

  SECTION("EndsWith, StartsWith") {
    REQUIRE(str::StrEndsWith("abcdefg", "efg"));
    REQUIRE(str::StrEndsWith("abcdefg", "fg"));
    REQUIRE(str::StrEndsWith("abcdefg", "g"));
    REQUIRE(str::StrEndsWith("abcdefg", "abcdefg"));
    REQUIRE(!str::StrEndsWith("abcdefg", "abcdefg1"));
    REQUIRE(!str::StrEndsWith("abcdefg", "1abcdefg"));

    REQUIRE(str::StrStartsWith("abcdefg", "ab"));
    REQUIRE(str::StrStartsWith("abcdefg", "abc"));
    REQUIRE(str::StrStartsWith("abcdefg", "abcd"));
    REQUIRE(!str::StrStartsWith("abcdefg", "abce"));

    REQUIRE(str::StartsWith<double>(1.23456, "1."));
    REQUIRE(str::StartsWith<double>(1.23456, "1.23456"));
    REQUIRE(!str::StartsWith<double>(1.23456, "1.23456f"));
    REQUIRE(!str::StartsWith<double>(1.23456, "1.23456g"));

    REQUIRE(str::EndsWith<double>(1.23456, "56"));
    REQUIRE(str::EndsWith<double>(1.23456, "1.23456"));
    REQUIRE(!str::EndsWith<double>(1.23456, "1.23456f"));
  }

  SECTION("Join") {
    std::vector<std::string> vec{"a", "b", "c"};
    REQUIRE(str::StrJoin(vec, ",") == "a,b,c");
    REQUIRE(str::StrJoin(vec, ", ") == "a, b, c");
    REQUIRE(str::StrJoin(vec, "") == "abc");
    REQUIRE(str::StrJoin(vec, "  ") == "a  b  c");

    std::vector<u32> vec2{1, 2, 3};
    REQUIRE(str::Join<u32, std::string>(vec2, ",") == "1,2,3");

    std::vector<float> vec3{1.0f, 2.0f, 3.0f};
    REQUIRE(str::Join<float, std::string>(vec3, ",") == "1,2,3");

    std::vector<double> vec4{1.1, 2.1, 3.1};
    REQUIRE(str::Join<double, std::string>(vec4, ",") == "1.1,2.1,3.1");

    std::vector<long> vec5{1, 2, 3};
    REQUIRE(str::Join<long, i32>(vec5, "0") == 10203);
  }

  SECTION("Trim") {
    REQUIRE(str::StrTrim("  abc  ") == "abc");

    REQUIRE(str::StrTrimLeft("  abc  ") == "abc  ");
    REQUIRE(str::StrTrimLeft("abc  ") == "abc  ");

    REQUIRE(str::StrTrimRight("  abc  ") == "  abc");
    REQUIRE(str::StrTrimRight("  abc") == "  abc");
  }

  SECTION("Replace") {
    REQUIRE(str::StrReplace("abc", "a", "b") == "bbc");
    REQUIRE(str::StrReplace("abc", "b", " ") == "a c");
    
    REQUIRE(str::Replace<u32>(123, 1, 2) == 223);
    REQUIRE(str::Replace<f32>(123.0f, 1.0f, 2.0f) == 223.0f);

    REQUIRE(str::StrReplaceAll("aabc", "a", "d") == "ddbc");
    REQUIRE(str::StrReplaceAll("aabc", "b", "d") == "aadc");

    REQUIRE(str::ReplaceAll<u64>(12234l, 2l, 5l) == 15534l);
  }

  SECTION("StrErase") {
    REQUIRE(str::StrErase("abc", "a") == "bc");
    REQUIRE(str::StrErase("abc", "b") == "ac");
    REQUIRE(str::StrErase("abc", "c") == "ab");
    REQUIRE(str::StrErase("abc", "d") == "abc");

    REQUIRE(str::Erase<int>(12345, 5) == 1234);
    REQUIRE(str::Erase<int>(12345, 1) == 2345);

    REQUIRE(str::StrEraseAll("aabc", "a") == "bc");
    REQUIRE(str::StrEraseAll("aabc", "b") == "aac");

    REQUIRE(str::EraseAll<u64>(12234l, 2l) == 134l);
  }

  SECTION("Regex") {
    REQUIRE(str::StrFindRegex("abcd12345678xxs", "\\d+") == "12345678");
    REQUIRE(str::StrFindRegex("abcd 12345678 xx s", "\\w+") == "abcd");

    REQUIRE(str::StrFindAllRegex("abcd12345678xxs", "\\d+") == std::vector<std::string>{"12345678"});
    REQUIRE(str::StrFindAllRegex("abcd 12345678 xx s", "\\w+") == std::vector<std::string>{"abcd", "12345678", "xx", "s"});

    REQUIRE(str::StrReplaceAllRegex("abcd12345678xxs", "\\d+", "123") == "abcd123xxs");
    REQUIRE(str::StrReplaceAllRegex("abcd 12345678 xx s", "\\w+", "123") == "123 123 123 123");

    REQUIRE(str::StrEraseAllRegex("abcd12345678xxs", "\\d+") == "abcdxxs");
    REQUIRE(str::StrEraseAllRegex("abcd 12345678 xx s", "\\w+") == "   ");

    REQUIRE(str::FindRegex<std::string, u32>("abcd12345678xxs", "\\d+") == 12345678);
    REQUIRE(str::FindRegex<std::string, u32>("abcd 12345678 xx s", "\\d+") == 12345678);
    REQUIRE(str::FindAllRegex<std::string, u32>("abcd12345678xxs1234", "\\d+") == std::vector<u32>{12345678, 1234});
  }
}