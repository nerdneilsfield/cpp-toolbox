#ifndef _CPP_TOOLBOX_H_
#define _CPP_TOOLBOX_H_

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <iterator>
#include <locale>
#include <memory>
#include <mutex>
#include <queue>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#define FUNCTION __FUNCSIG__
#else
#define FUNCTION __PRETTY_FUNCTION__
#endif

#if __GNUC__ >= 3
#define NORETURN __attribute__((noreturn))
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else
#define NORETURN /* nothing */
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define CACHELINE_SIZE 64

#define UNUSED(x) (void)(x)
#define UNUSED_RETURN(x) (void)(x)
#define UNUSED_RETURN_VOID(x) (void)(x)
#define UNUSED_VOID(x) (void)(x)
#define UNUSED_VOID_RETURN(x) (void)(x)
#define UNUSED_VOID_RETURN_VOID(x) (void)(x)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define DEFINE_TYPE_TRAIT(name, func)                                          \
  template <typename T> struct name {                                          \
    template <typename Class>                                                  \
    static constexpr bool Test(decltype(&Class::func) *) {                     \
      return true;                                                             \
    }                                                                          \
    template <typename> static constexpr bool Test(...) { return false; }      \
                                                                               \
    static constexpr bool value = Test<T>(nullptr);                            \
  };                                                                           \
                                                                               \
  template <typename T> constexpr bool name<T>::value;

// log macros
#define LOG_ERROR(...)                                                         \
  cpp_toolbox::logger::Logger::GetInstance().VError(                           \
      __FILE__, __LINE__, static_cast<const char *>(FUNCTION), __VA_ARGS__)
#define LOG_WARNING(...)                                                       \
  cpp_toolbox::logger::Logger::GetInstance().VWarning(                         \
      __FILE__, __LINE__, static_cast<const char *>(FUNCTION), __VA_ARGS__)

#define LOG_INFO(...)                                                          \
  cpp_toolbox::logger::Logger::GetInstance().VInfo(                            \
      __FILE__, __LINE__, static_cast<const char *>(FUNCTION), __VA_ARGS__)
#define LOG_DEBUG(...)                                                         \
  cpp_toolbox::logger::Logger::GetInstance().VDebug(                           \
      __FILE__, __LINE__, static_cast<const char *>(FUNCTION), __VA_ARGS__)

// type short
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

static_assert(sizeof(i8) == 1, "size of i8 is 1 byte");
static_assert(sizeof(i16) == 2, "size of i16 is 2 bytes");
static_assert(sizeof(i32) == 4, "size of i32 is 4 bytes");
static_assert(sizeof(i64) == 8, "size of i64 is 8 bytes");
static_assert(sizeof(u8) == 1, "size of u8 is 1 byte");
static_assert(sizeof(u16) == 2, "size of u16 is 2 bytes");
static_assert(sizeof(u32) == 4, "size of u32 is 4 bytes");
static_assert(sizeof(u64) == 8, "size of u64 is 8 bytes");
static_assert(sizeof(f32) == 4, "size of f32 is 4 bytes");
static_assert(sizeof(f64) == 8, "size of f64 is 8 bytes");

using ai8 = std::atomic<i8>;
using ai16 = std::atomic<i16>;
using ai32 = std::atomic<i32>;
using ai64 = std::atomic<i64>;
using au8 = std::atomic<u8>;
using au16 = std::atomic<u16>;
using au32 = std::atomic<u32>;
using au64 = std::atomic<u64>;
using abool = std::atomic<bool>;

namespace cpp_toolbox {

namespace compile {
template <typename T> struct is_const { static constexpr bool value = false; };

template <typename T> struct is_const<const T> {
  static constexpr bool value = true;
};

template <typename T> constexpr bool is_const_v = is_const<T>::value;
template <typename T> struct is_volatile {
  static constexpr bool value = false;
};

template <typename T> struct is_volatile<volatile T> {
  static constexpr bool value = true;
};
template <typename T> constexpr bool is_volatile_v = is_volatile<T>::value;

template <typename T> struct is_const_volatile {
  static constexpr bool value = false;
};
template <typename T> struct is_const_volatile<const volatile T> {
  static constexpr bool value = true;
};
template <typename T>
constexpr bool is_const_volatile_v = is_const_volatile<T>::value;

template <typename T> struct is_pointer {
  static constexpr bool value = false;
};

template <typename T> struct is_pointer<T *> {
  static constexpr bool value = true;
};
template <typename T> constexpr bool is_pointer_v = is_pointer<T>::value;

template <typename T> struct is_reference {
  static constexpr bool value = false;
};

template <typename T> struct is_reference<T &> {
  static constexpr bool value = true;
};
template <typename T> constexpr bool is_reference_v = is_reference<T>::value;

template <typename T> struct is_rvalue_reference {
  static constexpr bool value = false;
};
template <typename T> struct is_rvalue_reference<T &&> {
  static constexpr bool value = true;
};
template <typename T>
constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

template <typename T> struct is_void {
  static constexpr bool value = false;
};

template <> struct is_void<void> { static constexpr bool value = true; };
template <typename T> constexpr bool is_void_v = is_void<T>::value;

template <typename T> struct is_array {
  static constexpr bool value = false;
};

template <typename T, size_t N> struct is_array<T[N]> {
  static constexpr bool value = true;
};
template <typename T> constexpr bool is_array_v = is_array<T>::value;


} // namespace compile

namespace console {

enum class Color {
  Black = 30,
  Red = 31,
  Green = 32,
  Yellow = 33,
  Blue = 34,
  Magenta = 35,
  Cyan = 36,
  White = 37,
};

enum class Style {
  Normal = 0,
  Bold = 1,
  Dim = 2,
  Underlined = 3,
  Blink = 4,
  Reverse = 7,
  Hidden = 8,
};

enum class BackgroundColor {
  Black = 40,
  Red = 41,
  Green = 42,
  Yellow = 43,
  Blue = 44,
  Magenta = 45,
  Cyan = 46,
  White = 47,
};

enum class TextAlignment {
  Left = 0,
  Center = 1,
  Right = 2,
};

inline void SetColor(Color color) {
  std::cout << "\033[" << static_cast<int>(color) << "m";
  std::cout.flush();
  return;
}

inline void SetBackgroundColor(BackgroundColor background_color) {
  std::cout << "\033[" << static_cast<int>(background_color) << "m";
  std::cout.flush();
  return;
}

inline void SetStyle(Style style) {
  std::cout << "\033[" << static_cast<int>(style) << "m";
  std::cout.flush();
  return;
}

inline void ResetColor() {
  std::cout << "\033[0m";
  std::cout.flush();
  return;
}

} // namespace console

namespace logger {
enum class VerbosityLevel {
  Error = 0,
  Warning = 1,
  Info = 2,
  Debug = 3,
};

class Logger {
public:
  Logger() = default;
  Logger(Logger const &) = delete;
  void operator=(Logger const &) = delete;
  static Logger &GetInstance() {
    static Logger instance;
    return instance;
  }

