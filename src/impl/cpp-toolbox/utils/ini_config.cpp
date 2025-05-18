#include <fstream>
#include <sstream>
#include <filesystem>

#include <cpp-toolbox/logger/thread_logger.hpp>
#include <cpp-toolbox/utils/ini_config.hpp>

namespace toolbox::utils
{
bool ini_config_t::load(const std::filesystem::path& file_path)
{
  std::ifstream ifs(file_path);
  if (!ifs.is_open()) {
    LOG_ERROR_S << "Failed to open config file: " << file_path;
    return false;
  }

  std::string current_section;
  std::string line;
  while (std::getline(ifs, line)) {
    // Trim whitespace
    line.erase(0, line.find_first_not_of(" \t\r\n"));
    line.erase(line.find_last_not_of(" \t\r\n") + 1);

    if (line.empty() || line[0] == ';' || line[0] == '#') {
      continue;  // Skip comments/blank lines
    }

    if (line.front() == '[' && line.back() == ']') {
      current_section = line.substr(1, line.size() - 2);
      continue;
    }

    auto pos = line.find('=');
    if (pos == std::string::npos)
      pos = line.find(':');
    if (pos == std::string::npos)
      continue;  // Invalid line

    std::string key = line.substr(0, pos);
    std::string value = line.substr(pos + 1);
    // Trim
    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);

    data_[current_section][key] = value;
  }
  return true;
}

bool ini_config_t::has(const std::string& section, const std::string& key) const
{
  auto sec_it = data_.find(section);
  if (sec_it == data_.end())
    return false;
  return sec_it->second.find(key) != sec_it->second.end();
}

std::string ini_config_t::get_string(const std::string& section,
                                    const std::string& key,
                                    const std::string& default_value) const
{
  auto sec_it = data_.find(section);
  if (sec_it == data_.end())
    return default_value;
  auto key_it = sec_it->second.find(key);
  if (key_it == sec_it->second.end())
    return default_value;
  return key_it->second;
}
}  // namespace toolbox::utils
