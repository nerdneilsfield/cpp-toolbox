#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cpp-toolbox/pcl/correspondence/correspondence_sorter.hpp>
#include <cpp-toolbox/types/point.hpp>

#include <memory>
#include <random>
#include <vector>

using namespace toolbox::pcl;
using namespace toolbox::types;
using namespace Catch::Matchers;

// 辅助函数：创建测试数据 / Helper function: create test data
template<typename DataType>
void create_test_data(
    std::shared_ptr<point_cloud_t<DataType>>& cloud,
    std::shared_ptr<std::vector<correspondence_t>>& correspondences,
    std::size_t num_points = 100,
    std::size_t num_correspondences = 50)
{
  // 创建点云 / Create point cloud
  cloud = std::make_shared<point_cloud_t<DataType>>();
  cloud->points.reserve(num_points);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<DataType> dist(-10.0, 10.0);

  for (std::size_t i = 0; i < num_points; ++i) {
    point_t<DataType> pt;
    pt.x = dist(gen);
    pt.y = dist(gen);
    pt.z = dist(gen);
    cloud->points.push_back(pt);
  }

  // 创建对应关系，距离递增 / Create correspondences with increasing distances
  correspondences = std::make_shared<std::vector<correspondence_t>>();
  correspondences->reserve(num_correspondences);

  std::uniform_int_distribution<std::size_t> idx_dist(0, num_points - 1);
  for (std::size_t i = 0; i < num_correspondences; ++i) {
    correspondence_t corr;
    corr.src_idx = idx_dist(gen);
    corr.dst_idx = idx_dist(gen);
    // 距离从0.1递增到5.0 / Distance increases from 0.1 to 5.0
    corr.distance = 0.1f + (4.9f * i / (num_correspondences - 1));
    correspondences->push_back(corr);
  }
}

TEST_CASE("DescriptorDistanceSorter", "[correspondence][sorter]")
{
  using DataType = float;
  
  std::shared_ptr<point_cloud_t<DataType>> source_cloud, target_cloud;
  std::shared_ptr<std::vector<correspondence_t>> correspondences;
  create_test_data<DataType>(source_cloud, correspondences);
  create_test_data<DataType>(target_cloud, correspondences);

  SECTION("基本功能测试 / Basic functionality test")
  {
    descriptor_distance_sorter_t<DataType> sorter;
    sorter.set_point_clouds(source_cloud, target_cloud);
    sorter.set_correspondences(correspondences);
    sorter.set_invert_score(true);  // 距离越小质量越高 / Smaller distance means higher quality

    std::vector<DataType> scores;
    auto sorted_indices = sorter.compute_sorted_indices(scores);

    // 检查返回大小 / Check return size
    REQUIRE(sorted_indices.size() == correspondences->size());
    REQUIRE(scores.size() == correspondences->size());

    // 检查排序顺序（质量降序） / Check sorting order (quality descending)
    for (std::size_t i = 1; i < sorted_indices.size(); ++i) {
      REQUIRE(scores[sorted_indices[i - 1]] >= scores[sorted_indices[i]]);
    }

    // 检查第一个应该是距离最小的 / Check first should be smallest distance
    REQUIRE_THAT((*correspondences)[sorted_indices[0]].distance,
                 WithinAbs(0.1f, 0.01f));
  }

  SECTION("不反转分数测试 / Non-inverted score test")
  {
    descriptor_distance_sorter_t<DataType> sorter;
    sorter.set_point_clouds(source_cloud, target_cloud);
    sorter.set_correspondences(correspondences);
    sorter.set_invert_score(false);  // 距离越大质量越高 / Larger distance means higher quality

    std::vector<DataType> scores;
    auto sorted_indices = sorter.compute_sorted_indices(scores);

    // 检查第一个应该是距离最大的 / Check first should be largest distance
    REQUIRE_THAT((*correspondences)[sorted_indices[0]].distance,
                 WithinAbs(5.0f, 0.01f));
  }

  SECTION("归一化测试 / Normalization test")
  {
    descriptor_distance_sorter_t<DataType> sorter;
    sorter.set_point_clouds(source_cloud, target_cloud);
    sorter.set_correspondences(correspondences);
    sorter.set_normalize(true);
    sorter.set_invert_score(true);

    std::vector<DataType> scores;
    sorter.compute_sorted_indices(scores);

    // 检查分数范围在[0,1] / Check scores are in range [0,1]
    for (const auto& score : scores) {
      REQUIRE(score >= 0.0f);
      REQUIRE(score <= 1.0f);
    }
  }

  SECTION("缓存测试 / Cache test")
  {
    descriptor_distance_sorter_t<DataType> sorter;
    sorter.set_point_clouds(source_cloud, target_cloud);
    sorter.set_correspondences(correspondences);

    // 第一次计算 / First computation
    std::vector<DataType> scores1;
    auto indices1 = sorter.compute_sorted_indices(scores1);

    // 第二次计算（应该使用缓存） / Second computation (should use cache)
    std::vector<DataType> scores2;
    auto indices2 = sorter.compute_sorted_indices(scores2);

    // 结果应该相同 / Results should be identical
    REQUIRE(indices1 == indices2);
    REQUIRE(scores1 == scores2);

    // 改变参数后清除缓存 / Clear cache after changing parameters
    sorter.set_invert_score(!sorter.get_invert_score());
    std::vector<DataType> scores3;
    auto indices3 = sorter.compute_sorted_indices(scores3);

    // 结果应该不同 / Results should be different
    REQUIRE(indices1 != indices3);
  }
}