  void SetVerbosityLevel(VerbosityLevel verbosity_level) {
    verbosity_level_ = verbosity_level;
  }

  VerbosityLevel GetVerbosityLevel() const { return verbosity_level_; }

  template <typename... Args>
  void VError(const char *file, int line, const char *function, Args... args) {
    if (verbosity_level_ >= VerbosityLevel::Error) {
      console::SetColor(console::Color::Red);
      std::cout << "[ERROR] ";
      std::cout << "@" << file << ":" << line << ":" << function << "->";
      for (const auto &arg : {args...}) {
        std::cout << arg;
      }
      std::cout << std::endl;
      console::ResetColor();
    }
  }

  template <typename... Args>
  void VWarning(const char *file, int line, const char *function,
                Args... args) {
    if (verbosity_level_ >= VerbosityLevel::Warning) {
      console::SetColor(console::Color::Yellow);
      std::cout << "[WARNING] ";
      std::cout << "@" << file << ":" << line << ":" << function << "->";
      for (const auto &arg : {args...}) {
        std::cout << arg;
      }
      std::cout << std::endl;
      console::ResetColor();
    }
  }

  template <typename... Args>
  void VInfo(const char *file, int line, const char *function, Args... args) {
    if (verbosity_level_ >= VerbosityLevel::Info) {
      console::SetColor(console::Color::Green);
      std::cout << "[INFO] ";
      std::cout << "@" << file << ":" << line << ":" << function << "->";
      for (const auto &arg : {args...}) {
        std::cout << arg;
      }
      std::cout << std::endl;
      console::ResetColor();
    }
  }

