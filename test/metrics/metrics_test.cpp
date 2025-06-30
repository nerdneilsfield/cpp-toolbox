#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <cpp-toolbox/metrics/base_metric.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>
#include <cpp-toolbox/metrics/histogram_metrics.hpp>
#include <cpp-toolbox/metrics/angular_metrics.hpp>
#include <cpp-toolbox/metrics/custom_metric.hpp>
#include <cpp-toolbox/metrics/metric_factory.hpp>
#include <cpp-toolbox/metrics/point_cloud_metrics.hpp>

#include <vector>
#include <cmath>
#include <numeric>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace toolbox::metrics;
using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinAbs;

// Helper function to generate random vectors
template<typename T>
std::vector<T> generate_random_vector(std::size_t size, T min_val = -10, T max_val = 10)
{
  static std::mt19937 rng(42);  // Fixed seed for reproducibility
  std::uniform_real_distribution<T> dist(min_val, max_val);
  
  std::vector<T> vec(size);
  for (auto& v : vec) {
    v = dist(rng);
  }
  return vec;
}

// Helper function to generate random histogram (non-negative values)
template<typename T>
std::vector<T> generate_random_histogram(std::size_t size)
{
  static std::mt19937 rng(42);
  std::uniform_real_distribution<T> dist(0, 10);
  
  std::vector<T> vec(size);
  for (auto& v : vec) {
    v = dist(rng);
  }
  return vec;
}

TEST_CASE("Vector metrics compute correct distances", "[metrics][vector]")
{
  SECTION("L1 Metric")
  {
    float a[3] = {1.0f, -2.0f, 3.0f};
    float b[3] = {-4.0f, 6.0f, -8.0f};

    L1Metric<float> metric;
    auto dist = metric.distance(a, b, 3);
    auto sq = metric.squared_distance(a, b, 3);

    float expected = std::abs(1.0f - (-4.0f)) + std::abs(-2.0f - 6.0f)
        + std::abs(3.0f - (-8.0f));
    REQUIRE_THAT(dist, WithinRel(expected, 1e-5f));
    REQUIRE_THAT(sq, WithinRel(dist * dist, 1e-5f));
  }

  SECTION("L2 Metric")
  {
    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[3] = {4.0f, 6.0f, 8.0f};

    L2Metric<float> metric;
    auto sq = metric.squared_distance(a, b, 3);
    auto dist = metric.distance(a, b, 3);

    REQUIRE_THAT(dist, WithinRel(std::sqrt(sq), 1e-5f));

    float expected_sq = (1.0f - 4.0f) * (1.0f - 4.0f)
        + (2.0f - 6.0f) * (2.0f - 6.0f) + (3.0f - 8.0f) * (3.0f - 8.0f);
    REQUIRE_THAT(sq, WithinRel(expected_sq, 1e-5f));
  }

  SECTION("Linf Metric")
  {
    float a[4] = {1.0f, -2.0f, 3.0f, -4.0f};
    float b[4] = {5.0f, 6.0f, -1.0f, 2.0f};

    LinfMetric<float> metric;
    auto dist = metric.distance(a, b, 4);

    // L∞ is the maximum absolute difference
    float expected = std::max({
        std::abs(1.0f - 5.0f),    // 4
        std::abs(-2.0f - 6.0f),   // 8
        std::abs(3.0f - (-1.0f)), // 4
        std::abs(-4.0f - 2.0f)    // 6
    });
    REQUIRE_THAT(dist, WithinRel(expected, 1e-5f));
  }

  SECTION("Lp Metric with p=3")
  {
    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[3] = {4.0f, 5.0f, 6.0f};

    LpMetric<float, 3> metric;
    auto dist = metric.distance(a, b, 3);

    float expected = std::pow(
        std::pow(std::abs(1.0f - 4.0f), 3) +
        std::pow(std::abs(2.0f - 5.0f), 3) +
        std::pow(std::abs(3.0f - 6.0f), 3),
        1.0f / 3.0f
    );
    REQUIRE_THAT(dist, WithinRel(expected, 1e-5f));
  }

  SECTION("Generalized Lp Metric")
  {
    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[3] = {4.0f, 5.0f, 6.0f};

    // Test with p=2.5
    GeneralizedLpMetric<float> metric(2.5f);
    auto dist = metric.distance(a, b, 3);

    float expected = std::pow(
        std::pow(std::abs(1.0f - 4.0f), 2.5f) +
        std::pow(std::abs(2.0f - 5.0f), 2.5f) +
        std::pow(std::abs(3.0f - 6.0f), 2.5f),
        1.0f / 2.5f
    );
    REQUIRE_THAT(dist, WithinRel(expected, 1e-5f));
  }
}

