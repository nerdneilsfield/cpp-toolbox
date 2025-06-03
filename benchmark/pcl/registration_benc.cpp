#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cpp-toolbox/pcl/registration/ransac_registration.hpp>
#include <cpp-toolbox/pcl/registration/four_pcs_registration.hpp>
#include <cpp-toolbox/pcl/registration/super_four_pcs_registration.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence.hpp>
#include <cpp-toolbox/pcl/features/harris3d_keypoints.hpp>
#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <cpp-toolbox/pcl/filters/random_downsampling.hpp>
#include <cpp-toolbox/utils/random.hpp>

#include <random>

using namespace toolbox::pcl;
using namespace toolbox::types;

// 基准测试数据生成器 / Benchmark data generator
class registration_benchmark_data_t
{
public:
  using DataType = float;
  
  registration_benchmark_data_t(std::size_t num_points, DataType noise_level = 0.001f,
                               DataType outlier_ratio = 0.3f)
      : m_num_points(num_points), m_noise_level(noise_level), m_outlier_ratio(outlier_ratio)
  {
    generate_data();
  }
  
  auto get_source_cloud() const { return m_source_cloud; }
  auto get_target_cloud() const { return m_target_cloud; }
  auto get_correspondences() const { return m_correspondences; }
  const auto& get_ground_truth() const { return m_ground_truth; }
  
private:
  void generate_data()
  {
    // 生成源点云 / Generate source cloud
    m_source_cloud = std::make_shared<point_cloud_t<DataType>>();
    m_source_cloud->points.resize(m_num_points);
    
    std::mt19937 gen(42);
    std::uniform_real_distribution<DataType> pos_dist(-5.0f, 5.0f);
    
    // 创建一些结构化的点（平面、球面等） / Create some structured points (planes, spheres, etc.)
    for (std::size_t i = 0; i < m_num_points; ++i) {
      DataType t = static_cast<DataType>(i) / m_num_points;
      
      if (i < m_num_points / 3) {
        // 平面点 / Plane points
        m_source_cloud->points[i].x = pos_dist(gen);
        m_source_cloud->points[i].y = pos_dist(gen);
        m_source_cloud->points[i].z = 0.1f * pos_dist(gen);
      } else if (i < 2 * m_num_points / 3) {
        // 球面点 / Sphere points
        DataType theta = t * 2 * M_PI;
        DataType phi = t * M_PI;
        DataType r = 3.0f;
        m_source_cloud->points[i].x = r * std::sin(phi) * std::cos(theta);
        m_source_cloud->points[i].y = r * std::sin(phi) * std::sin(theta);
        m_source_cloud->points[i].z = r * std::cos(phi);
      } else {
        // 随机点 / Random points
        m_source_cloud->points[i].x = pos_dist(gen);
        m_source_cloud->points[i].y = pos_dist(gen);
        m_source_cloud->points[i].z = pos_dist(gen);
      }
    }
    
    // 生成变换矩阵 / Generate transformation matrix
    m_ground_truth.setIdentity();
    m_ground_truth.block<3, 3>(0, 0) = 
        Eigen::AngleAxis<DataType>(0.2f, Eigen::Matrix<DataType, 3, 1>(1, 1, 1).normalized()).matrix();
    m_ground_truth.block<3, 1>(0, 3) << 0.5f, 0.3f, 0.2f;
    
    // 变换并添加噪声生成目标点云 / Transform and add noise to generate target cloud
    m_target_cloud = std::make_shared<point_cloud_t<DataType>>();
    m_target_cloud->points.resize(m_num_points);
    
    std::normal_distribution<DataType> noise_dist(0.0f, m_noise_level);
    
    for (std::size_t i = 0; i < m_num_points; ++i) {
      const auto& src_pt = m_source_cloud->points[i];
      Eigen::Matrix<DataType, 4, 1> p;
      p << src_pt.x, src_pt.y, src_pt.z, 1.0f;
      
      Eigen::Matrix<DataType, 4, 1> tp = m_ground_truth * p;
      
      m_target_cloud->points[i].x = tp[0] + noise_dist(gen);
      m_target_cloud->points[i].y = tp[1] + noise_dist(gen);
      m_target_cloud->points[i].z = tp[2] + noise_dist(gen);
    }
    
    // 生成对应关系 / Generate correspondences
    m_correspondences = std::make_shared<std::vector<correspondence_t>>();
    m_correspondences->reserve(m_num_points);
    
    std::uniform_real_distribution<DataType> outlier_dist(0.0f, 1.0f);
    std::uniform_int_distribution<std::size_t> idx_dist(0, m_num_points - 1);
    
    for (std::size_t i = 0; i < m_num_points; ++i) {
      correspondence_t corr;
      corr.src_idx = i;
      
      if (outlier_dist(gen) < m_outlier_ratio) {
        // 外点 / Outlier
        corr.dst_idx = idx_dist(gen);
        corr.distance = outlier_dist(gen) * 10.0f;
      } else {
        // 内点 / Inlier
        corr.dst_idx = i;
        corr.distance = noise_dist(gen) + m_noise_level;
      }
      
      m_correspondences->push_back(corr);
    }
  }
  