TEST_CASE("GeometricConsistencySorter", "[correspondence][sorter]")
{
  using DataType = float;
  
  // 创建具有几何一致性的测试数据 / Create test data with geometric consistency
  auto source_cloud = std::make_shared<point_cloud_t<DataType>>();
  auto target_cloud = std::make_shared<point_cloud_t<DataType>>();
  auto correspondences = std::make_shared<std::vector<correspondence_t>>();

  // 创建一个简单的变换（平移） / Create a simple transformation (translation)
  DataType tx = 1.0, ty = 2.0, tz = 3.0;

  // 创建对应的点对 / Create corresponding point pairs
  for (int i = 0; i < 20; ++i) {
    point_t<DataType> src_pt;
    src_pt.x = i * 0.5f;
    src_pt.y = i * 0.3f;
    src_pt.z = i * 0.2f;
    source_cloud->points.push_back(src_pt);

    point_t<DataType> tgt_pt;
    tgt_pt.x = src_pt.x + tx;
    tgt_pt.y = src_pt.y + ty;
    tgt_pt.z = src_pt.z + tz;
    target_cloud->points.push_back(tgt_pt);

    correspondence_t corr;
    corr.src_idx = i;
    corr.dst_idx = i;
    corr.distance = 0.1f + i * 0.01f;  // 任意距离值 / Arbitrary distance values
    correspondences->push_back(corr);
  }

  // 添加一些外点 / Add some outliers
  for (int i = 20; i < 25; ++i) {
    point_t<DataType> src_pt;
    src_pt.x = i * 0.5f;
    src_pt.y = i * 0.3f;
    src_pt.z = i * 0.2f;
    source_cloud->points.push_back(src_pt);

    point_t<DataType> tgt_pt;
    tgt_pt.x = src_pt.x + tx + 5.0f;  // 错误的变换 / Wrong transformation
    tgt_pt.y = src_pt.y + ty - 3.0f;
    tgt_pt.z = src_pt.z + tz + 2.0f;
    target_cloud->points.push_back(tgt_pt);

    correspondence_t corr;
    corr.src_idx = i;
    corr.dst_idx = i;
    corr.distance = 0.5f;
    correspondences->push_back(corr);
  }

  SECTION("基本功能测试 / Basic functionality test")
  {
    geometric_consistency_sorter_t<DataType> sorter;
    sorter.set_point_clouds(source_cloud, target_cloud);
    sorter.set_correspondences(correspondences);
    sorter.set_neighborhood_size(5);
    sorter.set_distance_ratio_threshold(0.1f);

    std::vector<DataType> scores;
    auto sorted_indices = sorter.compute_sorted_indices(scores);

    REQUIRE(sorted_indices.size() == correspondences->size());
    REQUIRE(scores.size() == correspondences->size());

    // 前20个（内点）应该有更高的分数 / First 20 (inliers) should have higher scores
    DataType avg_inlier_score = 0;
    DataType avg_outlier_score = 0;
    
    for (int i = 0; i < 20; ++i) {
      avg_inlier_score += scores[i];
    }
    for (int i = 20; i < 25; ++i) {
      avg_outlier_score += scores[i];
    }
    
    avg_inlier_score /= 20;
    avg_outlier_score /= 5;

    // 内点平均分数应该高于外点 / Inlier average score should be higher than outlier
    REQUIRE(avg_inlier_score > avg_outlier_score);
  }

  SECTION("采样策略测试 / Sampling strategy test")
  {
    geometric_consistency_sorter_t<DataType> sorter;
    sorter.set_point_clouds(source_cloud, target_cloud);
    sorter.set_correspondences(correspondences);
    
    // 测试随机采样 / Test random sampling
    sorter.set_random_sampling(true);
    std::vector<DataType> scores1;
    sorter.compute_sorted_indices(scores1);

    // 测试最近邻采样 / Test nearest neighbor sampling  
    sorter.clear_cache();
    sorter.set_random_sampling(false);
    std::vector<DataType> scores2;
    sorter.compute_sorted_indices(scores2);

    // 两种策略应该产生不同但合理的结果 / Both strategies should produce different but reasonable results
    REQUIRE(!scores1.empty());
    REQUIRE(!scores2.empty());
  }
}

