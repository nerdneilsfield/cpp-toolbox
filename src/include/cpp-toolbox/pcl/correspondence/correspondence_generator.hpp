#pragma once

/**
 * @file correspondence_generator.hpp
 * @brief 对应点生成器的向后兼容别名 / Backward compatibility aliases for correspondence generators
 * 
 * 该文件提供了向后兼容的类型别名。新代码应该使用具体的实现类：
 * This file provides backward compatibility type aliases. New code should use specific implementation classes:
 * - knn_correspondence_generator_t: 基于KNN的对应点生成 / KNN-based correspondence generation
 * - brute_force_correspondence_generator_t: 暴力搜索对应点生成 / Brute-force correspondence generation
 */

#include <cpp-toolbox/pcl/correspondence/knn_correspondence_generator.hpp>

namespace toolbox::pcl
{

/**
 * @brief 对应点生成器的类型别名（向后兼容） / Type alias for correspondence generator (backward compatibility)
 * 
 * @deprecated 请使用 knn_correspondence_generator_t 代替 / Please use knn_correspondence_generator_t instead
 */
template<typename DataType, typename Signature, typename KNN>
using correspondence_generator_t = knn_correspondence_generator_t<DataType, Signature, KNN>;

}  // namespace toolbox::pcl