  std::size_t m_num_points;
  DataType m_noise_level;
  DataType m_outlier_ratio;
  
  std::shared_ptr<point_cloud_t<DataType>> m_source_cloud;
  std::shared_ptr<point_cloud_t<DataType>> m_target_cloud;
  std::shared_ptr<std::vector<correspondence_t>> m_correspondences;
  Eigen::Matrix<DataType, 4, 4> m_ground_truth;
};

TEST_CASE("Registration Benchmarks - RANSAC", "[pcl][registration][!benchmark]")
{
  using DataType = float;
  
  BENCHMARK_ADVANCED("RANSAC 1K points")(Catch::Benchmark::Chronometer meter)
  {
    registration_benchmark_data_t data(1000, 0.001f, 0.3f);
    
    ransac_registration_t<DataType> ransac;
    ransac.set_source(data.get_source_cloud());
    ransac.set_target(data.get_target_cloud());
    ransac.set_correspondences(data.get_correspondences());
    ransac.set_max_iterations(500);
    ransac.set_inlier_threshold(0.05f);
    ransac.enable_parallel(true);
    
    meter.measure([&] {
      registration_result_t<DataType> result;
      return ransac.align(result);
    });
  };
  
  BENCHMARK_ADVANCED("RANSAC 10K points")(Catch::Benchmark::Chronometer meter)
  {
    registration_benchmark_data_t data(10000, 0.001f, 0.3f);
    
    ransac_registration_t<DataType> ransac;
    ransac.set_source(data.get_source_cloud());
    ransac.set_target(data.get_target_cloud());
    ransac.set_correspondences(data.get_correspondences());
    ransac.set_max_iterations(1000);
    ransac.set_inlier_threshold(0.05f);
    ransac.enable_parallel(true);
    
    meter.measure([&] {
      registration_result_t<DataType> result;
      return ransac.align(result);
    });
  };
  
  BENCHMARK_ADVANCED("RANSAC 50K points")(Catch::Benchmark::Chronometer meter)
  {
    registration_benchmark_data_t data(50000, 0.001f, 0.3f);
    
    ransac_registration_t<DataType> ransac;
    ransac.set_source(data.get_source_cloud());
    ransac.set_target(data.get_target_cloud());
    ransac.set_correspondences(data.get_correspondences());
    ransac.set_max_iterations(2000);
    ransac.set_inlier_threshold(0.05f);
    ransac.enable_parallel(true);
    
    meter.measure([&] {
      registration_result_t<DataType> result;
      return ransac.align(result);
    });
  };
}

