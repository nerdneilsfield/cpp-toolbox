// Explicit instantiations for functional templates (if needed)
#include <cpp-toolbox/functional/functional.hpp>

namespace toolbox::functional
{

// Currently, explicit instantiations are likely not required for these function
// templates. If linker errors arise for specific uses of compose, map, filter,
// reduce, zip, memoize, etc., they might need to be instantiated here for the
// specific types involved. Example (if needed for map<vector<int>, lambda ->
// int>): template std::vector<int> map(const std::vector<int>&, int(*)(const
// int&));

// MemoizedFunction might require instantiation if its methods are used across
// translation units in ways that require the definitions to be available.
// Example (if needed for MemoizedFunction<int(int)>):
// template class CPP_TOOLBOX_EXPORT MemoizedFunction<int(int)>;

}  // namespace toolbox::functional
