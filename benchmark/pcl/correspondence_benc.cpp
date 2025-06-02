#include <chrono>
#include <random>

#include <catch2/catch_all.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence_generator.hpp>  // For backward compatibility
#include <cpp-toolbox/pcl/descriptors/descriptors.hpp>
#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/pfh_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/shot_extractor.hpp>
#include <cpp-toolbox/pcl/knn/bfknn.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/types/point.hpp>

// 描述子度量类 / Descriptor metric classes
namespace toolbox::metrics
{
template<typename T>
struct FPFHMetric
{
  using value_type = T;
  using result_type = T;

  T operator()(const toolbox::pcl::fpfh_signature_t<T>& a,
               const toolbox::pcl::fpfh_signature_t<T>& b) const
  {
    return a.distance(b);
  }

  T distance(const toolbox::pcl::fpfh_signature_t<T>& a,
             const toolbox::pcl::fpfh_signature_t<T>& b) const
  {
    return a.distance(b);
  }
};

template<typename T>
struct PFHMetric
{
  using value_type = T;
  using result_type = T;

  T operator()(const toolbox::pcl::pfh_signature_t<T>& a,
               const toolbox::pcl::pfh_signature_t<T>& b) const
  {
    return a.distance(b);
  }

  T distance(const toolbox::pcl::pfh_signature_t<T>& a,
             const toolbox::pcl::pfh_signature_t<T>& b) const
  {
    return a.distance(b);
  }
};

template<typename T>
struct SHOTMetric
{
  using value_type = T;
  using result_type = T;

  T operator()(const toolbox::pcl::shot_signature_t<T>& a,
               const toolbox::pcl::shot_signature_t<T>& b) const
  {
    return a.distance(b);
  }

  T distance(const toolbox::pcl::shot_signature_t<T>& a,
             const toolbox::pcl::shot_signature_t<T>& b) const
  {
    return a.distance(b);
  }
};

}  // namespace toolbox::metrics

using namespace toolbox::pcl;
using namespace toolbox::types;

// 创建测试点云和FPFH描述子 / Create test point cloud and FPFH descriptors
template<typename T>
std::pair<std::shared_ptr<std::vector<fpfh_signature_t<T>>>,
          std::shared_ptr<std::vector<std::size_t>>>
create_test_fpfh_descriptors(size_t num_descriptors, std::mt19937& rng)
{
  auto descriptors = std::make_shared<std::vector<fpfh_signature_t<T>>>();
  auto indices = std::make_shared<std::vector<std::size_t>>();

  std::uniform_real_distribution<T> dist(0.0, 1.0);

  for (size_t i = 0; i < num_descriptors; ++i) {
    fpfh_signature_t<T> desc;

    // 创建随机描述子 / Create random descriptor
    for (int j = 0; j < 33; ++j) {
      desc.histogram[j] = dist(rng);
    }

    // 归一化 / Normalize
    T sum = 0;
    for (int j = 0; j < 33; ++j) {
      sum += desc.histogram[j];
    }
    if (sum > 0) {
      for (int j = 0; j < 33; ++j) {
        desc.histogram[j] /= sum;
      }
    }

    descriptors->push_back(desc);
    indices->push_back(i * 10);  // 假设关键点索引 / Assume keypoint indices
  }

  return {descriptors, indices};
}

// 创建测试PFH描述子 / Create test PFH descriptors
template<typename T>
std::pair<std::shared_ptr<std::vector<pfh_signature_t<T>>>,
          std::shared_ptr<std::vector<std::size_t>>>
create_test_pfh_descriptors(size_t num_descriptors, std::mt19937& rng)
{
  auto descriptors = std::make_shared<std::vector<pfh_signature_t<T>>>();
  auto indices = std::make_shared<std::vector<std::size_t>>();

  std::uniform_real_distribution<T> dist(0.0, 1.0);

  for (size_t i = 0; i < num_descriptors; ++i) {
    pfh_signature_t<T> desc;

    // 创建随机描述子 / Create random descriptor (125 dimensions)
    for (int j = 0; j < 125; ++j) {
      desc.histogram[j] = dist(rng);
    }

    // 归一化 / Normalize
    T sum = 0;
    for (int j = 0; j < 125; ++j) {
      sum += desc.histogram[j];
    }
    if (sum > 0) {
      for (int j = 0; j < 125; ++j) {
        desc.histogram[j] /= sum;
      }
    }

    descriptors->push_back(desc);
    indices->push_back(i * 10);
  }

  return {descriptors, indices};
}