TEST_CASE("Container interface works correctly", "[metrics][interface]")
{
  std::vector<double> a = {1.0, 2.0, 3.0};
  std::vector<double> b = {4.0, 5.0, 6.0};

  L2Metric<double> metric;
  auto dist1 = metric.distance(a.data(), b.data(), a.size());
  auto dist2 = metric.distance(a, b);

  REQUIRE_THAT(dist1, WithinRel(dist2, 1e-10));
}

TEST_CASE("Histogram metrics compute correct distances", "[metrics][histogram]")
{
  SECTION("Chi-Squared Metric")
  {
    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[3] = {2.0f, 3.0f, 1.0f};

    ChiSquaredMetric<float> metric;
    auto dist = metric.distance(a, b, 3);

    float expected = 0.5f * (
        (1.0f - 2.0f) * (1.0f - 2.0f) / (1.0f + 2.0f) +
        (2.0f - 3.0f) * (2.0f - 3.0f) / (2.0f + 3.0f) +
        (3.0f - 1.0f) * (3.0f - 1.0f) / (3.0f + 1.0f)
    );
    REQUIRE_THAT(dist, WithinRel(expected, 1e-5f));
  }

  SECTION("Histogram Intersection Metric")
  {
    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[3] = {2.0f, 1.0f, 4.0f};

    HistogramIntersectionMetric<float> metric;
    auto dist = metric.distance(a, b, 3);

    float intersection = std::min(1.0f, 2.0f) + std::min(2.0f, 1.0f) + std::min(3.0f, 4.0f);
    float sum_a = 1.0f + 2.0f + 3.0f;
    float sum_b = 2.0f + 1.0f + 4.0f;
    float expected = 1.0f - (intersection / std::max(sum_a, sum_b));
    REQUIRE_THAT(dist, WithinRel(expected, 1e-5f));
  }

  SECTION("Bhattacharyya Metric")
  {
    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[3] = {3.0f, 2.0f, 1.0f};

    BhattacharyyaMetric<float> metric;
    auto dist = metric.distance(a, b, 3);

    // Bhattacharyya coefficient
    float sum_a = 6.0f;
    float sum_b = 6.0f;
    float bc = std::sqrt(1.0f * 3.0f / (sum_a * sum_b)) +
               std::sqrt(2.0f * 2.0f / (sum_a * sum_b)) +
               std::sqrt(3.0f * 1.0f / (sum_a * sum_b));
    float expected = -std::log(bc);
    REQUIRE_THAT(dist, WithinRel(expected, 1e-5f));
  }

  SECTION("Hellinger Metric")
  {
    float a[3] = {4.0f, 1.0f, 0.0f};
    float b[3] = {1.0f, 4.0f, 0.0f};

    HellingerMetric<float> metric;
    auto dist = metric.distance(a, b, 3);

    float sum_a = 5.0f;
    float sum_b = 5.0f;
    float h2 = (std::sqrt(4.0f/5.0f) - std::sqrt(1.0f/5.0f)) * (std::sqrt(4.0f/5.0f) - std::sqrt(1.0f/5.0f)) +
               (std::sqrt(1.0f/5.0f) - std::sqrt(4.0f/5.0f)) * (std::sqrt(1.0f/5.0f) - std::sqrt(4.0f/5.0f)) +
               (std::sqrt(0.0f/5.0f) - std::sqrt(0.0f/5.0f)) * (std::sqrt(0.0f/5.0f) - std::sqrt(0.0f/5.0f));
    float expected = std::sqrt(h2 / 2.0f);
    REQUIRE_THAT(dist, WithinRel(expected, 1e-5f));
  }
}