  template <typename... Args>
  void VDebug(const char *file, int line, const char *function, Args... args) {
    if (verbosity_level_ >= VerbosityLevel::Debug) {
      console::SetColor(console::Color::Blue);
      std::cout << "[DEBUG] ";
      std::cout << "@" << file << ":" << line << ":" << function << "->";
      for (const auto &arg : {args...}) {
        std::cout << arg;
      }
      std::cout << std::endl;
      console::ResetColor();
    }
  }

private:
  VerbosityLevel verbosity_level_ = VerbosityLevel::Info;
};

inline void SetLoggerVerbosityLevel(VerbosityLevel verbosity_level) {
  Logger::GetInstance().SetVerbosityLevel(verbosity_level);
  return;
}

inline VerbosityLevel GetLoggerVerbosityLevel() {
  return Logger::GetInstance().GetVerbosityLevel();
}

} // namespace logger

namespace str {
template <typename T> std::string ToString(const T &value) {
  std::ostringstream ss;
  ss << value;
  return ss.str();
}

template <typename T> T FromString(const std::string &str) {
  T value;
  std::istringstream ss(str);
  ss >> value;
  return value;
}

template <> inline std::string FromString<std::string>(const std::string &str) {
  return str;
}

template <typename T> std::string ToLower(const T &value) {
  std::string str = ToString(value);
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
}

template <typename T> std::string ToUpper(const T &value) {
  std::string str = ToString(value);
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
  return str;
}

inline std::string StrToLower(const std::string &str) {
  std::string str_lower = str;
  std::transform(str_lower.begin(), str_lower.end(), str_lower.begin(),
                 ::tolower);
  return str_lower;
}

inline std::string StrToUpper(const std::string &str) {
  std::string str_upper = str;
  std::transform(str_upper.begin(), str_upper.end(), str_upper.begin(),
                 ::toupper);
  return str_upper;
}

inline bool StrStartsWith(const std::string &value, const std::string &prefix) {
  return value.rfind(prefix, 0) == 0;
}

inline bool StrEndsWith(const std::string &value, const std::string &suffix) {
  return value.size() >= suffix.size() &&
         value.compare(value.size() - suffix.size(), suffix.size(), suffix) ==
             0;
}

inline bool StrContains(const std::string &value, const std::string &sub_str) {
  return value.find(sub_str) != std::string::npos;
}

template <typename T> bool Contains(const T &value, const std::string &str) {
  return StrContains(ToString(value), str);
}

template <typename T> bool StartsWith(const T &value, const std::string &str) {
  return StrStartsWith(ToString(value), str);
}

template <typename T> bool EndsWith(const T &value, const std::string &str) {
  return StrEndsWith(ToString(value), str);
}

inline std::vector<std::string> StrSplit(const std::string &value,
                                         const std::string &separator) {
  std::vector<std::string> result;
  std::string::size_type pos1, pos2;
  pos2 = value.find(separator);
  pos1 = 0;
  while (std::string::npos != pos2) {
    result.push_back(value.substr(pos1, pos2 - pos1));
    pos1 = pos2 + separator.size();
    pos2 = value.find(separator, pos1);
  }
  if (pos1 != value.length()) {
    result.push_back(value.substr(pos1));
  }
  return result;
}

template <typename T> std::vector<T> Split(const T &value, const T &separator) {
  std::vector<T> result;
  auto str = ToString(value);
  auto sep = ToString(separator);
  auto str_result = StrSplit(str, sep);
  for (const auto &str : str_result) {
    result.push_back(FromString<T>(str));
  }
  return result;
}

template <typename T, typename O>
O Join(std::initializer_list<T> list, const std::string &separator) {
  std::ostringstream ss;
  for (auto it = list.begin(); it != list.end(); ++it) {
    if (it != list.begin()) {
      ss << separator;
    }
    ss << *it;
  }
  return FromString<O>(ss.str());
}

template <typename T, typename O>
O Join(std::vector<T> &list, const std::string &separator) {
  std::ostringstream ss;
  for (auto it = list.begin(); it != list.end(); ++it) {
    if (it != list.begin()) {
      ss << separator;
    }
    ss << *it;
  }
  // LOG_INFO(ss.str());
  return FromString<O>(ss.str());
}

inline std::string StrJoin(std::vector<std::string> &values,
                           const std::string &separator) {
  return Join<std::string, std::string>(values, separator);
}

inline std::string StrJoin(std::initializer_list<const char *> &values,
                           const std::string &separator) {
  return Join<const char *, std::string>(values, separator);
}

inline std::string StrJoin(std::vector<int> &values,
                           const std::string &separator) {
  return Join<int, std::string>(values, separator);
}

inline std::string StrTrim(const std::string &str) {
  auto begin = str.begin();
  while (begin != str.end() && std::isspace(*begin)) {
    ++begin;
  }
  auto end = str.end();
  while (end != begin && std::isspace(*(end - 1))) {
    --end;
  }
  return std::string(begin, end);
}

inline std::string StrTrimLeft(const std::string &str) {
  auto begin = str.begin();
  while (begin != str.end() && std::isspace(*begin)) {
    ++begin;
  }
  return std::string(begin, str.end());
}

inline std::string StrTrimRight(const std::string &str) {
  auto end = str.end();
  while (end != str.begin() && std::isspace(*(end - 1))) {
    --end;
  }
  return std::string(str.begin(), end);
}

template <typename T> T Trim(const T &str) {
  return FromString<T>(StrTrim(ToString(str)));
}

inline std::string StrReplace(const std::string &str, const std::string &from,
                              const std::string &to) {
  std::string result = str;
  size_t start_pos = result.find(from);
  if (start_pos == std::string::npos) {
    return result;
  }
  result.replace(start_pos, from.length(), to);
  return result;
}

inline std::string StrReplaceAll(const std::string &str,
                                 const std::string &from,
                                 const std::string &to) {
  std::string result = str;
  if (from.empty()) {
    return result;
  }
  size_t start_pos = 0;
  while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
    result.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
  return result;
}

template <typename T> T Replace(const T &value, const T &from, const T &to) {
  return FromString<T>(
      StrReplace(ToString(value), ToString(from), ToString(to)));
}

template <typename T> T ReplaceAll(const T &value, const T &from, const T &to) {
  return FromString<T>(
      StrReplaceAll(ToString(value), ToString(from), ToString(to)));
}

inline std::string StrErase(const std::string &str, const std::string &erase) {
  std::string result = str;
  size_t start_pos = result.find(erase);
  if (start_pos == std::string::npos) {
    return result;
  }
  result.erase(start_pos, erase.length());
  return result;
}

template <typename T> T Erase(const T &value, const T &erase) {
  return FromString<T>(StrErase(ToString(value), ToString(erase)));
}

inline std::string StrEraseAll(const std::string &str,
                               const std::string &erase) {
  std::string result = str;
  if (erase.empty()) {
    return result;
  }
  size_t start_pos = 0;
  while ((start_pos = result.find(erase, start_pos)) != std::string::npos) {
    result.erase(start_pos, erase.length());
  }
  return result;
}

template <typename T> T EraseAll(const T &value, const T &erase) {
  return FromString<T>(StrEraseAll(ToString(value), ToString(erase)));
}

// find a regex file in a string
inline std::string StrFindRegex(const std::string &str,
                                const std::string &regex) {
  std::regex re(regex);
  std::smatch match;
  if (std::regex_search(str, match, re)) {
    return match.str();
  }
  return "";
}

template <typename In, typename Out>
Out FindRegex(const In &str, const std::string &regex) {
  return FromString<Out>(StrFindRegex(ToString(str), regex));
}

// find all string fitting regex in a string
inline std::vector<std::string> StrFindAllRegex(const std::string &str,
                                                const std::string &regex) {
  std::regex re(regex);
  std::smatch match;
  std::vector<std::string> result;
  std::string::const_iterator searchStart(str.cbegin());
  while (std::regex_search(searchStart, str.cend(), match, re)) {
    result.push_back(match.str());
    searchStart += match.position() + match.length();
  }
  return result;
}

template <typename In, typename Out>
std::vector<Out> FindAllRegex(const In &str, const std::string &regex) {
  std::vector<Out> result;
  for (const auto &s : StrFindAllRegex(ToString(str), regex)) {
    result.push_back(FromString<Out>(s));
  }
  return result;
}

// replace all string fitting regex in a string
inline std::string StrReplaceAllRegex(const std::string &str,
                                      const std::string &regex,
                                      const std::string &replace) {
  std::regex re(regex);
  return std::regex_replace(str, re, replace);
}

inline std::string StrEraseAllRegex(const std::string &str,
                                    const std::string &regex) {
  return StrReplaceAllRegex(str, regex, "");
}

inline bool StrMatchRegex(const std::string &str, const std::string &regex) {
  std::regex re(regex);
  return std::regex_match(str, re);
}

} // namespace str

namespace type {

// class Dtype {
//  public:
//   static const Dtype Undefined;
//   static const Dtype Float32;
//   static const Dtype Float64;
//   static const Dtype Int8;
//   static const Dtype Int16;
//   static const Dtype Int32;
//   static const Dtype Int64;
//   static const Dtype UInt8;
//   static const Dtype UInt16;
//   static const Dtype UInt32;
//   static const Dtype UInt64;
//   static const Dtype Bool;
//
//  public:
//   enum class DtypeCode {
//     Undefined,
//     Bool,
//     Int,
//     UInt,
//     Float,
//     Object,
//   }
// }

template <typename T> struct PointXY {
  T x;
  T y;
};

template <typename T> struct PPointXYZ {
  T x;
  T y;
  T z;
};

template <typename T> struct PointXYZI {
  T x;
  T y;
  T z;
  T intensity;
};

template <typename T> struct PointXYZRGB {
  T x;
  T y;
  T z;
  T r;
  T g;
  T b;
};

template <typename T> struct PointXYZRGBA {
  T x;
  T y;
  T z;
  T r;
  T g;
  T b;
  T a;
};

} // namespace type

namespace file {
// open a file to read (if exist append other create)
inline std::fstream OpenFile(const std::string &filename,
                             const std::ios_base::openmode &mode) {
  std::fstream file;
  file.open(filename, mode);
  if (!file.is_open()) {
    LOG_ERROR("Failed to open file: " + filename);
    throw std::runtime_error("Failed to open file: " + filename);
  }
  return file;
}

// delete a file
inline void DeleteFile(const std::string &filename) {
  if (std::remove(filename.c_str()) != 0) {
    LOG_ERROR("Failed to delete file: " + filename);
    throw std::runtime_error("Failed to delete file: " + filename);
  }
}

inline bool FileExist(const std::string &filename) {
  std::ifstream file(filename);
  return file.good();
}

inline std::string GetFileExtension(const std::string &filename) {
  std::string::size_type idx;
  idx = filename.rfind('.');
  if (idx != std::string::npos) {
    return filename.substr(idx + 1);
  }
  return "";
}

inline std::string GetFileName(const std::string &filename) {
  std::string::size_type idx;
  idx = filename.rfind('/');
  if (idx != std::string::npos) {
    return filename.substr(idx + 1);
  }
  return filename;
}

inline std::string GetFileNameWithoutExtension(const std::string &filename) {
  std::string::size_type idx;
  idx = filename.rfind('.');
  if (idx != std::string::npos) {
    return filename.substr(0, idx);
  }
  return filename;
}

inline std::string GetFileDirectory(const std::string &filename) {
  std::string::size_type idx;
  idx = filename.rfind('/');
  if (idx != std::string::npos) {
    return filename.substr(0, idx);
  }
  return "";
}

inline std::string GetFileDirectoryWithSlash(const std::string &filename) {
  std::string::size_type idx;
  idx = filename.rfind('/');
  if (idx != std::string::npos) {
    return filename.substr(0, idx + 1);
  }
  return "";
}

inline std::size_t GetFileSize(const std::string &filename) {
  return std::filesystem::file_size(filename);
}

inline bool GetContent(const std::string &filename, std::string &content) {
  std::ifstream file(filename);
  if (!file.good()) {
    LOG_ERROR("Failed to open file: " + filename);
    return false;
  }
  content.assign((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());
  return true;
}

inline bool PathExists(const std::string &path) {
  return std::filesystem::exists(path);
}

inline bool IsDirectory(const std::string &path) {
  return std::filesystem::is_directory(path);
}

inline bool IsRegularFile(const std::string &path) {
  return std::filesystem::is_regular_file(path);
}

inline bool CreateDirectory(const std::string &path) {
  return std::filesystem::create_directory(path);
}

inline bool CreateDirectories(const std::string &path) {
  return std::filesystem::create_directories(path);
}

inline bool DeleteDirectory(const std::string &path) {
  return std::filesystem::remove(path);
}

inline bool DeleteDirectories(const std::string &path) {
  return std::filesystem::remove_all(path);
}

inline bool CopyFile(const std::string &from, const std::string &to) {
  return std::filesystem::copy_file(from, to);
}

inline bool CopyDirectory(const std::string &from, const std::string &to) {
  if (std::filesystem::exists(to)) {
    LOG_ERROR("Failed to copy directory: " + from + " to " + to +
              " because the destination directory already exists");
    return false;
  }
  if (std::filesystem::exists(from)) {
    LOG_ERROR("Failed to copy directory: " + from + " to " + to +
              " because the source directory does not exist");
    return false;
  }
  try {
    std::filesystem::copy(from, to, std::filesystem::copy_options::recursive);
  } catch (const std::filesystem::filesystem_error &e) {
    LOG_ERROR("Failed to copy directory: " + from + " to " + to + " because " +
              e.what());
    return false;
  }
  return true;
}

inline bool MoveFile(const std::string &from, const std::string &to) {
  if (std::filesystem::exists(to)) {
    LOG_ERROR("Failed to move file: " + from + " to " + to +
              " because the destination file already exists");
    return false;
  }
  if (std::filesystem::exists(from)) {
    LOG_ERROR("Failed to move file: " + from + " to " + to +
              " because the source file does not exist");
    return false;
  }
  try {
    std::filesystem::rename(from, to);
  } catch (const std::filesystem::filesystem_error &e) {
    LOG_ERROR("Failed to move file: " + from + " to " + to + " because " +
              e.what());
    return false;
  }
  return true;
}

inline bool MoveDirectory(const std::string &from, const std::string &to) {
  if (std::filesystem::exists(to)) {
    LOG_ERROR("Failed to move directory: " + from + " to " + to +
              " because the destination directory already exists");
    return false;
  }
  if (std::filesystem::exists(from)) {
    LOG_ERROR("Failed to move directory: " + from + " to " + to +
              " because the source directory does not exist");
    return false;
  }

  try {
    std::filesystem::rename(from, to);
  } catch (const std::filesystem::filesystem_error &e) {
    LOG_ERROR("Failed to move directory: " + from + " to " + to + " because " +
              e.what());
    return false;
  }
  return true;
}

inline bool RenameFile(const std::string &from, const std::string &to) {
  if (std::filesystem::exists(to)) {
    LOG_ERROR("Failed to rename file: " + from + " to " + to +
              " because the destination file already exists");
    return false;
  }
  if (std::filesystem::exists(from)) {
    LOG_ERROR("Failed to rename file: " + from + " to " + to +
              " because the source file does not exist");
    return false;
  }
  try {
    std::filesystem::rename(from, to);
  } catch (const std::filesystem::filesystem_error &e) {
    LOG_ERROR("Failed to rename file: " + from + " to " + to + " because " +
              e.what());
    return false;
  }
  return true;
}

inline std::size_t GetSubDirectoryCount(const std::string &path) {
  std::size_t count = 0;
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (std::filesystem::is_directory(entry)) {
      count++;
    }
  }
  return count;
}

inline std::size_t GetDirectoryFileCount(const std::string &path) {
  std::size_t size = 0;
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (std::filesystem::is_regular_file(entry)) {
      size++;
    }
  }
  return size;
}

inline std::size_t GetDirectoryCount(const std::string &path) {
  if (!std::filesystem::exists(path)) {
    LOG_ERROR(
        "Failed to get directory count because the directory does not exist: " +
        path);
    return 0;
  }
  std::size_t size = 0;
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (std::filesystem::is_directory(entry)) {
      size++;
    } else if (std::filesystem::is_regular_file(entry)) {
      size++;
    }
  }
  return size;
}

