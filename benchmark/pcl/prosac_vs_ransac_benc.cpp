#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cpp-toolbox/pcl/correspondence/correspondence_sorter.hpp>
#include <cpp-toolbox/pcl/registration/prosac_registration.hpp>
#include <cpp-toolbox/pcl/registration/ransac_registration.hpp>
#include <cpp-toolbox/types/point.hpp>

#include <Eigen/Dense>
#include <memory>
#include <random>
#include <vector>

using namespace toolbox::pcl;
using namespace toolbox::types;

// 创建带有不同外点率的测试数据 / Create test data with different outlier ratios
template<typename DataType>
struct registration_test_data_t
{
  std::shared_ptr<point_cloud_t<DataType>> source_cloud;
  std::shared_ptr<point_cloud_t<DataType>> target_cloud;
  std::shared_ptr<std::vector<correspondence_t>> correspondences;
  Eigen::Matrix<DataType, 4, 4> true_transform;
  std::size_t num_inliers;
  std::size_t num_outliers;
  DataType outlier_ratio;
};

template<typename DataType>
registration_test_data_t<DataType> create_benchmark_data(
    std::size_t num_inliers,
    std::size_t num_outliers,
    DataType noise_level = 0.01,
    bool sort_by_quality = true)
{
  registration_test_data_t<DataType> data;
  data.num_inliers = num_inliers;
  data.num_outliers = num_outliers;
  data.outlier_ratio = static_cast<DataType>(num_outliers) / 
                       static_cast<DataType>(num_inliers + num_outliers);

  data.source_cloud = std::make_shared<point_cloud_t<DataType>>();
  data.target_cloud = std::make_shared<point_cloud_t<DataType>>();
  data.correspondences = std::make_shared<std::vector<correspondence_t>>();

  // 定义真实变换 / Define true transformation
  data.true_transform = Eigen::Matrix<DataType, 4, 4>::Identity();
  
  // 旋转：绕Y轴45度 / Rotation: 45 degrees around Y axis
  DataType angle = M_PI / 4;
  data.true_transform(0, 0) = std::cos(angle);
  data.true_transform(0, 2) = std::sin(angle);
  data.true_transform(2, 0) = -std::sin(angle);
  data.true_transform(2, 2) = std::cos(angle);
  
  // 平移 / Translation
  data.true_transform(0, 3) = 2.0;
  data.true_transform(1, 3) = -1.5;
  data.true_transform(2, 3) = 3.0;

  std::random_device rd;
  std::mt19937 gen(42);  // 固定种子以获得可重复的结果 / Fixed seed for reproducible results
  std::uniform_real_distribution<DataType> coord_dist(-10.0, 10.0);
  std::uniform_real_distribution<DataType> noise_dist(-noise_level, noise_level);

  // 创建内点 / Create inliers
  for (std::size_t i = 0; i < num_inliers; ++i) {
    // 源点 / Source point
    point_t<DataType> src_pt;
    src_pt.x = coord_dist(gen);
    src_pt.y = coord_dist(gen);
    src_pt.z = coord_dist(gen);
    data.source_cloud->points.push_back(src_pt);

    // 变换后的目标点 / Transformed target point
    Eigen::Vector4d src_vec(src_pt.x, src_pt.y, src_pt.z, 1.0);
    Eigen::Vector4d tgt_vec = data.true_transform.template cast<double>() * src_vec;
    
    point_t<DataType> tgt_pt;
    tgt_pt.x = tgt_vec[0] + noise_dist(gen);
    tgt_pt.y = tgt_vec[1] + noise_dist(gen);
    tgt_pt.z = tgt_vec[2] + noise_dist(gen);
    data.target_cloud->points.push_back(tgt_pt);

    correspondence_t corr;
    corr.src_idx = i;
    corr.dst_idx = i;
    // 内点有较小的描述子距离 / Inliers have smaller descriptor distances
    corr.distance = 0.05f + (sort_by_quality ? i * 0.001f : coord_dist(gen) * 0.01f);
    data.correspondences->push_back(corr);
  }

  // 创建外点 / Create outliers
  for (std::size_t i = 0; i < num_outliers; ++i) {
    // 随机源点 / Random source point
    point_t<DataType> src_pt;
    src_pt.x = coord_dist(gen);
    src_pt.y = coord_dist(gen);
    src_pt.z = coord_dist(gen);
    data.source_cloud->points.push_back(src_pt);

    // 随机目标点 / Random target point
    point_t<DataType> tgt_pt;
    tgt_pt.x = coord_dist(gen);
    tgt_pt.y = coord_dist(gen);
    tgt_pt.z = coord_dist(gen);
    data.target_cloud->points.push_back(tgt_pt);

    correspondence_t corr;
    corr.src_idx = num_inliers + i;
    corr.dst_idx = num_inliers + i;
    // 外点有较大的描述子距离 / Outliers have larger descriptor distances
    corr.distance = 1.0f + (sort_by_quality ? i * 0.05f : coord_dist(gen) * 0.1f);
    data.correspondences->push_back(corr);
  }

  // 如果需要打乱对应关系顺序 / Shuffle correspondences if needed
  if (!sort_by_quality) {
    std::shuffle(data.correspondences->begin(), data.correspondences->end(), gen);
  }

  return data;
}