TEST_CASE("Registration Benchmarks - 4PCS", "[pcl][registration][!benchmark]")
{
  using DataType = float;
  
  BENCHMARK_ADVANCED("4PCS 500 points")(Catch::Benchmark::Chronometer meter)
  {
    registration_benchmark_data_t data(500, 0.002f, 0.0f);  // 无外点 / No outliers
    
    four_pcs_registration_t<DataType> fourpcs;
    fourpcs.set_source(data.get_source_cloud());
    fourpcs.set_target(data.get_target_cloud());
    fourpcs.set_delta(0.02f);
    fourpcs.set_overlap(0.8f);
    fourpcs.set_sample_size(200);
    fourpcs.set_num_bases(50);
    fourpcs.enable_parallel(true);
    
    meter.measure([&] {
      registration_result_t<DataType> result;
      return fourpcs.align(result);
    });
  };
  
  BENCHMARK_ADVANCED("4PCS 1K points")(Catch::Benchmark::Chronometer meter)
  {
    registration_benchmark_data_t data(1000, 0.002f, 0.0f);
    
    four_pcs_registration_t<DataType> fourpcs;
    fourpcs.set_source(data.get_source_cloud());
    fourpcs.set_target(data.get_target_cloud());
    fourpcs.set_delta(0.02f);
    fourpcs.set_overlap(0.7f);
    fourpcs.set_sample_size(300);
    fourpcs.set_num_bases(100);
    fourpcs.enable_parallel(true);
    
    meter.measure([&] {
      registration_result_t<DataType> result;
      return fourpcs.align(result);
    });
  };
  
  BENCHMARK_ADVANCED("4PCS 5K points")(Catch::Benchmark::Chronometer meter)
  {
    registration_benchmark_data_t data(5000, 0.002f, 0.0f);
    
    four_pcs_registration_t<DataType> fourpcs;
    fourpcs.set_source(data.get_source_cloud());
    fourpcs.set_target(data.get_target_cloud());
    fourpcs.set_delta(0.02f);
    fourpcs.set_overlap(0.6f);
    fourpcs.set_sample_size(500);
    fourpcs.set_num_bases(200);
    fourpcs.enable_parallel(true);
    
    meter.measure([&] {
      registration_result_t<DataType> result;
      return fourpcs.align(result);
    });
  };
}

TEST_CASE("Registration Benchmarks - Super4PCS", "[pcl][registration][!benchmark]")
{
  using DataType = float;
  
  BENCHMARK_ADVANCED("Super4PCS 1K points")(Catch::Benchmark::Chronometer meter)
  {
    registration_benchmark_data_t data(1000, 0.002f, 0.0f);
    
    super_four_pcs_registration_t<DataType> super4pcs;
    super4pcs.set_source(data.get_source_cloud());
    super4pcs.set_target(data.get_target_cloud());
    super4pcs.set_delta(0.02f);
    super4pcs.set_overlap(0.7f);
    super4pcs.set_sample_size(500);
    super4pcs.set_num_bases(100);
    super4pcs.enable_smart_indexing(true);
    super4pcs.enable_parallel(true);
    
    meter.measure([&] {
      registration_result_t<DataType> result;
      return super4pcs.align(result);
    });
  };
  
  BENCHMARK_ADVANCED("Super4PCS 10K points")(Catch::Benchmark::Chronometer meter)
  {
    registration_benchmark_data_t data(10000, 0.002f, 0.0f);
    
    super_four_pcs_registration_t<DataType> super4pcs;
    super4pcs.set_source(data.get_source_cloud());
    super4pcs.set_target(data.get_target_cloud());
    super4pcs.set_delta(0.02f);
    super4pcs.set_overlap(0.6f);
    super4pcs.set_sample_size(1000);
    super4pcs.set_num_bases(200);
    super4pcs.enable_smart_indexing(true);
    super4pcs.enable_parallel(true);
    
    meter.measure([&] {
      registration_result_t<DataType> result;
      return super4pcs.align(result);
    });
  };
  
  BENCHMARK_ADVANCED("Super4PCS 50K points")(Catch::Benchmark::Chronometer meter)
  {
    registration_benchmark_data_t data(50000, 0.002f, 0.0f);
    
    super_four_pcs_registration_t<DataType> super4pcs;
    super4pcs.set_source(data.get_source_cloud());
    super4pcs.set_target(data.get_target_cloud());
    super4pcs.set_delta(0.02f);
    super4pcs.set_overlap(0.5f);
    super4pcs.set_sample_size(2000);
    super4pcs.set_num_bases(500);
    super4pcs.enable_smart_indexing(true);
    super4pcs.enable_parallel(true);
    
    meter.measure([&] {
      registration_result_t<DataType> result;
      return super4pcs.align(result);
    });
  };
}