inline std::size_t GetDirectorySize(const std::string &path) {
  std::size_t size = 0;
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (std::filesystem::is_regular_file(entry)) {
      size += std::filesystem::file_size(entry);
    } else if (std::filesystem::is_directory(entry)) {
      size += GetDirectorySize(entry.path());
    }
  }
  return size;
}

inline bool IsEmptyFile(const std::string &path) {
  return GetFileSize(path) == 0;
}

inline bool IsEmptyDirectory(const std::string &path) {
  return GetDirectoryCount(path) == 0;
}

inline bool IsFileExist(const std::string &filename) {
  return std::filesystem::exists(filename);
}

inline std::vector<std::string> ListFilesInDirectory(const std::string &path) {
  std::vector<std::string> files;
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    files.push_back(entry.path());
  }
  return files;
}

inline std::vector<std::string>
ListFilesInDirectoryWithExtension(const std::string &path,
                                  const std::string &extension) {
  std::vector<std::string> files;
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (entry.path().extension() == extension) {
      files.push_back(entry.path());
    }
  }
  return files;
}

inline std::vector<std::string>
ListFilesInDirectoryWithExtensions(const std::string &path,
                                   const std::vector<std::string> &extensions) {
  std::vector<std::string> files;
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    for (const auto &extension : extensions) {
      if (entry.path().extension() == extension) {
        files.push_back(entry.path());
      }
    }
  }
  return files;
}