TEST_CASE("PROSAC vs RANSAC Performance Benchmark", "[pcl][benchmark]")
{
  using DataType = float;

  // 测试不同的外点率 / Test different outlier ratios
  std::vector<std::pair<std::size_t, std::size_t>> test_configs = {
      {100, 100},   // 50% outliers
      {50, 150},    // 75% outliers
      {30, 270},    // 90% outliers
      {20, 380},    // 95% outliers
      {10, 490}     // 98% outliers
  };

  for (const auto& [num_inliers, num_outliers] : test_configs) {
    DataType outlier_ratio = static_cast<DataType>(num_outliers) / 
                             static_cast<DataType>(num_inliers + num_outliers);
    
    std::string section_name = "Outlier ratio: " + 
                               std::to_string(static_cast<int>(outlier_ratio * 100)) + "%";
    
    SECTION(section_name)
    {
      // 创建测试数据 / Create test data
      auto data = create_benchmark_data<DataType>(num_inliers, num_outliers, 0.01f, true);
      
      // 排序对应关系用于PROSAC / Sort correspondences for PROSAC
      descriptor_distance_sorter_t<DataType> sorter;
      sorter.set_correspondences(data.correspondences);
      sorter.set_invert_score(true);
      
      std::vector<DataType> quality_scores;
      auto sorted_indices = sorter.compute_sorted_indices(quality_scores);

      // 配置参数 / Configure parameters
      const DataType inlier_threshold = 0.05f;
      const DataType confidence = 0.99f;
      const std::size_t max_iterations = 10000;

      BENCHMARK("PROSAC")
      {
        prosac_registration_t<DataType> prosac;
        prosac.set_source(data.source_cloud);
        prosac.set_target(data.target_cloud);
        prosac.set_sorted_correspondences(data.correspondences, sorted_indices);
        prosac.set_max_iterations(max_iterations);
        prosac.set_inlier_threshold(inlier_threshold);
        prosac.set_confidence(confidence);

        registration_result_t<DataType> result;
        bool success = prosac.align(result);
        
        return std::make_pair(success, result.num_iterations);
      };

      BENCHMARK("RANSAC")
      {
        ransac_registration_t<DataType> ransac;
        ransac.set_source(data.source_cloud);
        ransac.set_target(data.target_cloud);
        ransac.set_correspondences(data.correspondences);
        ransac.set_max_iterations(max_iterations);
        ransac.set_inlier_threshold(inlier_threshold);
        ransac.set_confidence(confidence);

        registration_result_t<DataType> result;
        bool success = ransac.align(result);
        
        return std::make_pair(success, result.num_iterations);
      };
    }
  }
}

TEST_CASE("PROSAC Quality Ordering Impact", "[pcl][benchmark]")
{
  using DataType = float;
  
  const std::size_t num_inliers = 30;
  const std::size_t num_outliers = 270;  // 90% outliers

  SECTION("With quality ordering")
  {
    auto data = create_benchmark_data<DataType>(num_inliers, num_outliers, 0.01f, true);
    
    descriptor_distance_sorter_t<DataType> sorter;
    sorter.set_correspondences(data.correspondences);
    sorter.set_invert_score(true);
    
    std::vector<DataType> quality_scores;
    auto sorted_indices = sorter.compute_sorted_indices(quality_scores);

    BENCHMARK("PROSAC with quality ordering")
    {
      prosac_registration_t<DataType> prosac;
      prosac.set_source(data.source_cloud);
      prosac.set_target(data.target_cloud);
      prosac.set_sorted_correspondences(data.correspondences, sorted_indices);
      prosac.set_max_iterations(5000);
      prosac.set_inlier_threshold(0.05f);

      registration_result_t<DataType> result;
      prosac.align(result);
      
      return result.num_iterations;
    };
  }

  SECTION("Without quality ordering (random)")
  {
    auto data = create_benchmark_data<DataType>(num_inliers, num_outliers, 0.01f, false);

    BENCHMARK("PROSAC without quality ordering")
    {
      prosac_registration_t<DataType> prosac;
      prosac.set_source(data.source_cloud);
      prosac.set_target(data.target_cloud);
      prosac.set_sorted_correspondences(data.correspondences);  // 未排序 / Not sorted
      prosac.set_max_iterations(5000);
      prosac.set_inlier_threshold(0.05f);

      registration_result_t<DataType> result;
      prosac.align(result);
      
      return result.num_iterations;
    };
  }
}

