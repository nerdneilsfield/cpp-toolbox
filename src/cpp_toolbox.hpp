#ifndef _CPP_TOOLBOX_H_
#define _CPP_TOOLBOX_H_

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <locale>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#define FUNCTION __FUNCSIG__
#else
#define FUNCTION __PRETTY_FUNCTION__
#endif

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

namespace cpp_toolbox {

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

template <typename In, typename Out> Out FindRegex(const In &str, const std::string& regex) {
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

template <typename In, typename Out> std::vector<Out> FindAllRegex(const In &str, const std::string& regex) {
  std::vector<Out> result;
  for (const auto& s : StrFindAllRegex(ToString(str), regex)) {
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
    throw std::runtime_error("Failed to delete file: " + filename);
  }
}

} // namespace file

namespace container {} // namespace container

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