// 创建测试SHOT描述子 / Create test SHOT descriptors
template<typename T>
std::pair<std::shared_ptr<std::vector<shot_signature_t<T>>>,
          std::shared_ptr<std::vector<std::size_t>>>
create_test_shot_descriptors(size_t num_descriptors, std::mt19937& rng)
{
  auto descriptors = std::make_shared<std::vector<shot_signature_t<T>>>();
  auto indices = std::make_shared<std::vector<std::size_t>>();

  std::uniform_real_distribution<T> dist(0.0, 1.0);

  for (size_t i = 0; i < num_descriptors; ++i) {
    shot_signature_t<T> desc;

    // 创建随机描述子 / Create random descriptor (352 dimensions)
    for (int j = 0; j < 352; ++j) {
      desc.histogram[j] = dist(rng);
    }

    // 归一化 / Normalize
    T sum = 0;
    for (int j = 0; j < 352; ++j) {
      sum += desc.histogram[j];
    }
    if (sum > 0) {
      for (int j = 0; j < 352; ++j) {
        desc.histogram[j] /= sum;
      }
    }

    descriptors->push_back(desc);
    indices->push_back(i * 10);
  }

  return {descriptors, indices};
}


TEST_CASE("对应点生成性能比较 / Correspondence Generation Performance Comparison",
          "[pcl][correspondence][benchmark]")
{
  using T = float;

  SECTION("不同描述子类型性能比较 / Performance comparison of different descriptor types")
  {
    const size_t num_desc = 500; // 固定数量进行比较 / Fixed count for comparison
    std::mt19937 rng(42);

    // 创建假的点云 / Create fake point clouds
    auto src_cloud = std::make_shared<point_cloud_t<T>>();
    auto dst_cloud = std::make_shared<point_cloud_t<T>>();
    src_cloud->points.resize(num_desc * 10);
    dst_cloud->points.resize(num_desc * 10);

    SECTION("FPFH描述子性能 / FPFH descriptor performance")
    {
      auto src_data = create_test_fpfh_descriptors<T>(num_desc, rng);
      auto dst_data = create_test_fpfh_descriptors<T>(num_desc, rng);
      auto src_descriptors = src_data.first;
      auto src_indices = src_data.second;
      auto dst_descriptors = dst_data.first;
      auto dst_indices = dst_data.second;

      BENCHMARK("FPFH - KNN方法 / FPFH - KNN method")
      {
        using CorrespondenceGen = knn_correspondence_generator_t<
            T,
            fpfh_signature_t<T>,
            bfknn_generic_t<fpfh_signature_t<T>,
                            toolbox::metrics::FPFHMetric<T>>>;
        CorrespondenceGen corr_gen;

        auto knn = std::make_shared<
            bfknn_generic_t<fpfh_signature_t<T>,
                            toolbox::metrics::FPFHMetric<T>>>();
        corr_gen.set_knn(knn);
        corr_gen.set_source(src_cloud, src_descriptors, src_indices);
        corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
        corr_gen.set_ratio(0.8f);
        corr_gen.set_mutual_verification(true);

        std::vector<correspondence_t> correspondences;
        corr_gen.compute(correspondences);

        return correspondences.size();
      };

      BENCHMARK("FPFH - 暴力搜索（串行） / FPFH - Brute-force (serial)")
      {
        brute_force_correspondence_generator_t<T, fpfh_signature_t<T>>
            corr_gen;

        corr_gen.enable_parallel(false);
        corr_gen.set_source(src_cloud, src_descriptors, src_indices);
        corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
        corr_gen.set_ratio(0.8f);
        corr_gen.set_mutual_verification(true);

        std::vector<correspondence_t> correspondences;
        corr_gen.compute(correspondences);

        return correspondences.size();
      };

      BENCHMARK("FPFH - 暴力搜索（并行） / FPFH - Brute-force (parallel)")
      {
        brute_force_correspondence_generator_t<T, fpfh_signature_t<T>>
            corr_gen;

        corr_gen.enable_parallel(true);
        corr_gen.set_source(src_cloud, src_descriptors, src_indices);
        corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
        corr_gen.set_ratio(0.8f);
        corr_gen.set_mutual_verification(true);

        std::vector<correspondence_t> correspondences;
        corr_gen.compute(correspondences);

        return correspondences.size();
      };
    }

    SECTION("PFH描述子性能 / PFH descriptor performance")
    {
      auto src_data = create_test_pfh_descriptors<T>(num_desc, rng);
      auto dst_data = create_test_pfh_descriptors<T>(num_desc, rng);
      auto src_descriptors = src_data.first;
      auto src_indices = src_data.second;
      auto dst_descriptors = dst_data.first;
      auto dst_indices = dst_data.second;

      BENCHMARK("PFH - KNN方法 / PFH - KNN method")
      {
        using CorrespondenceGen = knn_correspondence_generator_t<
            T,
            pfh_signature_t<T>,
            bfknn_generic_t<pfh_signature_t<T>,
                            toolbox::metrics::PFHMetric<T>>>;
        CorrespondenceGen corr_gen;

        auto knn = std::make_shared<
            bfknn_generic_t<pfh_signature_t<T>,
                            toolbox::metrics::PFHMetric<T>>>();
        corr_gen.set_knn(knn);
        corr_gen.set_source(src_cloud, src_descriptors, src_indices);
        corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
        corr_gen.set_ratio(0.8f);
        corr_gen.set_mutual_verification(true);

        std::vector<correspondence_t> correspondences;
        corr_gen.compute(correspondences);

        return correspondences.size();
      };

      BENCHMARK("PFH - 暴力搜索（并行） / PFH - Brute-force (parallel)")
      {
        brute_force_correspondence_generator_t<T, pfh_signature_t<T>>
            corr_gen;

        corr_gen.enable_parallel(true);
        corr_gen.set_source(src_cloud, src_descriptors, src_indices);
        corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
        corr_gen.set_ratio(0.8f);
        corr_gen.set_mutual_verification(true);

        std::vector<correspondence_t> correspondences;
        corr_gen.compute(correspondences);

        return correspondences.size();
      };
    }

    SECTION("SHOT描述子性能 / SHOT descriptor performance")
    {
      auto src_data = create_test_shot_descriptors<T>(num_desc, rng);
      auto dst_data = create_test_shot_descriptors<T>(num_desc, rng);
      auto src_descriptors = src_data.first;
      auto src_indices = src_data.second;
      auto dst_descriptors = dst_data.first;
      auto dst_indices = dst_data.second;

      BENCHMARK("SHOT - KNN方法 / SHOT - KNN method")
      {
        using CorrespondenceGen = knn_correspondence_generator_t<
            T,
            shot_signature_t<T>,
            bfknn_generic_t<shot_signature_t<T>,
                            toolbox::metrics::SHOTMetric<T>>>;
        CorrespondenceGen corr_gen;

        auto knn = std::make_shared<
            bfknn_generic_t<shot_signature_t<T>,
                            toolbox::metrics::SHOTMetric<T>>>();
        corr_gen.set_knn(knn);
        corr_gen.set_source(src_cloud, src_descriptors, src_indices);
        corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
        corr_gen.set_ratio(0.8f);
        corr_gen.set_mutual_verification(true);

        std::vector<correspondence_t> correspondences;
        corr_gen.compute(correspondences);

        return correspondences.size();
      };

      BENCHMARK("SHOT - 暴力搜索（并行） / SHOT - Brute-force (parallel)")
      {
        brute_force_correspondence_generator_t<T, shot_signature_t<T>>
            corr_gen;

        corr_gen.enable_parallel(true);
        corr_gen.set_source(src_cloud, src_descriptors, src_indices);
        corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
        corr_gen.set_ratio(0.8f);
        corr_gen.set_mutual_verification(true);

        std::vector<correspondence_t> correspondences;
        corr_gen.compute(correspondences);

        return correspondences.size();
      };
    }

  }

  SECTION("不同描述子数量的性能影响 / Performance impact of different descriptor counts")
  {
    const std::vector<size_t> descriptor_counts = {50, 100, 200, 500, 1000, 2000};

    for (size_t num_desc : descriptor_counts) {
      DYNAMIC_SECTION("FPFH描述子数量 / FPFH descriptor count: " << num_desc)
      {
        // 准备测试数据 / Prepare test data
        std::mt19937 rng(42);
        auto src_data = create_test_fpfh_descriptors<T>(num_desc, rng);
        auto dst_data = create_test_fpfh_descriptors<T>(num_desc, rng);
        auto src_descriptors = src_data.first;
        auto src_indices = src_data.second;
        auto dst_descriptors = dst_data.first;
        auto dst_indices = dst_data.second;

        // 创建假的点云 / Create fake point clouds
        auto src_cloud = std::make_shared<point_cloud_t<T>>();
        auto dst_cloud = std::make_shared<point_cloud_t<T>>();
        src_cloud->points.resize(num_desc * 10);
        dst_cloud->points.resize(num_desc * 10);

        // KNN方法 / KNN method
        BENCHMARK("FPFH - KNN方法 / FPFH - KNN method")
        {
          using CorrespondenceGen = knn_correspondence_generator_t<
              T,
              fpfh_signature_t<T>,
              bfknn_generic_t<fpfh_signature_t<T>,
                              toolbox::metrics::FPFHMetric<T>>>;
          CorrespondenceGen corr_gen;

          auto knn = std::make_shared<
              bfknn_generic_t<fpfh_signature_t<T>,
                              toolbox::metrics::FPFHMetric<T>>>();
          corr_gen.set_knn(knn);
          corr_gen.set_source(src_cloud, src_descriptors, src_indices);
          corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
          corr_gen.set_ratio(0.8f);
          corr_gen.set_mutual_verification(true);

          std::vector<correspondence_t> correspondences;
          corr_gen.compute(correspondences);

          return correspondences.size();
        };

        // 暴力搜索方法（并行） / Brute-force method (parallel)
        BENCHMARK("FPFH - 暴力搜索（并行） / FPFH - Brute-force (parallel)")
        {
          brute_force_correspondence_generator_t<T, fpfh_signature_t<T>>
              corr_gen;

          corr_gen.enable_parallel(true);
          corr_gen.set_source(src_cloud, src_descriptors, src_indices);
          corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
          corr_gen.set_ratio(0.8f);
          corr_gen.set_mutual_verification(true);

          std::vector<correspondence_t> correspondences;
          corr_gen.compute(correspondences);

          return correspondences.size();
        };
      }

      // 同样为其他描述子添加数量测试 / Add quantity tests for other descriptors
      DYNAMIC_SECTION("PFH描述子数量 / PFH descriptor count: " << num_desc)
      {
        std::mt19937 rng(42);
        auto src_data = create_test_pfh_descriptors<T>(num_desc, rng);
        auto dst_data = create_test_pfh_descriptors<T>(num_desc, rng);
        auto src_descriptors = src_data.first;
        auto src_indices = src_data.second;
        auto dst_descriptors = dst_data.first;
        auto dst_indices = dst_data.second;

        auto src_cloud = std::make_shared<point_cloud_t<T>>();
        auto dst_cloud = std::make_shared<point_cloud_t<T>>();
        src_cloud->points.resize(num_desc * 10);
        dst_cloud->points.resize(num_desc * 10);

        BENCHMARK("PFH - 暴力搜索（并行） / PFH - Brute-force (parallel)")
        {
          brute_force_correspondence_generator_t<T, pfh_signature_t<T>>
              corr_gen;

          corr_gen.enable_parallel(true);
          corr_gen.set_source(src_cloud, src_descriptors, src_indices);
          corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
          corr_gen.set_ratio(0.8f);
          corr_gen.set_mutual_verification(true);

          std::vector<correspondence_t> correspondences;
          corr_gen.compute(correspondences);

          return correspondences.size();
        };
      }

      DYNAMIC_SECTION("SHOT描述子数量 / SHOT descriptor count: " << num_desc)
      {
        std::mt19937 rng(42);
        auto src_data = create_test_shot_descriptors<T>(num_desc, rng);
        auto dst_data = create_test_shot_descriptors<T>(num_desc, rng);
        auto src_descriptors = src_data.first;
        auto src_indices = src_data.second;
        auto dst_descriptors = dst_data.first;
        auto dst_indices = dst_data.second;

        auto src_cloud = std::make_shared<point_cloud_t<T>>();
        auto dst_cloud = std::make_shared<point_cloud_t<T>>();
        src_cloud->points.resize(num_desc * 10);
        dst_cloud->points.resize(num_desc * 10);

        BENCHMARK("SHOT - 暴力搜索（并行） / SHOT - Brute-force (parallel)")
        {
          brute_force_correspondence_generator_t<T, shot_signature_t<T>>
              corr_gen;

          corr_gen.enable_parallel(true);
          corr_gen.set_source(src_cloud, src_descriptors, src_indices);
          corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
          corr_gen.set_ratio(0.8f);
          corr_gen.set_mutual_verification(true);

          std::vector<correspondence_t> correspondences;
          corr_gen.compute(correspondences);

          return correspondences.size();
        };
      }
    }
  }

  SECTION("参数影响分析 / Parameter effects analysis")
  {
    // 准备固定大小的测试数据 / Prepare fixed-size test data
    const size_t num_desc = 500;
    std::mt19937 rng(42);
    auto src_data = create_test_fpfh_descriptors<T>(num_desc, rng);
    auto dst_data = create_test_fpfh_descriptors<T>(num_desc, rng);
    auto src_descriptors = src_data.first;
    auto src_indices = src_data.second;
    auto dst_descriptors = dst_data.first;
    auto dst_indices = dst_data.second;

    auto src_cloud = std::make_shared<point_cloud_t<T>>();
    auto dst_cloud = std::make_shared<point_cloud_t<T>>();
    src_cloud->points.resize(num_desc * 10);
    dst_cloud->points.resize(num_desc * 10);

    // 测试不同比率阈值 / Test different ratio thresholds
    for (float ratio : {0.6f, 0.7f, 0.8f, 0.9f}) {
      DYNAMIC_SECTION("比率阈值 / Ratio threshold: " << ratio)
      {
        BENCHMARK("FPFH对应点生成 / FPFH correspondence generation")
        {
          using CorrespondenceGen = knn_correspondence_generator_t<
              T,
              fpfh_signature_t<T>,
              bfknn_generic_t<fpfh_signature_t<T>,
                              toolbox::metrics::FPFHMetric<T>>>;
          CorrespondenceGen corr_gen;

          auto knn = std::make_shared<
              bfknn_generic_t<fpfh_signature_t<T>,
                              toolbox::metrics::FPFHMetric<T>>>();
          corr_gen.set_knn(knn);
          corr_gen.set_source(src_cloud, src_descriptors, src_indices);
          corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
          corr_gen.set_ratio(ratio);
          corr_gen.set_mutual_verification(false);

          std::vector<correspondence_t> correspondences;
          corr_gen.compute(correspondences);

          return correspondences.size();
        };
      }
    }

    // 测试双向验证的影响 / Test mutual verification effect
    BENCHMARK("无双向验证 / No mutual verification")
    {
      using CorrespondenceGen = knn_correspondence_generator_t<
          T,
          fpfh_signature_t<T>,
          bfknn_generic_t<fpfh_signature_t<T>,
                          toolbox::metrics::FPFHMetric<T>>>;
      CorrespondenceGen corr_gen;

      auto knn =
          std::make_shared<bfknn_generic_t<fpfh_signature_t<T>,
                                           toolbox::metrics::FPFHMetric<T>>>();
      corr_gen.set_knn(knn);
      corr_gen.set_source(src_cloud, src_descriptors, src_indices);
      corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
      corr_gen.set_ratio(0.8f);
      corr_gen.set_mutual_verification(false);

      std::vector<correspondence_t> correspondences;
      corr_gen.compute(correspondences);

      return correspondences.size();
    };

    BENCHMARK("有双向验证 / With mutual verification")
    {
      using CorrespondenceGen = knn_correspondence_generator_t<
          T,
          fpfh_signature_t<T>,
          bfknn_generic_t<fpfh_signature_t<T>,
                          toolbox::metrics::FPFHMetric<T>>>;
      CorrespondenceGen corr_gen;

      auto knn =
          std::make_shared<bfknn_generic_t<fpfh_signature_t<T>,
                                           toolbox::metrics::FPFHMetric<T>>>();
      corr_gen.set_knn(knn);
      corr_gen.set_source(src_cloud, src_descriptors, src_indices);
      corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
      corr_gen.set_ratio(0.8f);
      corr_gen.set_mutual_verification(true);

      std::vector<correspondence_t> correspondences;
      corr_gen.compute(correspondences);

      return correspondences.size();
    };
  }
}