TEST_CASE("Correspondence Sorter Performance", "[pcl][benchmark]")
{
  using DataType = float;
  
  // 创建不同规模的对应关系 / Create correspondences of different scales
  std::vector<std::size_t> correspondence_counts = {100, 500, 1000, 5000, 10000};
  
  for (auto count : correspondence_counts) {
    std::string section_name = std::to_string(count) + " correspondences";
    
    SECTION(section_name)
    {
      auto data = create_benchmark_data<DataType>(count / 2, count / 2, 0.01f, false);

      BENCHMARK("Descriptor Distance Sorter")
      {
        descriptor_distance_sorter_t<DataType> sorter;
        sorter.set_correspondences(data.correspondences);
        sorter.set_invert_score(true);
        
        std::vector<DataType> scores;
        return sorter.compute_sorted_indices(scores);
      };

      BENCHMARK("Geometric Consistency Sorter")
      {
        geometric_consistency_sorter_t<DataType> sorter;
        sorter.set_point_clouds(data.source_cloud, data.target_cloud);
        sorter.set_correspondences(data.correspondences);
        sorter.set_neighborhood_size(10);
        sorter.enable_parallel(true);
        
        std::vector<DataType> scores;
        return sorter.compute_sorted_indices(scores);
      };

      BENCHMARK("Combined Sorter (0.7 desc + 0.3 geom)")
      {
        combined_sorter_t<DataType> combined;
        
        auto dist_sorter = std::make_shared<descriptor_distance_sorter_t<DataType>>();
        dist_sorter->set_invert_score(true);
        combined.add_sorter(dist_sorter, 0.7f);
        
        auto geom_sorter = std::make_shared<geometric_consistency_sorter_t<DataType>>();
        geom_sorter->set_neighborhood_size(5);
        combined.add_sorter(geom_sorter, 0.3f);
        
        combined.set_point_clouds(data.source_cloud, data.target_cloud);
        combined.set_correspondences(data.correspondences);
        
        std::vector<DataType> scores;
        return combined.compute_sorted_indices(scores);
      };
    }
  }
}

TEST_CASE("PROSAC Parameter Sensitivity", "[pcl][benchmark]")
{
  using DataType = float;
  
  // 固定的测试数据 / Fixed test data
  auto data = create_benchmark_data<DataType>(25, 225, 0.01f, true);  // 90% outliers
  
  descriptor_distance_sorter_t<DataType> sorter;
  sorter.set_correspondences(data.correspondences);
  sorter.set_invert_score(true);
  
  std::vector<DataType> quality_scores;
  auto sorted_indices = sorter.compute_sorted_indices(quality_scores);

  SECTION("Initial inlier ratio impact")
  {
    BENCHMARK("Initial inlier ratio: 0.05")
    {
      prosac_registration_t<DataType> prosac;
      prosac.set_source(data.source_cloud);
      prosac.set_target(data.target_cloud);
      prosac.set_sorted_correspondences(data.correspondences, sorted_indices);
      prosac.set_max_iterations(5000);
      prosac.set_inlier_threshold(0.05f);
      prosac.set_initial_inlier_ratio(0.05f);

      registration_result_t<DataType> result;
      prosac.align(result);
      
      return result.num_iterations;
    };
    
    BENCHMARK("Initial inlier ratio: 0.1")
    {
      prosac_registration_t<DataType> prosac;
      prosac.set_source(data.source_cloud);
      prosac.set_target(data.target_cloud);
      prosac.set_sorted_correspondences(data.correspondences, sorted_indices);
      prosac.set_max_iterations(5000);
      prosac.set_inlier_threshold(0.05f);
      prosac.set_initial_inlier_ratio(0.1f);

      registration_result_t<DataType> result;
      prosac.align(result);
      
      return result.num_iterations;
    };
    
    BENCHMARK("Initial inlier ratio: 0.3")
    {
      prosac_registration_t<DataType> prosac;
      prosac.set_source(data.source_cloud);
      prosac.set_target(data.target_cloud);
      prosac.set_sorted_correspondences(data.correspondences, sorted_indices);
      prosac.set_max_iterations(5000);
      prosac.set_inlier_threshold(0.05f);
      prosac.set_initial_inlier_ratio(0.3f);

      registration_result_t<DataType> result;
      prosac.align(result);
      
      return result.num_iterations;
    };
  }

  SECTION("Non-randomness threshold impact")
  {
    BENCHMARK("Non-randomness threshold: 0.01")
    {
      prosac_registration_t<DataType> prosac;
      prosac.set_source(data.source_cloud);
      prosac.set_target(data.target_cloud);
      prosac.set_sorted_correspondences(data.correspondences, sorted_indices);
      prosac.set_max_iterations(5000);
      prosac.set_inlier_threshold(0.05f);
      prosac.set_non_randomness_threshold(0.01f);

      registration_result_t<DataType> result;
      prosac.align(result);
      
      return result.num_iterations;
    };
    
    BENCHMARK("Non-randomness threshold: 0.05")
    {
      prosac_registration_t<DataType> prosac;
      prosac.set_source(data.source_cloud);
      prosac.set_target(data.target_cloud);
      prosac.set_sorted_correspondences(data.correspondences, sorted_indices);
      prosac.set_max_iterations(5000);
      prosac.set_inlier_threshold(0.05f);
      prosac.set_non_randomness_threshold(0.05f);

      registration_result_t<DataType> result;
      prosac.align(result);
      
      return result.num_iterations;
    };
  }
}