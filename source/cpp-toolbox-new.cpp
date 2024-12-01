#include <string>

#include "cpp-toolbox-new/cpp-toolbox-new.hpp"

exported_class::exported_class()
    : m_name {"cpp-toolbox-new"}
{
}

auto exported_class::name() const -> char const*
{
  return m_name.c_str();
}