TEST_CASE("描述子维度对性能的影响 / Impact of descriptor dimensions on performance",
          "[pcl][correspondence][benchmark][dimensions]")
{
  using T = float;
  const size_t num_desc = 300; // 中等数量以便比较 / Medium count for comparison
  std::mt19937 rng(42);

  auto src_cloud = std::make_shared<point_cloud_t<T>>();
  auto dst_cloud = std::make_shared<point_cloud_t<T>>();
  src_cloud->points.resize(num_desc * 10);
  dst_cloud->points.resize(num_desc * 10);

  SECTION("按描述子维度比较 / Comparison by descriptor dimensions")
  {
    // FPFH (33维) / FPFH (33 dimensions)
    {
      auto src_data = create_test_fpfh_descriptors<T>(num_desc, rng);
      auto dst_data = create_test_fpfh_descriptors<T>(num_desc, rng);

      BENCHMARK("FPFH (33维) - 暴力搜索 / FPFH (33D) - Brute-force")
      {
        brute_force_correspondence_generator_t<T, fpfh_signature_t<T>> corr_gen;
        corr_gen.enable_parallel(true);
        corr_gen.set_source(src_cloud, src_data.first, src_data.second);
        corr_gen.set_destination(dst_cloud, dst_data.first, dst_data.second);
        corr_gen.set_ratio(0.8f);
        corr_gen.set_mutual_verification(true);

        std::vector<correspondence_t> correspondences;
        corr_gen.compute(correspondences);
        return correspondences.size();
      };
    }

    // PFH (125维) / PFH (125 dimensions)
    {
      auto src_data = create_test_pfh_descriptors<T>(num_desc, rng);
      auto dst_data = create_test_pfh_descriptors<T>(num_desc, rng);

      BENCHMARK("PFH (125维) - 暴力搜索 / PFH (125D) - Brute-force")
      {
        brute_force_correspondence_generator_t<T, pfh_signature_t<T>> corr_gen;
        corr_gen.enable_parallel(true);
        corr_gen.set_source(src_cloud, src_data.first, src_data.second);
        corr_gen.set_destination(dst_cloud, dst_data.first, dst_data.second);
        corr_gen.set_ratio(0.8f);
        corr_gen.set_mutual_verification(true);

        std::vector<correspondence_t> correspondences;
        corr_gen.compute(correspondences);
        return correspondences.size();
      };
    }

    // SHOT (352维) / SHOT (352 dimensions)
    {
      auto src_data = create_test_shot_descriptors<T>(num_desc, rng);
      auto dst_data = create_test_shot_descriptors<T>(num_desc, rng);

      BENCHMARK("SHOT (352维) - 暴力搜索 / SHOT (352D) - Brute-force")
      {
        brute_force_correspondence_generator_t<T, shot_signature_t<T>> corr_gen;
        corr_gen.enable_parallel(true);
        corr_gen.set_source(src_cloud, src_data.first, src_data.second);
        corr_gen.set_destination(dst_cloud, dst_data.first, dst_data.second);
        corr_gen.set_ratio(0.8f);
        corr_gen.set_mutual_verification(true);

        std::vector<correspondence_t> correspondences;
        corr_gen.compute(correspondences);
        return correspondences.size();
      };
    }

  }
}

