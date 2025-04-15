#include <cstdint>

#include <cpp-toolbox/types.hpp>
#include <cpp-toolbox/types/minmax.hpp>

namespace toolbox::types
{

// Explicit template instantiation for common types
// Forces the compiler to generate code for these, linkable into the library.

// minmax_t<T> instantiations for primitive types
template struct CPP_TOOLBOX_EXPORT minmax_t<std::int8_t>;
template struct CPP_TOOLBOX_EXPORT minmax_t<std::uint8_t>;
template struct CPP_TOOLBOX_EXPORT minmax_t<std::int16_t>;
template struct CPP_TOOLBOX_EXPORT minmax_t<std::uint16_t>;
template struct CPP_TOOLBOX_EXPORT minmax_t<std::int32_t>;
template struct CPP_TOOLBOX_EXPORT minmax_t<std::uint32_t>;
template struct CPP_TOOLBOX_EXPORT minmax_t<std::int64_t>;
template struct CPP_TOOLBOX_EXPORT minmax_t<std::uint64_t>;
template struct CPP_TOOLBOX_EXPORT minmax_t<float>;
template struct CPP_TOOLBOX_EXPORT minmax_t<double>;

// minmax_t<point_t<T>> instantiations for common coordinate types
template struct CPP_TOOLBOX_EXPORT minmax_t<point_t<std::int32_t>>;
template struct CPP_TOOLBOX_EXPORT minmax_t<point_t<std::uint32_t>>;
template struct CPP_TOOLBOX_EXPORT minmax_t<point_t<std::int64_t>>;
template struct CPP_TOOLBOX_EXPORT minmax_t<point_t<std::uint64_t>>;
template struct CPP_TOOLBOX_EXPORT minmax_t<point_t<std::int8_t>>;
template struct CPP_TOOLBOX_EXPORT minmax_t<point_t<std::uint8_t>>;
template struct CPP_TOOLBOX_EXPORT minmax_t<point_t<std::int16_t>>;
template struct CPP_TOOLBOX_EXPORT minmax_t<point_t<std::uint16_t>>;
template struct CPP_TOOLBOX_EXPORT minmax_t<point_t<float>>;
template struct CPP_TOOLBOX_EXPORT minmax_t<point_t<double>>;

// Note: Explicit instantiations for the helper functions (combine_minmax,
// calculate_minmax, calculate_minmax_parallel) are generally not needed
// because they are template functions themselves, and their instantiations
// are triggered when called with specific types (like those above).
// If you encounter linking errors related to them, you might need to
// instantiate them explicitly, similar to how operator<< was done for point_t,
// but it's less common for these kinds of helper function templates.

}  // namespace toolbox::types