inline bool RemoveFilesInDirectoryWithExtension(const std::string &path,
                                                const std::string &extension) {
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (entry.path().extension() == extension) {
      DeleteFile(entry.path());
    }
  }
  return true;
}

inline bool RemoveFilesInDirectoryWithExtensions(
    const std::string &path, const std::vector<std::string> &extensions) {
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    for (const auto &extension : extensions) {
      if (entry.path().extension() == extension) {
        DeleteFile(entry.path());
      }
    }
  }
  return true;
}

} // namespace file

namespace env {
inline std::string GetEnv(const std::string &name) {
  const char *value = std::getenv(name.c_str());
  if (value == nullptr) {
    LOG_WARNING("environment variable " + name + " is not set");
    return "";
  }
  return std::string(value);
}

inline bool SetEnv(const std::string &name, const std::string &value) {
  if (setenv(name.c_str(), value.c_str(), 1) != 0) {
    LOG_ERROR("Failed to set environment variable " + name + " to " + value);
    return false;
  }
  return true;
}
} // namespace env

namespace memory {
inline void *CheckedMalloc(std::size_t size) {
  void *ptr = std::malloc(size);
  if (ptr == nullptr) {
    LOG_ERROR("Failed to allocate memory of size " + std::to_string(size));
    throw std::bad_alloc();
  }
  return ptr;
}
inline void *CheckedCalloc(std::size_t size) {
  void *ptr = std::calloc(size, 1);
  if (ptr == nullptr) {
    LOG_ERROR("Failed to allocate memory of size " + std::to_string(size));
    throw std::bad_alloc();
  }
  return ptr;
}

inline void *CheckedRealloc(void *ptr, std::size_t size) {
  void *new_ptr = std::realloc(ptr, size);
  if (new_ptr == nullptr) {
    LOG_ERROR("Failed to reallocate memory of size " + std::to_string(size));
    throw std::bad_alloc();
  }
  return new_ptr;
}

inline void CheckedFree(void *ptr) {
  if (ptr != nullptr) {
    std::free(ptr);
  }
}

inline void *AlignedMalloc(std::size_t size, std::size_t alignment) {
  void *ptr = nullptr;
  if (posix_memalign(&ptr, alignment, size) != 0) {
    LOG_ERROR("Failed to allocate aligned memory of size " +
              std::to_string(size) + " with alignment " +
              std::to_string(alignment));
    throw std::bad_alloc();
  }
  return ptr;
}

inline void AlignedFree(void *ptr) {
  if (ptr != nullptr) {
    free(ptr);
  }
}

inline void *AlignedRealloc(void *ptr, std::size_t size,
                            std::size_t alignment) {
  void *new_ptr = nullptr;
  if (posix_memalign(&new_ptr, alignment, size) != 0) {
    LOG_ERROR("Failed to allocate aligned memory of size " +
              std::to_string(size) + " with alignment " +
              std::to_string(alignment));
    throw std::bad_alloc();
  }
  if (ptr != nullptr) {
    std::memcpy(new_ptr, ptr, size);
    free(ptr);
  }
  return new_ptr;
}

inline void *AlignedZeroMalloc(std::size_t size, std::size_t alignment) {
  void *ptr = nullptr;
  if (posix_memalign(&ptr, alignment, size) != 0) {
    LOG_ERROR("Failed to allocate aligned memory of size " +
              std::to_string(size) + " with alignment " +
              std::to_string(alignment));
    throw std::bad_alloc();
  }
  std::memset(ptr, 0, size);
  return ptr;
}

inline void AlignedZeroFree(void *ptr) {
  if (ptr != nullptr) {
    free(ptr);
  }
}
} // namespace memory

namespace lower {
inline void CpuRelax() {
#if defined(__aarch64__)
  asm volatile("yield" ::: "memory");
#else
  asm volatile("rep; nop" ::: "memory");
#endif
}
} // namespace lower

namespace thread {
template <typename RWLock> class ReadLockGuard {
public:
  explicit ReadLockGuard(RWLock &lock) : rw_lock_(lock) { rw_lock_.ReadLock(); }

  ~ReadLockGuard() { rw_lock_.ReadUnlock(); }

private:
  ReadLockGuard(const ReadLockGuard &other) = delete;
  ReadLockGuard &operator=(const ReadLockGuard &other) = delete;
  RWLock &rw_lock_;
};

template <typename RWLock> class WriteLockGuard {
public:
  explicit WriteLockGuard(RWLock &lock) : rw_lock_(lock) {
    rw_lock_.WriteLock();
  }

  ~WriteLockGuard() { rw_lock_.WriteUnlock(); }

private:
  WriteLockGuard(const WriteLockGuard &other) = delete;
  WriteLockGuard &operator=(const WriteLockGuard &other) = delete;
  RWLock &rw_lock_;
};

class AtomicRWLock {
  friend class ReadLockGuard<AtomicRWLock>;
  friend class WriteLockGuard<AtomicRWLock>;

public:
  static const int32_t RW_LOCK_FREE = 0;
  static const int32_t WRITE_EXCLUSIVE = -1;
  static const uint32_t MAX_RETRY_TIMES = 5;
  AtomicRWLock() {}
  explicit AtomicRWLock(bool write_first) : write_first_(write_first) {}

private:
  // all these function only can used by ReadLockGuard/WriteLockGuard;
  void ReadLock();
  void WriteLock();

  void ReadUnlock();
  void WriteUnlock();

