#pragma once

/**
 * @file knn.hpp
 * @brief KNN算法统一导出文件 / Unified export file for KNN algorithms
 * 
 * 该文件提供了所有KNN（K近邻）算法的统一接口，包括暴力搜索、KD树和并行版本。
 * This file provides a unified interface for all KNN (K-Nearest Neighbors) algorithms,
 * including brute-force search, KD-tree, and parallel versions.
 * 
 * @code
 * #include <cpp-toolbox/pcl/knn/knn.hpp>
 * 
 * using namespace toolbox::pcl;
 * 
 * // 使用暴力搜索 / Using brute-force search
 * bfknn_t<float> bf_knn;
 * 
 * // 使用KD树（适合大规模数据） / Using KD-tree (suitable for large datasets)
 * kdtree_t<float> kd_knn;
 * 
 * // 使用并行暴力搜索 / Using parallel brute-force search
 * bfknn_parallel_t<float> parallel_knn;
 * 
 * // 设置数据并搜索 / Set data and search
 * point_cloud_t<float> cloud = load_point_cloud();
 * kd_knn.set_input(cloud);
 * 
 * point_t<float> query = {1.0f, 2.0f, 3.0f};
 * std::vector<std::size_t> indices;
 * std::vector<float> distances;
 * kd_knn.kneighbors(query, 10, indices, distances);
 * @endcode
 * 
 * @code
 * // 使用自定义度量 / Using custom metrics
 * using namespace toolbox::metrics;
 * 
 * // 编译时指定度量 / Compile-time metric specification
 * bfknn_generic_t<point_t<float>, L1Metric<float>> knn_l1;
 * bfknn_generic_t<point_t<float>, CosineMetric<float>> knn_cosine;
 * 
 * // 运行时切换度量 / Runtime metric switching
 * auto knn = std::make_unique<kdtree_t<float>>();
 * auto metric = MetricFactory<float>::instance().create("angular");
 * knn->set_metric(std::move(metric));
 * @endcode
 */

#include <cpp-toolbox/pcl/knn/base_knn.hpp>
#include <cpp-toolbox/pcl/knn/bfknn.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>

namespace toolbox::pcl
{

/**
 * @defgroup knn KNN算法 / KNN Algorithms
 * @{
 */

/**
 * @brief KNN算法的选择指南 / Guide for choosing KNN algorithms
 * 
 * - bfknn_t: 适用于小规模数据集（<1000点）或需要精确结果的场景 / 
 *   Suitable for small datasets (<1000 points) or scenarios requiring exact results
 * 
 * - kdtree_t: 适用于大规模数据集（>1000点）且使用L2度量的场景 / 
 *   Suitable for large datasets (>1000 points) with L2 metric
 * 
 * - bfknn_parallel_t: 适用于中等规模数据集需要加速的场景 / 
 *   Suitable for medium-sized datasets requiring acceleration
 * 
 * @code
 * // 根据数据规模选择算法 / Choose algorithm based on data size
 * template<typename T>
 * std::unique_ptr<base_knn_generic_t<?, point_t<T>, L2Metric<T>>> 
 * create_knn(size_t num_points) {
 *     if (num_points < 1000) {
 *         return std::make_unique<bfknn_t<T>>();
 *     } else if (num_points < 10000) {
 *         return std::make_unique<bfknn_parallel_t<T>>();
 *     } else {
 *         return std::make_unique<kdtree_t<T>>();
 *     }
 * }
 * @endcode
 */

/**
 * @brief 创建默认的KNN搜索器（根据数据规模自动选择） / Create default KNN searcher (auto-select based on data size)
 * @tparam T 数据类型 / Data type
 * @param num_points 预期的数据点数量 / Expected number of data points
 * @return KNN搜索器的智能指针 / Smart pointer to KNN searcher
 * 
 * @code
 * auto knn = create_default_knn<float>(cloud.size());
 * knn->set_input(cloud);
 * @endcode
 */
template<typename T>
inline auto create_default_knn(size_t num_points = 0) {
    if (num_points > 10000) {
        return std::make_unique<kdtree_t<T>>();
    } else if (num_points > 1000) {
        return std::make_unique<bfknn_parallel_t<T>>();
    } else {
        return std::make_unique<bfknn_t<T>>();
    }
}

/**
 * @brief 性能基准测试辅助函数 / Performance benchmark helper function
 * @tparam KNN KNN算法类型 / KNN algorithm type
 * @param knn KNN搜索器 / KNN searcher
 * @param queries 查询点集合 / Query points collection
 * @param k 近邻数量 / Number of neighbors
 * @return 平均搜索时间（毫秒） / Average search time (milliseconds)
 * 
 * @code
 * // 比较不同算法的性能 / Compare performance of different algorithms
 * bfknn_t<float> bf_knn;
 * kdtree_t<float> kd_knn;
 * 
 * bf_knn.set_input(cloud);
 * kd_knn.set_input(cloud);
 * 
 * double bf_time = benchmark_knn(bf_knn, queries, 10);
 * double kd_time = benchmark_knn(kd_knn, queries, 10);
 * 
 * std::cout << "暴力搜索时间 / Brute-force time: " << bf_time << " ms\n";
 * std::cout << "KD树搜索时间 / KD-tree time: " << kd_time << " ms\n";
 * @endcode
 */
template<typename KNN>
double benchmark_knn(KNN& knn, 
                     const std::vector<typename KNN::element_type>& queries,
                     size_t k) {
    using namespace std::chrono;
    
    std::vector<std::size_t> indices;
    std::vector<typename KNN::distance_type> distances;
    
    auto start = high_resolution_clock::now();
    
    for (const auto& query : queries) {
        knn.kneighbors(query, k, indices, distances);
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    
    return static_cast<double>(duration.count()) / 1000.0 / queries.size();
}

/** @} */ // end of knn group

}  // namespace toolbox::pcl