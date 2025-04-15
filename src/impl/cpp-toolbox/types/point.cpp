#include <cstdint>

#include <cpp-toolbox/types/point.hpp>

namespace toolbox::types
{

// point_t instantiations
template struct CPP_TOOLBOX_EXPORT point_t<std::int8_t>;
template struct CPP_TOOLBOX_EXPORT point_t<std::uint8_t>;
template struct CPP_TOOLBOX_EXPORT point_t<std::int16_t>;
template struct CPP_TOOLBOX_EXPORT point_t<std::uint16_t>;
template struct CPP_TOOLBOX_EXPORT point_t<std::int32_t>;
template struct CPP_TOOLBOX_EXPORT point_t<std::uint32_t>;
template struct CPP_TOOLBOX_EXPORT point_t<std::int64_t>;
template struct CPP_TOOLBOX_EXPORT point_t<std::uint64_t>;
template struct CPP_TOOLBOX_EXPORT point_t<float>;
template struct CPP_TOOLBOX_EXPORT point_t<double>;

// point_cloud_t instantiations
template class CPP_TOOLBOX_EXPORT point_cloud_t<std::int8_t>;
template class CPP_TOOLBOX_EXPORT point_cloud_t<std::uint8_t>;
template class CPP_TOOLBOX_EXPORT point_cloud_t<std::int16_t>;
template class CPP_TOOLBOX_EXPORT point_cloud_t<std::uint16_t>;
template class CPP_TOOLBOX_EXPORT point_cloud_t<std::int32_t>;
template class CPP_TOOLBOX_EXPORT point_cloud_t<std::uint32_t>;
template class CPP_TOOLBOX_EXPORT point_cloud_t<std::int64_t>;
template class CPP_TOOLBOX_EXPORT point_cloud_t<std::uint64_t>;
template class CPP_TOOLBOX_EXPORT point_cloud_t<float>;
template class CPP_TOOLBOX_EXPORT point_cloud_t<double>;

// Explicit instantiation for operator<<
// Note: For non-member function templates, explicit instantiation is slightly
// different.
template std::ostream& operator<<(std::ostream&, const point_t<std::int8_t>&);
template std::ostream& operator<<(std::ostream&, const point_t<std::uint8_t>&);
template std::ostream& operator<<(std::ostream&, const point_t<std::int16_t>&);
template std::ostream& operator<<(std::ostream&, const point_t<std::uint16_t>&);
template std::ostream& operator<<(std::ostream&, const point_t<std::int32_t>&);
template std::ostream& operator<<(std::ostream&, const point_t<std::uint32_t>&);
template std::ostream& operator<<(std::ostream&, const point_t<std::int64_t>&);
template std::ostream& operator<<(std::ostream&, const point_t<std::uint64_t>&);
template std::ostream& operator<<(std::ostream&, const point_t<float>&);
template std::ostream& operator<<(std::ostream&, const point_t<double>&);

}  // namespace toolbox::types