TEST_CASE("Registration Benchmarks - Comparison", "[pcl][registration][!benchmark]")
{
  using DataType = float;
  
  // 固定数据集用于公平比较 / Fixed dataset for fair comparison
  registration_benchmark_data_t data(5000, 0.002f, 0.2f);
  
  BENCHMARK("RANSAC with correspondences")
  {
    ransac_registration_t<DataType> ransac;
    ransac.set_source(data.get_source_cloud());
    ransac.set_target(data.get_target_cloud());
    ransac.set_correspondences(data.get_correspondences());
    ransac.set_max_iterations(1000);
    ransac.set_inlier_threshold(0.05f);
    ransac.enable_parallel(true);
    
    registration_result_t<DataType> result;
    return ransac.align(result);
  };
  
  BENCHMARK("4PCS without correspondences")
  {
    four_pcs_registration_t<DataType> fourpcs;
    fourpcs.set_source(data.get_source_cloud());
    fourpcs.set_target(data.get_target_cloud());
    fourpcs.set_delta(0.02f);
    fourpcs.set_overlap(0.6f);
    fourpcs.set_sample_size(1000);
    fourpcs.set_num_bases(100);
    fourpcs.enable_parallel(true);
    
    registration_result_t<DataType> result;
    return fourpcs.align(result);
  };
  
  BENCHMARK("Super4PCS with smart indexing")
  {
    super_four_pcs_registration_t<DataType> super4pcs;
    super4pcs.set_source(data.get_source_cloud());
    super4pcs.set_target(data.get_target_cloud());
    super4pcs.set_delta(0.02f);
    super4pcs.set_overlap(0.6f);
    super4pcs.set_sample_size(1000);
    super4pcs.set_num_bases(100);
    super4pcs.enable_smart_indexing(true);
    super4pcs.enable_parallel(true);
    
    registration_result_t<DataType> result;
    return super4pcs.align(result);
  };
}

TEST_CASE("Registration Benchmarks - Parallel vs Sequential", "[pcl][registration][!benchmark]")
{
  using DataType = float;
  
  registration_benchmark_data_t data(10000, 0.001f, 0.3f);
  
  BENCHMARK("RANSAC Sequential")
  {
    ransac_registration_t<DataType> ransac;
    ransac.set_source(data.get_source_cloud());
    ransac.set_target(data.get_target_cloud());
    ransac.set_correspondences(data.get_correspondences());
    ransac.set_max_iterations(500);
    ransac.set_inlier_threshold(0.05f);
    ransac.enable_parallel(false);  // 串行 / Sequential
    
    registration_result_t<DataType> result;
    return ransac.align(result);
  };
  
  BENCHMARK("RANSAC Parallel")
  {
    ransac_registration_t<DataType> ransac;
    ransac.set_source(data.get_source_cloud());
    ransac.set_target(data.get_target_cloud());
    ransac.set_correspondences(data.get_correspondences());
    ransac.set_max_iterations(500);
    ransac.set_inlier_threshold(0.05f);
    ransac.enable_parallel(true);  // 并行 / Parallel
    
    registration_result_t<DataType> result;
    return ransac.align(result);
  };
}

TEST_CASE("Registration Benchmarks - Parameter Sensitivity", "[pcl][registration][!benchmark]")
{
  using DataType = float;
  
  registration_benchmark_data_t data(5000, 0.002f, 0.0f);
  
  BENCHMARK("4PCS low overlap (30%)")
  {
    four_pcs_registration_t<DataType> fourpcs;
    fourpcs.set_source(data.get_source_cloud());
    fourpcs.set_target(data.get_target_cloud());
    fourpcs.set_delta(0.02f);
    fourpcs.set_overlap(0.3f);  // 低重叠 / Low overlap
    fourpcs.set_sample_size(1000);
    fourpcs.set_num_bases(200);
    
    registration_result_t<DataType> result;
    return fourpcs.align(result);
  };
  
  BENCHMARK("4PCS medium overlap (60%)")
  {
    four_pcs_registration_t<DataType> fourpcs;
    fourpcs.set_source(data.get_source_cloud());
    fourpcs.set_target(data.get_target_cloud());
    fourpcs.set_delta(0.02f);
    fourpcs.set_overlap(0.6f);  // 中等重叠 / Medium overlap
    fourpcs.set_sample_size(1000);
    fourpcs.set_num_bases(200);
    
    registration_result_t<DataType> result;
    return fourpcs.align(result);
  };
  
  BENCHMARK("4PCS high overlap (90%)")
  {
    four_pcs_registration_t<DataType> fourpcs;
    fourpcs.set_source(data.get_source_cloud());
    fourpcs.set_target(data.get_target_cloud());
    fourpcs.set_delta(0.02f);
    fourpcs.set_overlap(0.9f);  // 高重叠 / High overlap
    fourpcs.set_sample_size(1000);
    fourpcs.set_num_bases(200);
    
    registration_result_t<DataType> result;
    return fourpcs.align(result);
  };
}