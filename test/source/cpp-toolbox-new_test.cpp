#include <string>

#include "cpp-toolbox-new/cpp-toolbox-new.hpp"

auto main() -> int
{
  auto const exported = exported_class {};

  return std::string("cpp-toolbox-new") == exported.name() ? 0 : 1;
}