TEST_CASE("大规模描述子对应点生成 / Large-scale descriptor correspondence generation",
          "[pcl][correspondence][benchmark][large-scale]")
{
  using T = float;
  const std::vector<size_t> large_counts = {1000, 3000, 5000, 10000};

  for (size_t count : large_counts) {
    DYNAMIC_SECTION("大规模FPFH描述子 / Large-scale FPFH descriptors: " << count)
    {
      std::mt19937 rng(42);
      auto src_data = create_test_fpfh_descriptors<T>(count, rng);
      auto dst_data = create_test_fpfh_descriptors<T>(count, rng);

      auto src_cloud = std::make_shared<point_cloud_t<T>>();
      auto dst_cloud = std::make_shared<point_cloud_t<T>>();
      src_cloud->points.resize(count * 10);
      dst_cloud->points.resize(count * 10);

      BENCHMARK("大规模FPFH对应点生成 / Large-scale FPFH correspondence")
      {
        brute_force_correspondence_generator_t<T, fpfh_signature_t<T>> corr_gen;
        corr_gen.enable_parallel(true);
        corr_gen.set_source(src_cloud, src_data.first, src_data.second);
        corr_gen.set_destination(dst_cloud, dst_data.first, dst_data.second);
        corr_gen.set_ratio(0.8f);
        corr_gen.set_mutual_verification(false); // 大规模时关闭双向验证以提高速度 / Disable mutual verification for large scale

        std::vector<correspondence_t> correspondences;
        corr_gen.compute(correspondences);
        return correspondences.size();
      };
    }
  }
}