  AtomicRWLock(const AtomicRWLock &) = delete;
  AtomicRWLock &operator=(const AtomicRWLock &) = delete;
  std::atomic<uint32_t> write_lock_wait_num_ = {0};
  std::atomic<int32_t> lock_num_ = {0};
  bool write_first_ = true;
};

inline void AtomicRWLock::ReadLock() {
  uint32_t retry_times = 0;
  int32_t lock_num = lock_num_.load();
  if (write_first_) {
    do {
      while (lock_num < RW_LOCK_FREE || write_lock_wait_num_.load() > 0) {
        if (++retry_times == MAX_RETRY_TIMES) {
          // saving cpu
          std::this_thread::yield();
          retry_times = 0;
        }
        lock_num = lock_num_.load();
      }
    } while (!lock_num_.compare_exchange_weak(lock_num, lock_num + 1,
                                              std::memory_order_acq_rel,
                                              std::memory_order_relaxed));
  } else {
    do {
      while (lock_num < RW_LOCK_FREE) {
        if (++retry_times == MAX_RETRY_TIMES) {
          // saving cpu
          std::this_thread::yield();
          retry_times = 0;
        }
        lock_num = lock_num_.load();
      }
    } while (!lock_num_.compare_exchange_weak(lock_num, lock_num + 1,
                                              std::memory_order_acq_rel,
                                              std::memory_order_relaxed));
  }
}

inline void AtomicRWLock::WriteLock() {
  int32_t rw_lock_free = RW_LOCK_FREE;
  uint32_t retry_times = 0;
  write_lock_wait_num_.fetch_add(1);
  while (!lock_num_.compare_exchange_weak(rw_lock_free, WRITE_EXCLUSIVE,
                                          std::memory_order_acq_rel,
                                          std::memory_order_relaxed)) {
    // rw_lock_free will change after CAS fail, so init agin
    rw_lock_free = RW_LOCK_FREE;
    if (++retry_times == MAX_RETRY_TIMES) {
      // saving cpu
      std::this_thread::yield();
      retry_times = 0;
    }
  }
  write_lock_wait_num_.fetch_sub(1);
}

inline void AtomicRWLock::ReadUnlock() { lock_num_.fetch_sub(1); }

inline void AtomicRWLock::WriteUnlock() { lock_num_.fetch_add(1); }

static const std::thread::id NULL_THREAD_ID = std::thread::id();
class ReentrantRWLock {
  friend class ReadLockGuard<ReentrantRWLock>;
  friend class WriteLockGuard<ReentrantRWLock>;

public:
  static const int32_t RW_LOCK_FREE = 0;
  static const int32_t WRITE_EXCLUSIVE = -1;
  static const uint32_t MAX_RETRY_TIMES = 5;
  static const std::thread::id null_thread;
  ReentrantRWLock() {}
  explicit ReentrantRWLock(bool write_first) : write_first_(write_first) {}

private:
  // all these function only can used by ReadLockGuard/WriteLockGuard;
  void ReadLock();
  void WriteLock();

  void ReadUnlock();
  void WriteUnlock();

  ReentrantRWLock(const ReentrantRWLock &) = delete;
  ReentrantRWLock &operator=(const ReentrantRWLock &) = delete;
  std::thread::id write_thread_id_ = {NULL_THREAD_ID};
  std::atomic<uint32_t> write_lock_wait_num_ = {0};
  std::atomic<int32_t> lock_num_ = {0};
  bool write_first_ = true;
};

inline void ReentrantRWLock::ReadLock() {
  if (write_thread_id_ == std::this_thread::get_id()) {
    return;
  }

  uint32_t retry_times = 0;
  int32_t lock_num = lock_num_.load(std::memory_order_acquire);
  if (write_first_) {
    do {
      while (lock_num < RW_LOCK_FREE ||
             write_lock_wait_num_.load(std::memory_order_acquire) > 0) {
        if (++retry_times == MAX_RETRY_TIMES) {
          // saving cpu
          std::this_thread::yield();
          retry_times = 0;
        }
        lock_num = lock_num_.load(std::memory_order_acquire);
      }
    } while (!lock_num_.compare_exchange_weak(lock_num, lock_num + 1,
                                              std::memory_order_acq_rel,
                                              std::memory_order_relaxed));
  } else {
    do {
      while (lock_num < RW_LOCK_FREE) {
        if (++retry_times == MAX_RETRY_TIMES) {
          // saving cpu
          std::this_thread::yield();
          retry_times = 0;
        }
        lock_num = lock_num_.load(std::memory_order_acquire);
      }
    } while (!lock_num_.compare_exchange_weak(lock_num, lock_num + 1,
                                              std::memory_order_acq_rel,
                                              std::memory_order_relaxed));
  }
}

inline void ReentrantRWLock::WriteLock() {
  auto this_thread_id = std::this_thread::get_id();
  if (write_thread_id_ == this_thread_id) {
    lock_num_.fetch_sub(1);
    return;
  }
  int32_t rw_lock_free = RW_LOCK_FREE;
  uint32_t retry_times = 0;
  write_lock_wait_num_.fetch_add(1);
  while (!lock_num_.compare_exchange_weak(rw_lock_free, WRITE_EXCLUSIVE,
                                          std::memory_order_acq_rel,
                                          std::memory_order_relaxed)) {
    // rw_lock_free will change after CAS fail, so init agin
    rw_lock_free = RW_LOCK_FREE;
    if (++retry_times == MAX_RETRY_TIMES) {
      // saving cpu
      std::this_thread::yield();
      retry_times = 0;
    }
  }
  write_thread_id_ = this_thread_id;
  write_lock_wait_num_.fetch_sub(1);
}

inline void ReentrantRWLock::ReadUnlock() {
  if (write_thread_id_ == std::this_thread::get_id()) {
    return;
  }
  lock_num_.fetch_sub(1);
}

inline void ReentrantRWLock::WriteUnlock() {
  if (lock_num_.fetch_add(1) == WRITE_EXCLUSIVE) {
    write_thread_id_ = NULL_THREAD_ID;
  }
}

class WaitStrategy {
public:
  virtual void NotifyOne() {}
  virtual void BreakAllWait() {}
  virtual bool EmptyWait() = 0;
  virtual ~WaitStrategy() {}
};

class BlockWaitStrategy : public WaitStrategy {
public:
  BlockWaitStrategy() {}
  void NotifyOne() override { cv_.notify_one(); }

  bool EmptyWait() override {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock);
    return true;
  }

  void BreakAllWait() override { cv_.notify_all(); }

private:
  std::mutex mutex_;
  std::condition_variable cv_;
};

class SleepWaitStrategy : public WaitStrategy {
public:
  SleepWaitStrategy() {}
  explicit SleepWaitStrategy(uint64_t sleep_time_us)
      : sleep_time_us_(sleep_time_us) {}

  bool EmptyWait() override {
    std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_us_));
    return true;
  }

  void SetSleepTimeMicroSeconds(uint64_t sleep_time_us) {
    sleep_time_us_ = sleep_time_us;
  }

