#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::utils
{
class CPP_TOOLBOX_EXPORT ini_config_t
{
public:
  bool load(const std::filesystem::path& file_path);
  bool has(const std::string& section, const std::string& key) const;
  std::string get_string(const std::string& section,
                         const std::string& key,
                         const std::string& default_value = "") const;

private:
  using section_map_t =
      std::unordered_map<std::string, std::unordered_map<std::string, std::string>>;
  section_map_t data_;
};
}  // namespace toolbox::utils
