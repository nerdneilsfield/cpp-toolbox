#pragma once

/**
 * @file correspondence_generator.hpp
 * @brief 向后兼容的别名文件 / Backward compatibility alias file
 * 
 * 此文件仅用于向后兼容。新代码应直接使用 knn_correspondence_generator_t
 * This file is for backward compatibility only. New code should use knn_correspondence_generator_t directly
 */

#include <cpp-toolbox/pcl/correspondence/knn_correspondence_generator.hpp>

namespace toolbox::pcl
{

/**
 * @brief 向后兼容的类型别名 / Backward compatibility type alias
 * 
 * @deprecated 请使用 knn_correspondence_generator_t / Please use knn_correspondence_generator_t
 */
template<typename DataType, typename Signature, typename KNN>
using correspondence_generator_t = knn_correspondence_generator_t<DataType, Signature, KNN>;

}  // namespace toolbox::pcl