private:
  uint64_t sleep_time_us_ = 10000;
};

class YieldWaitStrategy : public WaitStrategy {
public:
  YieldWaitStrategy() {}
  bool EmptyWait() override {
    std::this_thread::yield();
    return true;
  }
};

class BusySpinWaitStrategy : public WaitStrategy {
public:
  BusySpinWaitStrategy() {}
  bool EmptyWait() override { return true; }
};

class TimeoutBlockWaitStrategy : public WaitStrategy {
public:
  TimeoutBlockWaitStrategy() {}
  explicit TimeoutBlockWaitStrategy(uint64_t timeout)
      : time_out_(std::chrono::milliseconds(timeout)) {}

  void NotifyOne() override { cv_.notify_one(); }

  bool EmptyWait() override {
    std::unique_lock<std::mutex> lock(mutex_);
    if (cv_.wait_for(lock, time_out_) == std::cv_status::timeout) {
      return false;
    }
    return true;
  }

  void BreakAllWait() override { cv_.notify_all(); }

  void SetTimeout(uint64_t timeout) {
    time_out_ = std::chrono::milliseconds(timeout);
  }

private:
  std::mutex mutex_;
  std::condition_variable cv_;
  std::chrono::milliseconds time_out_;
};

template <typename T> class BoundedQueue {
public:
  using value_type = T;
  using size_type = uint64_t;

public:
  BoundedQueue() {}
  BoundedQueue &operator=(const BoundedQueue &other) = delete;
  BoundedQueue(const BoundedQueue &other) = delete;
  ~BoundedQueue();
  bool Init(uint64_t size);
  bool Init(uint64_t size, WaitStrategy *strategy);
  bool Enqueue(const T &element);
  bool Enqueue(T &&element);
  bool WaitEnqueue(const T &element);
  bool WaitEnqueue(T &&element);
  bool Dequeue(T *element);
  bool WaitDequeue(T *element);
  uint64_t Size();
  bool Empty();
  void SetWaitStrategy(WaitStrategy *WaitStrategy);
  void BreakAllWait();
  uint64_t Head() { return head_.load(); }
  uint64_t Tail() { return tail_.load(); }
  uint64_t Commit() { return commit_.load(); }

private:
  uint64_t GetIndex(uint64_t num);

  alignas(CACHELINE_SIZE) std::atomic<uint64_t> head_ = {0};
  alignas(CACHELINE_SIZE) std::atomic<uint64_t> tail_ = {1};
  alignas(CACHELINE_SIZE) std::atomic<uint64_t> commit_ = {1};
  // alignas(CACHELINE_SIZE) std::atomic<uint64_t> size_ = {0};
  uint64_t pool_size_ = 0;
  T *pool_ = nullptr;
  std::unique_ptr<WaitStrategy> wait_strategy_ = nullptr;
  volatile bool break_all_wait_ = false;
};

template <typename T> BoundedQueue<T>::~BoundedQueue() {
  if (wait_strategy_) {
    BreakAllWait();
  }
  if (pool_) {
    for (uint64_t i = 0; i < pool_size_; ++i) {
      pool_[i].~T();
    }
    std::free(pool_);
  }
}

template <typename T> inline bool BoundedQueue<T>::Init(uint64_t size) {
  return Init(size, new SleepWaitStrategy());
}

template <typename T>
bool BoundedQueue<T>::Init(uint64_t size, WaitStrategy *strategy) {
  // Head and tail each occupy a space
  pool_size_ = size + 2;
  pool_ = reinterpret_cast<T *>(std::calloc(pool_size_, sizeof(T)));
  if (pool_ == nullptr) {
    return false;
  }
  for (uint64_t i = 0; i < pool_size_; ++i) {
    new (&(pool_[i])) T();
  }
  wait_strategy_.reset(strategy);
  return true;
}

template <typename T> bool BoundedQueue<T>::Enqueue(const T &element) {
  uint64_t new_tail = 0;
  uint64_t old_commit = 0;
  uint64_t old_tail = tail_.load(std::memory_order_acquire);
  do {
    new_tail = old_tail + 1;
    if (GetIndex(new_tail) == GetIndex(head_.load(std::memory_order_acquire))) {
      return false;
    }
  } while (!tail_.compare_exchange_weak(old_tail, new_tail,
                                        std::memory_order_acq_rel,
                                        std::memory_order_relaxed));
  pool_[GetIndex(old_tail)] = element;
  do {
    old_commit = old_tail;
  } while (unlikely(!commit_.compare_exchange_weak(old_commit, new_tail,
                                                   std::memory_order_acq_rel,
                                                   std::memory_order_relaxed)));
  wait_strategy_->NotifyOne();
  return true;
}

template <typename T> bool BoundedQueue<T>::Enqueue(T &&element) {
  uint64_t new_tail = 0;
  uint64_t old_commit = 0;
  uint64_t old_tail = tail_.load(std::memory_order_acquire);
  do {
    new_tail = old_tail + 1;
    if (GetIndex(new_tail) == GetIndex(head_.load(std::memory_order_acquire))) {
      return false;
    }
  } while (!tail_.compare_exchange_weak(old_tail, new_tail,
                                        std::memory_order_acq_rel,
                                        std::memory_order_relaxed));
  pool_[GetIndex(old_tail)] = std::move(element);
  do {
    old_commit = old_tail;
  } while (unlikely(!commit_.compare_exchange_weak(old_commit, new_tail,
                                                   std::memory_order_acq_rel,
                                                   std::memory_order_relaxed)));
  wait_strategy_->NotifyOne();
  return true;
}

template <typename T> bool BoundedQueue<T>::Dequeue(T *element) {
  uint64_t new_head = 0;
  uint64_t old_head = head_.load(std::memory_order_acquire);
  do {
    new_head = old_head + 1;
    if (new_head == commit_.load(std::memory_order_acquire)) {
      return false;
    }
    *element = pool_[GetIndex(new_head)];
  } while (!head_.compare_exchange_weak(old_head, new_head,
                                        std::memory_order_acq_rel,
                                        std::memory_order_relaxed));
  return true;
}

template <typename T> bool BoundedQueue<T>::WaitEnqueue(const T &element) {
  while (!break_all_wait_) {
    if (Enqueue(element)) {
      return true;
    }
    if (wait_strategy_->EmptyWait()) {
      continue;
    }
    // wait timeout
    break;
  }

  return false;
}