TEST_CASE("CombinedSorter", "[correspondence][sorter]")
{
  using DataType = float;
  
  std::shared_ptr<point_cloud_t<DataType>> source_cloud, target_cloud;
  std::shared_ptr<std::vector<correspondence_t>> correspondences;
  create_test_data<DataType>(source_cloud, correspondences);
  create_test_data<DataType>(target_cloud, correspondences);

  SECTION("组合排序器测试 / Combined sorter test")
  {
    combined_sorter_t<DataType> combined_sorter;

    // 添加描述子距离排序器 / Add descriptor distance sorter
    auto dist_sorter = std::make_shared<descriptor_distance_sorter_t<DataType>>();
    dist_sorter->set_invert_score(true);
    combined_sorter.add_sorter(dist_sorter, 0.7f);

    // 添加几何一致性排序器 / Add geometric consistency sorter
    auto geom_sorter = std::make_shared<geometric_consistency_sorter_t<DataType>>();
    geom_sorter->set_neighborhood_size(3);
    combined_sorter.add_sorter(geom_sorter, 0.3f);

    // 归一化权重 / Normalize weights
    combined_sorter.normalize_weights();

    // 设置数据 / Set data
    combined_sorter.set_point_clouds(source_cloud, target_cloud);
    combined_sorter.set_correspondences(correspondences);

    std::vector<DataType> scores;
    auto sorted_indices = combined_sorter.compute_sorted_indices(scores);

    REQUIRE(sorted_indices.size() == correspondences->size());
    REQUIRE(scores.size() == correspondences->size());

    // 检查权重 / Check weights
    REQUIRE(combined_sorter.get_num_sorters() == 2);
    REQUIRE_THAT(combined_sorter.get_weight(0), WithinAbs(0.7f, 0.001f));
    REQUIRE_THAT(combined_sorter.get_weight(1), WithinAbs(0.3f, 0.001f));

    // 检查排序器名称 / Check sorter name
    std::string name = combined_sorter.get_sorter_name();
    REQUIRE(name.find("Combined") != std::string::npos);
  }

  SECTION("清除排序器测试 / Clear sorters test")
  {
    combined_sorter_t<DataType> combined_sorter;
    
    auto dist_sorter = std::make_shared<descriptor_distance_sorter_t<DataType>>();
    combined_sorter.add_sorter(dist_sorter, 1.0f);
    
    REQUIRE(combined_sorter.get_num_sorters() == 1);
    
    combined_sorter.clear_sorters();
    REQUIRE(combined_sorter.get_num_sorters() == 0);
  }
}

TEST_CASE("CustomFunctionSorter", "[correspondence][sorter]")
{
  using DataType = float;
  
  std::shared_ptr<point_cloud_t<DataType>> source_cloud, target_cloud;
  std::shared_ptr<std::vector<correspondence_t>> correspondences;
  create_test_data<DataType>(source_cloud, correspondences);
  create_test_data<DataType>(target_cloud, correspondences);

  SECTION("完整质量函数测试 / Full quality function test")
  {
    custom_function_sorter_t<DataType> sorter;
    
    // 设置自定义质量函数 / Set custom quality function
    sorter.set_quality_function(
        [](const correspondence_t& corr, std::size_t index,
           const point_cloud_t<DataType>& src,
           const point_cloud_t<DataType>& tgt) -> DataType {
          // 示例：基于索引和距离的组合 / Example: combination of index and distance
          return static_cast<DataType>(index) * 0.1f + (1.0f - corr.distance / 10.0f);
        });

    sorter.set_point_clouds(source_cloud, target_cloud);
    sorter.set_correspondences(correspondences);

    std::vector<DataType> scores;
    auto sorted_indices = sorter.compute_sorted_indices(scores);

    REQUIRE(sorted_indices.size() == correspondences->size());
    REQUIRE(scores.size() == correspondences->size());
    REQUIRE(sorter.has_quality_function());
  }

  SECTION("简化质量函数测试 / Simple quality function test")
  {
    custom_function_sorter_t<DataType> sorter;
    
    // 设置简化的质量函数 / Set simplified quality function
    sorter.set_simple_quality_function(
        [](const correspondence_t& corr) -> DataType {
          // 简单的反距离函数 / Simple inverse distance function
          return 1.0f / (1.0f + corr.distance);
        });

    sorter.set_correspondences(correspondences);

    std::vector<DataType> scores;
    auto sorted_indices = sorter.compute_sorted_indices(scores);

    REQUIRE(sorted_indices.size() == correspondences->size());
    REQUIRE(sorter.has_quality_function());

    // 检查分数递减 / Check scores are decreasing
    for (std::size_t i = 1; i < scores.size(); ++i) {
      REQUIRE(scores[sorted_indices[i - 1]] >= scores[sorted_indices[i]]);
    }
  }

  SECTION("无质量函数错误处理 / No quality function error handling")
  {
    custom_function_sorter_t<DataType> sorter;
    sorter.set_correspondences(correspondences);

    REQUIRE(!sorter.has_quality_function());

    std::vector<DataType> scores;
    auto sorted_indices = sorter.compute_sorted_indices(scores);

    // 应该返回空结果 / Should return empty results
    REQUIRE(sorted_indices.empty());
  }
}