TEST_CASE("Angular metrics compute correct distances", "[metrics][angular]")
{
  SECTION("Cosine Metric")
  {
    float a[3] = {1.0f, 0.0f, 0.0f};
    float b[3] = {0.0f, 1.0f, 0.0f};

    CosineMetric<float> metric;
    auto dist = metric.distance(a, b, 3);

    // Orthogonal vectors have cosine similarity 0, distance 1
    REQUIRE_THAT(dist, WithinRel(1.0f, 1e-5f));

    // Same vector
    auto dist_same = metric.distance(a, a, 3);
    REQUIRE_THAT(dist_same, WithinAbs(0.0f, 1e-5f));
  }

  SECTION("Angular Metric")
  {
    float a[3] = {1.0f, 0.0f, 0.0f};
    float b[3] = {0.0f, 1.0f, 0.0f};

    AngularMetric<float> metric;
    auto dist = metric.distance(a, b, 3);

    // Orthogonal vectors have angle π/2
    REQUIRE_THAT(dist, WithinRel(static_cast<float>(M_PI / 2.0f), 1e-5f));

    // Same vector has angle 0
    auto dist_same = metric.distance(a, a, 3);
    REQUIRE_THAT(dist_same, WithinAbs(0.0f, 1e-5f));
  }

  SECTION("Correlation Metric")
  {
    float a[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float b[4] = {2.0f, 4.0f, 6.0f, 8.0f};  // Perfect positive correlation

    CorrelationMetric<float> metric;
    auto dist = metric.distance(a, b, 4);

    // Perfect positive correlation should have distance 0
    REQUIRE_THAT(dist, WithinAbs(0.0f, 1e-5f));

    // Anti-correlated vectors
    float c[4] = {4.0f, 3.0f, 2.0f, 1.0f};
    auto dist_anti = metric.distance(a, c, 4);
    REQUIRE_THAT(dist_anti, WithinRel(1.0f, 1e-5f));
  }
}

TEST_CASE("Custom metrics work correctly", "[metrics][custom]")
{
  SECTION("Function-based custom metric")
  {
    auto manhattan_func = [](const float* a, const float* b, std::size_t size) {
      float sum = 0;
      for (std::size_t i = 0; i < size; ++i) {
        sum += std::abs(a[i] - b[i]);
      }
      return sum;
    };

    CustomMetric<float> metric(manhattan_func);
    
    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[3] = {4.0f, 5.0f, 6.0f};
    
    auto dist = metric.distance(a, b, 3);
    REQUIRE_THAT(dist, WithinRel(9.0f, 1e-5f));  // |1-4| + |2-5| + |3-6| = 9
  }

  SECTION("Lambda metric")
  {
    auto squared_euclidean = make_lambda_metric<float>(
      [](const float* a, const float* b, std::size_t size) {
        float sum = 0;
        for (std::size_t i = 0; i < size; ++i) {
          float diff = a[i] - b[i];
          sum += diff * diff;
        }
        return sum;
      }
    );

    float a[3] = {1.0f, 0.0f, 0.0f};
    float b[3] = {0.0f, 1.0f, 0.0f};
    
    auto dist = squared_euclidean.distance(a, b, 3);
    REQUIRE_THAT(dist, WithinRel(2.0f, 1e-5f));
  }

  SECTION("Weighted metric")
  {
    L2Metric<float> base_metric;
    std::vector<float> weights = {1.0f, 2.0f, 3.0f};
    
    auto weighted = make_weighted_metric(base_metric, weights);
    
    float a[3] = {1.0f, 1.0f, 1.0f};
    float b[3] = {2.0f, 2.0f, 2.0f};
    
    auto dist = weighted.distance(a, b, 3);
    // Each component contributes (a[i]-b[i])^2 * weight[i]
    // sqrt(1*1 + 2*1 + 3*1) = sqrt(6)
    REQUIRE_THAT(dist, WithinRel(std::sqrt(6.0f), 1e-5f));
  }
}

TEST_CASE("Metric factory creates metrics correctly", "[metrics][factory]")
{
  SECTION("Create metrics by name")
  {
    auto l1 = create_metric<float>("l1");
    auto l2 = create_metric<float>("l2");
    auto cosine = create_metric<float>("cosine");

    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[3] = {4.0f, 5.0f, 6.0f};

    REQUIRE(l1 != nullptr);
    REQUIRE(l2 != nullptr);
    REQUIRE(cosine != nullptr);

    // Test that they compute reasonable values
    auto dist_l1 = l1->distance(a, b, 3);
    auto dist_l2 = l2->distance(a, b, 3);
    
    REQUIRE(dist_l1 > 0);
    REQUIRE(dist_l2 > 0);
    REQUIRE(dist_l1 != dist_l2);  // Different metrics should give different results
  }

  SECTION("Available metrics")
  {
    auto& factory = MetricFactory<float>::instance();
    auto metrics = factory.available_metrics();
    
    // Check that common metrics are registered
    REQUIRE(std::find(metrics.begin(), metrics.end(), "l1") != metrics.end());
    REQUIRE(std::find(metrics.begin(), metrics.end(), "l2") != metrics.end());
    REQUIRE(std::find(metrics.begin(), metrics.end(), "euclidean") != metrics.end());
    REQUIRE(std::find(metrics.begin(), metrics.end(), "manhattan") != metrics.end());
    REQUIRE(std::find(metrics.begin(), metrics.end(), "cosine") != metrics.end());
    REQUIRE(std::find(metrics.begin(), metrics.end(), "chi_squared") != metrics.end());
  }

  SECTION("Unknown metric throws")
  {
    REQUIRE_THROWS_AS(create_metric<float>("unknown_metric"), std::invalid_argument);
  }
}

TEST_CASE("Metric properties are correct", "[metrics][traits]")
{
  SECTION("Symmetric metrics")
  {
    L2Metric<float> l2;
    CosineMetric<float> cosine;
    
    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[3] = {4.0f, 5.0f, 6.0f};
    
    // Symmetric metrics should satisfy d(a,b) = d(b,a)
    REQUIRE_THAT(l2.distance(a, b, 3), WithinRel(l2.distance(b, a, 3), 1e-5f));
    REQUIRE_THAT(cosine.distance(a, b, 3), WithinRel(cosine.distance(b, a, 3), 1e-5f));
  }

  SECTION("Triangle inequality")
  {
    L2Metric<float> metric;
    
    float a[3] = {0.0f, 0.0f, 0.0f};
    float b[3] = {1.0f, 0.0f, 0.0f};
    float c[3] = {1.0f, 1.0f, 0.0f};
    
    auto d_ab = metric.distance(a, b, 3);
    auto d_bc = metric.distance(b, c, 3);
    auto d_ac = metric.distance(a, c, 3);
    
    // Triangle inequality: d(a,c) <= d(a,b) + d(b,c)
    REQUIRE(d_ac <= d_ab + d_bc + 1e-5f);
  }

  SECTION("Non-negativity")
  {
    std::vector<float> vec1 = generate_random_vector<float>(10);
    std::vector<float> vec2 = generate_random_vector<float>(10);
    
    L1Metric<float> l1;
    L2Metric<float> l2;
    LinfMetric<float> linf;
    
    // All distances should be non-negative
    REQUIRE(l1.distance(vec1.data(), vec2.data(), vec1.size()) >= 0);
    REQUIRE(l2.distance(vec1.data(), vec2.data(), vec1.size()) >= 0);
    REQUIRE(linf.distance(vec1.data(), vec2.data(), vec1.size()) >= 0);
  }
}

TEST_CASE("Edge cases are handled correctly", "[metrics][edge]")
{
  SECTION("Zero-length vectors")
  {
    L2Metric<float> metric;
    float* null_ptr = nullptr;
    
    auto dist = metric.distance(null_ptr, null_ptr, 0);
    REQUIRE_THAT(dist, WithinAbs(0.0f, 1e-5f));
  }

  SECTION("Identical vectors")
  {
    float a[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    
    L1Metric<float> l1;
    L2Metric<float> l2;
    CosineMetric<float> cosine;
    
    REQUIRE_THAT(l1.distance(a, a, 5), WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(l2.distance(a, a, 5), WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(cosine.distance(a, a, 5), WithinAbs(0.0f, 1e-5f));
  }

  SECTION("Zero vectors in angular metrics")
  {
    float zero[3] = {0.0f, 0.0f, 0.0f};
    float nonzero[3] = {1.0f, 2.0f, 3.0f};
    
    CosineMetric<float> cosine;
    AngularMetric<float> angular;
    
    // Should return maximum distance for zero vectors
    auto dist_cosine = cosine.distance(zero, nonzero, 3);
    auto dist_angular = angular.distance(zero, nonzero, 3);
    
    REQUIRE(dist_cosine > 0);
    REQUIRE(dist_angular > 0);
  }
}

TEST_CASE("Point cloud metrics compute correct distances", "[metrics][pointcloud]")
{
  using point_t = toolbox::types::point_t<float>;
  using point_cloud_t = toolbox::types::point_cloud_t<float>;

  // Helper function to create a simple point cloud
  auto create_cloud = [](const std::vector<std::array<float, 3>>& points) {
    point_cloud_t cloud;
    for (const auto& p : points) {
      cloud += point_t(p[0], p[1], p[2]);
    }
    return cloud;
  };

  SECTION("Hausdorff distance")
  {
    // Create two simple clouds
    auto cloud1 = create_cloud({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}});
    auto cloud2 = create_cloud({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}});
    
    HausdorffMetric<float> metric;
    auto dist = metric.distance(cloud1, cloud2);
    
    // The Hausdorff distance should be the distance from (1,1,0) to nearest point in cloud1
    // which is either (1,0,0) or (0,1,0), both at distance 1
    REQUIRE_THAT(dist, WithinRel(1.0f, 1e-5f));
    
    // Same clouds should have distance 0
    auto dist_same = metric.distance(cloud1, cloud1);
    REQUIRE_THAT(dist_same, WithinAbs(0.0f, 1e-5f));
  }

  SECTION("Modified Hausdorff distance")
  {
    auto cloud1 = create_cloud({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}});
    auto cloud2 = create_cloud({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {10, 10, 10}}); // Outlier
    
    ModifiedHausdorffMetric<float> metric(2); // Average of 2 smallest
    auto dist = metric.distance(cloud1, cloud2);
    
    // Should be more robust to the outlier than standard Hausdorff
    HausdorffMetric<float> hausdorff;
    auto hausdorff_dist = hausdorff.distance(cloud1, cloud2);
    
    REQUIRE(dist < hausdorff_dist); // Modified should be less affected by outlier
  }

  SECTION("Chamfer distance")
  {
    auto cloud1 = create_cloud({{0, 0, 0}, {1, 0, 0}});
    auto cloud2 = create_cloud({{0.5, 0, 0}, {1.5, 0, 0}});
    
    ChamferMetric<float> metric;
    auto dist = metric.distance(cloud1, cloud2);
    
    // Average nearest neighbor distance
    // cloud1 to cloud2: (0,0,0) -> (0.5,0,0) = 0.5, (1,0,0) -> (0.5,0,0) = 0.5
    // cloud2 to cloud1: (0.5,0,0) -> (0,0,0) = 0.5, (1.5,0,0) -> (1,0,0) = 0.5
    // Average = 0.5
    REQUIRE_THAT(dist, WithinRel(0.5f, 1e-5f));
  }

  SECTION("Centroid distance")
  {
    auto cloud1 = create_cloud({{0, 0, 0}, {2, 0, 0}, {0, 2, 0}, {2, 2, 0}});
    auto cloud2 = create_cloud({{1, 1, 0}, {3, 1, 0}, {1, 3, 0}, {3, 3, 0}});
    
    CentroidMetric<float> metric;
    auto dist = metric.distance(cloud1, cloud2);
    
    // Centroid of cloud1: (1, 1, 0)
    // Centroid of cloud2: (2, 2, 0)
    // Distance = sqrt(2)
    REQUIRE_THAT(dist, WithinRel(std::sqrt(2.0f), 1e-5f));
  }

  SECTION("Bounding box metrics")
  {
    auto cloud1 = create_cloud({{0, 0, 0}, {1, 1, 1}});
    auto cloud2 = create_cloud({{2, 0, 0}, {3, 1, 1}});
    
    SECTION("Center distance")
    {
      BoundingBoxMetric<float> metric(BoundingBoxMetric<float>::Mode::CENTER_DISTANCE);
      auto dist = metric.distance(cloud1, cloud2);
      
      // Center1: (0.5, 0.5, 0.5), Center2: (2.5, 0.5, 0.5)
      // Distance = 2
      REQUIRE_THAT(dist, WithinRel(2.0f, 1e-5f));
    }
    
    SECTION("Minimum distance")
    {
      BoundingBoxMetric<float> metric(BoundingBoxMetric<float>::Mode::MIN_DISTANCE);
      auto dist = metric.distance(cloud1, cloud2);
      
      // Boxes don't overlap, minimum distance is 1 along x-axis
      REQUIRE_THAT(dist, WithinRel(1.0f, 1e-5f));
      
      // Overlapping boxes
      auto cloud3 = create_cloud({{0.5, 0.5, 0.5}, {1.5, 1.5, 1.5}});
      auto dist_overlap = metric.distance(cloud1, cloud3);
      REQUIRE_THAT(dist_overlap, WithinAbs(0.0f, 1e-5f));
    }
    
    SECTION("IoU distance")
    {
      BoundingBoxMetric<float> metric(BoundingBoxMetric<float>::Mode::IOU_DISTANCE);
      
      // No overlap
      auto dist = metric.distance(cloud1, cloud2);
      REQUIRE_THAT(dist, WithinRel(1.0f, 1e-5f)); // 1 - 0 = 1
      
      // Full overlap (same cloud)
      auto dist_same = metric.distance(cloud1, cloud1);
      REQUIRE_THAT(dist_same, WithinAbs(0.0f, 1e-5f)); // 1 - 1 = 0
      
      // Partial overlap
      auto cloud3 = create_cloud({{0.5, 0.5, 0.5}, {1.5, 1.5, 1.5}});
      auto dist_partial = metric.distance(cloud1, cloud3);
      // IoU = 0.5^3 / (1 + 1 - 0.5^3) = 0.125 / 1.875 ≈ 0.0667
      // Distance = 1 - 0.0667 ≈ 0.933
      REQUIRE(dist_partial > 0.9f);
      REQUIRE(dist_partial < 1.0f);
    }
  }

  SECTION("Point cloud EMD metric")
  {
    auto cloud1 = create_cloud({{0, 0, 0}, {1, 0, 0}});
    auto cloud2 = create_cloud({{0.5, 0, 0}, {1.5, 0, 0}});
    
    PointCloudEMDMetric<float> metric;
    auto dist = metric.distance(cloud1, cloud2);
    
    // For equal-sized clouds, should find optimal matching
    // In this case, (0,0,0) -> (0.5,0,0) and (1,0,0) -> (1.5,0,0)
    // Average distance = 0.5
    REQUIRE_THAT(dist, WithinRel(0.5f, 1e-5f));
  }

  SECTION("Empty cloud handling")
  {
    point_cloud_t empty_cloud;
    auto cloud = create_cloud({{0, 0, 0}, {1, 1, 1}});
    
    HausdorffMetric<float> hausdorff;
    ChamferMetric<float> chamfer;
    CentroidMetric<float> centroid;
    
    REQUIRE(std::isinf(hausdorff.distance(empty_cloud, cloud)));
    REQUIRE(std::isinf(chamfer.distance(empty_cloud, cloud)));
    REQUIRE(std::isinf(centroid.distance(empty_cloud, cloud)));
  }

  SECTION("Large cloud performance characteristics")
  {
    // Create larger clouds for more realistic testing
    point_cloud_t cloud1, cloud2;
    
    // Create two spherical clouds with different centers
    const int num_points = 100;
    for (int i = 0; i < num_points; ++i) {
      float theta = 2 * M_PI * i / num_points;
      float phi = M_PI * i / num_points;
      
      // Cloud 1 centered at origin
      cloud1 += point_t(std::sin(phi) * std::cos(theta),
                       std::sin(phi) * std::sin(theta),
                       std::cos(phi));
      
      // Cloud 2 centered at (2, 0, 0)
      cloud2 += point_t(2 + std::sin(phi) * std::cos(theta),
                       std::sin(phi) * std::sin(theta),
                       std::cos(phi));
    }
    
    CentroidMetric<float> centroid_metric;
    auto centroid_dist = centroid_metric.distance(cloud1, cloud2);
    
    // Centroids should be approximately 2 units apart
    REQUIRE_THAT(centroid_dist, WithinRel(2.0f, 0.1f));
    
    // Chamfer distance should be reasonable
    ChamferMetric<float> chamfer_metric;
    auto chamfer_dist = chamfer_metric.distance(cloud1, cloud2);
    REQUIRE(chamfer_dist > 1.5f); // Should be at least 1.5
    REQUIRE(chamfer_dist < 2.5f); // But not more than 2.5
  }
}

TEST_CASE("LCP metric for point cloud registration", "[metrics][pointcloud][lcp]")
{
  using point_t = toolbox::types::point_t<float>;
  using point_cloud_t = toolbox::types::point_cloud_t<float>;
  using transformation_t = Eigen::Matrix4f;

  // Helper function to create a simple point cloud
  auto create_cloud = [](const std::vector<std::array<float, 3>>& points) {
    point_cloud_t cloud;
    for (const auto& p : points) {
      cloud += point_t(p[0], p[1], p[2]);
    }
    return cloud;
  };

  // Helper function to apply transformation to cloud
  auto transform_cloud = [](const point_cloud_t& cloud, const transformation_t& transform) {
    point_cloud_t transformed;
    for (const auto& pt : cloud.points) {
      Eigen::Vector4f p(pt.x, pt.y, pt.z, 1.0f);
      Eigen::Vector4f tp = transform * p;
      transformed += point_t(tp[0], tp[1], tp[2]);
    }
    return transformed;
  };

  SECTION("Basic LCP score computation")
  {
    // Create source cloud
    auto source = create_cloud({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}});
    auto target = create_cloud({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}});
    
    // Identity transformation
    transformation_t identity = transformation_t::Identity();
    
    LCPMetric<float> metric(0.1f); // 0.1 threshold
    auto score = metric.compute_lcp_score(source, target, identity);
    
    // Perfect alignment should have score 0
    REQUIRE_THAT(score, WithinAbs(0.0f, 1e-5f));
    
    // Check inliers
    std::vector<std::size_t> inliers;
    metric.compute_lcp_score(source, target, identity, &inliers);
    REQUIRE(inliers.size() == source.size()); // All points are inliers
  }

  SECTION("LCP score with translation")
  {
    auto source = create_cloud({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}});
    auto target = create_cloud({{1, 0, 0}, {2, 0, 0}, {1, 1, 0}});
    
    // Translation by (1, 0, 0)
    transformation_t transform = transformation_t::Identity();
    transform(0, 3) = 1.0f;
    
    LCPMetric<float> metric(0.1f);
    auto score = metric.compute_lcp_score(source, target, transform);
    
    // Should have perfect alignment after transformation
    REQUIRE_THAT(score, WithinAbs(0.0f, 1e-5f));
  }

  SECTION("LCP score with rotation")
  {
    auto source = create_cloud({{1, 0, 0}, {0, 1, 0}, {-1, 0, 0}, {0, -1, 0}});
    
    // 90-degree rotation around Z axis
    transformation_t transform = transformation_t::Identity();
    float angle = M_PI / 2;
    transform(0, 0) = std::cos(angle);
    transform(0, 1) = -std::sin(angle);
    transform(1, 0) = std::sin(angle);
    transform(1, 1) = std::cos(angle);
    
    auto target = transform_cloud(source, transform);
    
    LCPMetric<float> metric(0.1f);
    auto score = metric.compute_lcp_score(source, target, transform);
    
    // Should have perfect alignment
    REQUIRE_THAT(score, WithinAbs(0.0f, 1e-5f));
  }

  SECTION("LCP score with partial overlap")
  {
    auto source = create_cloud({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}});
    auto target = create_cloud({{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}});
    
    transformation_t identity = transformation_t::Identity();
    
    LCPMetric<float> metric(0.5f); // Larger threshold
    std::vector<std::size_t> inliers;
    auto score = metric.compute_lcp_score(source, target, identity, &inliers);
    
    // Only 2 points should be inliers
    REQUIRE(inliers.size() == 2);
    REQUIRE(score == 0.0f); // Perfect match for inliers
  }

  SECTION("LCP score with noise")
  {
    auto source = create_cloud({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}});
    auto target = create_cloud({{0.05, 0.05, 0}, {1.05, -0.05, 0}, {-0.05, 1.05, 0}});
    
    transformation_t identity = transformation_t::Identity();
    
    // Small threshold - no inliers
    LCPMetric<float> metric_small(0.01f);
    auto score_small = metric_small.compute_lcp_score(source, target, identity);
    REQUIRE(score_small == std::numeric_limits<float>::max()); // No inliers
    
    // Larger threshold - all inliers
    LCPMetric<float> metric_large(0.2f);
    std::vector<std::size_t> inliers;
    auto score_large = metric_large.compute_lcp_score(source, target, identity, &inliers);
    REQUIRE(inliers.size() == 3);
    REQUIRE(score_large > 0); // Should have some average distance
    REQUIRE(score_large < 0.1f); // But still small
  }

  SECTION("LCP score threshold behavior")
  {
    auto source = create_cloud({{0, 0, 0}, {1, 0, 0}});
    auto target = create_cloud({{0.1, 0, 0}, {1.2, 0, 0}});
    
    transformation_t identity = transformation_t::Identity();
    
    // Test different thresholds
    LCPMetric<float> metric1(0.05f);
    LCPMetric<float> metric2(0.15f);
    LCPMetric<float> metric3(0.25f);
    
    std::vector<std::size_t> inliers1, inliers2, inliers3;
    metric1.compute_lcp_score(source, target, identity, &inliers1);
    metric2.compute_lcp_score(source, target, identity, &inliers2);
    metric3.compute_lcp_score(source, target, identity, &inliers3);
    
    // More relaxed threshold should have more inliers
    REQUIRE(inliers1.size() <= inliers2.size());
    REQUIRE(inliers2.size() <= inliers3.size());
  }

  SECTION("LCP metric getter/setter")
  {
    LCPMetric<float> metric(1.0f);
    REQUIRE_THAT(metric.get_inlier_threshold(), WithinRel(1.0f, 1e-5f));
    
    metric.set_inlier_threshold(2.5f);
    REQUIRE_THAT(metric.get_inlier_threshold(), WithinRel(2.5f, 1e-5f));
  }

  SECTION("Empty cloud handling")
  {
    point_cloud_t empty_cloud;
    auto cloud = create_cloud({{0, 0, 0}, {1, 1, 1}});
    transformation_t identity = transformation_t::Identity();
    
    LCPMetric<float> metric(1.0f);
    
    // Empty source
    auto score1 = metric.compute_lcp_score(empty_cloud, cloud, identity);
    REQUIRE(score1 == std::numeric_limits<float>::max());
    
    // Empty target
    auto score2 = metric.compute_lcp_score(cloud, empty_cloud, identity);
    REQUIRE(score2 == std::numeric_limits<float>::max());
  }

  SECTION("Complex transformation")
  {
    // Create a more complex test case
    auto source = create_cloud({
      {0, 0, 0}, {1, 0, 0}, {2, 0, 0},
      {0, 1, 0}, {1, 1, 0}, {2, 1, 0},
      {0, 2, 0}, {1, 2, 0}, {2, 2, 0}
    });
    
    // Combined rotation and translation
    transformation_t transform = transformation_t::Identity();
    float angle = M_PI / 6; // 30 degrees
    transform(0, 0) = std::cos(angle);
    transform(0, 1) = -std::sin(angle);
    transform(1, 0) = std::sin(angle);
    transform(1, 1) = std::cos(angle);
    transform(0, 3) = 0.5f;
    transform(1, 3) = 0.5f;
    
    auto target = transform_cloud(source, transform);
    
    LCPMetric<float> metric(0.01f);
    std::vector<std::size_t> inliers;
    auto score = metric.compute_lcp_score(source, target, transform, &inliers);
    
    // Should have perfect alignment
    REQUIRE_THAT(score, WithinAbs(0.0f, 1e-4f));
    REQUIRE(inliers.size() == source.size());
  }

  SECTION("Performance with large clouds")
  {
    // Create larger clouds for performance testing
    point_cloud_t source, target;
    
    const int grid_size = 10;
    for (int i = 0; i < grid_size; ++i) {
      for (int j = 0; j < grid_size; ++j) {
        source += point_t(i * 0.1f, j * 0.1f, 0);
        target += point_t(i * 0.1f + 0.05f, j * 0.1f + 0.05f, 0);
      }
    }
    
    transformation_t identity = transformation_t::Identity();
    
    LCPMetric<float> metric(0.1f);
    std::vector<std::size_t> inliers;
    auto score = metric.compute_lcp_score(source, target, identity, &inliers);
    
    // All points should be inliers (distance ~0.07)
    REQUIRE(inliers.size() == source.size());
    REQUIRE_THAT(score, WithinRel(std::sqrt(0.05f * 0.05f + 0.05f * 0.05f), 0.01f));
  }
}