template <typename T> bool BoundedQueue<T>::WaitEnqueue(T &&element) {
  while (!break_all_wait_) {
    if (Enqueue(std::move(element))) {
      return true;
    }
    if (wait_strategy_->EmptyWait()) {
      continue;
    }
    // wait timeout
    break;
  }

  return false;
}

template <typename T> bool BoundedQueue<T>::WaitDequeue(T *element) {
  while (!break_all_wait_) {
    if (Dequeue(element)) {
      return true;
    }
    if (wait_strategy_->EmptyWait()) {
      continue;
    }
    // wait timeout
    break;
  }

  return false;
}

template <typename T> inline uint64_t BoundedQueue<T>::Size() {
  return tail_ - head_ - 1;
}

template <typename T> inline bool BoundedQueue<T>::Empty() {
  return Size() == 0;
}

template <typename T> inline uint64_t BoundedQueue<T>::GetIndex(uint64_t num) {
  return num - (num / pool_size_) * pool_size_; // faster than %
}

template <typename T>
inline void BoundedQueue<T>::SetWaitStrategy(WaitStrategy *strategy) {
  wait_strategy_.reset(strategy);
}

template <typename T> inline void BoundedQueue<T>::BreakAllWait() {
  break_all_wait_ = true;
  wait_strategy_->BreakAllWait();
}

template <typename T> class UnboundedQueue {
public:
  UnboundedQueue() { Reset(); }
  UnboundedQueue &operator=(const UnboundedQueue &other) = delete;
  UnboundedQueue(const UnboundedQueue &other) = delete;

  ~UnboundedQueue() { Destroy(); }

  void Clear() {
    Destroy();
    Reset();
  }

  void Enqueue(const T &element) {
    auto node = new Node();
    node->data = element;
    Node *old_tail = tail_.load();

    while (true) {
      if (tail_.compare_exchange_strong(old_tail, node)) {
        old_tail->next = node;
        old_tail->release();
        size_.fetch_add(1);
        break;
      }
    }
  }

  bool Dequeue(T *element) {
    Node *old_head = head_.load();
    Node *head_next = nullptr;
    do {
      head_next = old_head->next;

      if (head_next == nullptr) {
        return false;
      }
    } while (!head_.compare_exchange_strong(old_head, head_next));
    *element = head_next->data;
    size_.fetch_sub(1);
    old_head->release();
    return true;
  }

  size_t Size() { return size_.load(); }

  bool Empty() { return size_.load() == 0; }

private:
  struct Node {
    T data;
    std::atomic<uint32_t> ref_count;
    Node *next = nullptr;
    Node() { ref_count.store(2); }
    void release() {
      ref_count.fetch_sub(1);
      if (ref_count.load() == 0) {
        delete this;
      }
    }
  };

  void Reset() {
    auto node = new Node();
    head_.store(node);
    tail_.store(node);
    size_.store(0);
  }

  void Destroy() {
    auto ite = head_.load();
    Node *tmp = nullptr;
    while (ite != nullptr) {
      tmp = ite->next;
      delete ite;
      ite = tmp;
    }
  }

  std::atomic<Node *> head_;
  std::atomic<Node *> tail_;
  std::atomic<size_t> size_;
};

class ThreadPool {
public:
  explicit ThreadPool(std::size_t thread_num, std::size_t max_task_num = 1000);

  template <typename F, typename... Args>
  auto Enqueue(F &&f, Args &&...args)
      -> std::future<typename std::result_of<F(Args...)>::type>;

  ~ThreadPool();

private:
  std::vector<std::thread> workers_;
  BoundedQueue<std::function<void()>> task_queue_;
  std::atomic_bool stop_;
};

inline ThreadPool::ThreadPool(std::size_t threads, std::size_t max_task_num)
    : stop_(false) {
  if (!task_queue_.Init(max_task_num, new BlockWaitStrategy())) {
    throw std::runtime_error("Task queue init failed.");
  }
  workers_.reserve(threads);
  for (size_t i = 0; i < threads; ++i) {
    workers_.emplace_back([this] {
      while (!stop_) {
        std::function<void()> task;
        if (task_queue_.WaitDequeue(&task)) {
          task();
        }
      }
    });
  }
}

// before using the return value, you should check value.valid()
template <typename F, typename... Args>
auto ThreadPool::Enqueue(F &&f, Args &&...args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
  using return_type = typename std::result_of<F(Args...)>::type;

  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();

  // don't allow enqueueing after stopping the pool
  if (stop_) {
    return std::future<return_type>();
  }
  task_queue_.Enqueue([task]() { (*task)(); });
  return res;
};

// the destructor joins all threads
inline ThreadPool::~ThreadPool() {
  if (stop_.exchange(true)) {
    return;
  }
  task_queue_.BreakAllWait();
  for (std::thread &worker : workers_) {
    worker.join();
  }
}

template <typename T> class ThreadSafeQueue {
public:
  ThreadSafeQueue() {}
  ThreadSafeQueue &operator=(const ThreadSafeQueue &other) = delete;
  ThreadSafeQueue(const ThreadSafeQueue &other) = delete;

  ~ThreadSafeQueue() { BreakAllWait(); }

  void Enqueue(const T &element) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.emplace(element);
    cv_.notify_one();
  }

  bool Dequeue(T *element) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
      return false;
    }
    *element = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  bool WaitDequeue(T *element) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() { return break_all_wait_ || !queue_.empty(); });
    if (break_all_wait_) {
      return false;
    }
    *element = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  typename std::queue<T>::size_type Size() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  bool Empty() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  void BreakAllWait() {
    break_all_wait_ = true;
    cv_.notify_all();
  }

private:
  volatile bool break_all_wait_ = false;
  std::mutex mutex_;
  std::queue<T> queue_;
  std::condition_variable cv_;
};

} // namespace thread

namespace io {

enum class PCDDataType { ASCII = 0, BINARY = 1, BINARY_COMPRESSED = 2 };

struct PCLPointField {
public:
  std::string name;
  int size;
  char type;
  int count;
  // helper variable
  int count_offset = 0;
  int offset = 0;
};

struct PCDHeader {
public:
  std::string version;
  std::vector<PCLPointField> fields;
  int width;
  int height;
  int points;
  PCDDataType datatype;
  std::string viewpoint;

  // helper variable
  int elementnum;
  int pointsize;
  // std::unordered_map<std::string, bool> has_attr;
  // std::unordered_map<std::string, bool> attr_dtype;
};

static bool InitializePCDHeader(PCDHeader &header) { return false; }

} // namespace io

} // namespace cpp_toolbox

#endif // __CPP_TOOLBOX_H_
