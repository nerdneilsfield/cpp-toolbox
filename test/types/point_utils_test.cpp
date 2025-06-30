#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cpp-toolbox/types/point_utils.hpp>
#include <cpp-toolbox/types/point.hpp>

#include <Eigen/Core>
#include <cmath>

using namespace toolbox::types;
using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinAbs;

// Helper function to create a simple test cloud
template<typename T>
point_cloud_t<T> create_test_cloud(std::size_t size = 100)
{
  point_cloud_t<T> cloud;
  cloud.points.reserve(size);
  
  for (std::size_t i = 0; i < size; ++i) {
    cloud.points.emplace_back(
        static_cast<T>(i),
        static_cast<T>(i * 2),
        static_cast<T>(i * 3));
  }
  
  return cloud;
}

TEST_CASE("Point cloud transformations", "[types][point_utils][transform]")
{
  SECTION("Identity transformation")
  {
    auto cloud = create_test_cloud<float>(10);
    Eigen::Matrix4f identity = Eigen::Matrix4f::Identity();
    
    auto transformed = transform_point_cloud(cloud, identity);
    
    REQUIRE(transformed.size() == cloud.size());
    for (std::size_t i = 0; i < cloud.size(); ++i) {
      REQUIRE_THAT(transformed.points[i].x, WithinAbs(cloud.points[i].x, 1e-6f));
      REQUIRE_THAT(transformed.points[i].y, WithinAbs(cloud.points[i].y, 1e-6f));
      REQUIRE_THAT(transformed.points[i].z, WithinAbs(cloud.points[i].z, 1e-6f));
    }
  }
  
  SECTION("Translation transformation")
  {
    auto cloud = create_test_cloud<float>(5);
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    transform(0, 3) = 10.0f;  // Translate X by 10
    transform(1, 3) = 20.0f;  // Translate Y by 20
    transform(2, 3) = 30.0f;  // Translate Z by 30
    
    auto transformed = transform_point_cloud(cloud, transform);
    
    REQUIRE(transformed.size() == cloud.size());
    for (std::size_t i = 0; i < cloud.size(); ++i) {
      REQUIRE_THAT(transformed.points[i].x, WithinAbs(cloud.points[i].x + 10.0f, 1e-6f));
      REQUIRE_THAT(transformed.points[i].y, WithinAbs(cloud.points[i].y + 20.0f, 1e-6f));
      REQUIRE_THAT(transformed.points[i].z, WithinAbs(cloud.points[i].z + 30.0f, 1e-6f));
    }
  }
  
  SECTION("Rotation transformation (90 degrees around Z)")
  {
    point_cloud_t<float> cloud;
    cloud.points.emplace_back(1.0f, 0.0f, 0.0f);
    cloud.points.emplace_back(0.0f, 1.0f, 0.0f);
    
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    float angle = M_PI / 2;  // 90 degrees
    transform(0, 0) = std::cos(angle);
    transform(0, 1) = -std::sin(angle);
    transform(1, 0) = std::sin(angle);
    transform(1, 1) = std::cos(angle);
    
    auto transformed = transform_point_cloud(cloud, transform);
    
    REQUIRE(transformed.size() == 2);
    // (1,0,0) -> (0,1,0)
    REQUIRE_THAT(transformed.points[0].x, WithinAbs(0.0f, 1e-6f));
    REQUIRE_THAT(transformed.points[0].y, WithinAbs(1.0f, 1e-6f));
    REQUIRE_THAT(transformed.points[0].z, WithinAbs(0.0f, 1e-6f));
    // (0,1,0) -> (-1,0,0)
    REQUIRE_THAT(transformed.points[1].x, WithinAbs(-1.0f, 1e-6f));
    REQUIRE_THAT(transformed.points[1].y, WithinAbs(0.0f, 1e-6f));
    REQUIRE_THAT(transformed.points[1].z, WithinAbs(0.0f, 1e-6f));
  }
  
  SECTION("Combined rotation and translation")
  {
    point_cloud_t<float> cloud;
    cloud.points.emplace_back(1.0f, 0.0f, 0.0f);
    
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    // 180 degree rotation around Z
    transform(0, 0) = -1.0f;
    transform(1, 1) = -1.0f;
    // Translation
    transform(0, 3) = 5.0f;
    transform(1, 3) = 3.0f;
    
    auto transformed = transform_point_cloud(cloud, transform);
    
    REQUIRE(transformed.size() == 1);
    // (1,0,0) rotated 180° -> (-1,0,0), then translated by (5,3,0) -> (4,3,0)
    REQUIRE_THAT(transformed.points[0].x, WithinAbs(4.0f, 1e-6f));
    REQUIRE_THAT(transformed.points[0].y, WithinAbs(3.0f, 1e-6f));
    REQUIRE_THAT(transformed.points[0].z, WithinAbs(0.0f, 1e-6f));
  }
  
  SECTION("Empty cloud transformation")
  {
    point_cloud_t<float> empty_cloud;
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    
    auto transformed = transform_point_cloud(empty_cloud, transform);
    REQUIRE(transformed.empty());
  }
}

TEST_CASE("Parallel point cloud transformations", "[types][point_utils][transform][parallel]")
{
  SECTION("Parallel transformation matches sequential")
  {
    auto cloud = create_test_cloud<float>(1000);
    
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    float angle = M_PI / 4;  // 45 degrees
    transform(0, 0) = std::cos(angle);
    transform(0, 1) = -std::sin(angle);
    transform(1, 0) = std::sin(angle);
    transform(1, 1) = std::cos(angle);
    transform(0, 3) = 10.0f;
    transform(1, 3) = 20.0f;
    transform(2, 3) = 30.0f;
    
    auto transformed_seq = transform_point_cloud(cloud, transform);
    auto transformed_par = transform_point_cloud_parallel(cloud, transform);
    
    REQUIRE(transformed_seq.size() == transformed_par.size());
    for (std::size_t i = 0; i < transformed_seq.size(); ++i) {
      REQUIRE_THAT(transformed_par.points[i].x, WithinAbs(transformed_seq.points[i].x, 1e-5f));
      REQUIRE_THAT(transformed_par.points[i].y, WithinAbs(transformed_seq.points[i].y, 1e-5f));
      REQUIRE_THAT(transformed_par.points[i].z, WithinAbs(transformed_seq.points[i].z, 1e-5f));
    }
  }
  
  SECTION("Parallel empty cloud")
  {
    point_cloud_t<float> empty_cloud;
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    
    auto transformed = transform_point_cloud_parallel(empty_cloud, transform);
    REQUIRE(transformed.empty());
  }
}

TEST_CASE("In-place point cloud transformations", "[types][point_utils][transform][inplace]")
{
  SECTION("In-place identity transformation")
  {
    auto cloud = create_test_cloud<float>(10);
    auto original = cloud;  // Make a copy
    Eigen::Matrix4f identity = Eigen::Matrix4f::Identity();
    
    transform_point_cloud_inplace(cloud, identity);
    
    REQUIRE(cloud.size() == original.size());
    for (std::size_t i = 0; i < cloud.size(); ++i) {
      REQUIRE_THAT(cloud.points[i].x, WithinAbs(original.points[i].x, 1e-6f));
      REQUIRE_THAT(cloud.points[i].y, WithinAbs(original.points[i].y, 1e-6f));
      REQUIRE_THAT(cloud.points[i].z, WithinAbs(original.points[i].z, 1e-6f));
    }
  }
  
  SECTION("In-place translation")
  {
    auto cloud = create_test_cloud<float>(5);
    auto original = cloud;  // Make a copy
    
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    transform(0, 3) = 10.0f;
    transform(1, 3) = 20.0f;
    transform(2, 3) = 30.0f;
    
    transform_point_cloud_inplace(cloud, transform);
    
    for (std::size_t i = 0; i < cloud.size(); ++i) {
      REQUIRE_THAT(cloud.points[i].x, WithinAbs(original.points[i].x + 10.0f, 1e-6f));
      REQUIRE_THAT(cloud.points[i].y, WithinAbs(original.points[i].y + 20.0f, 1e-6f));
      REQUIRE_THAT(cloud.points[i].z, WithinAbs(original.points[i].z + 30.0f, 1e-6f));
    }
  }
  
  SECTION("In-place parallel matches sequential")
  {
    auto cloud_seq = create_test_cloud<float>(1000);
    auto cloud_par = cloud_seq;  // Make a copy
    
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    float angle = M_PI / 3;  // 60 degrees
    transform(0, 0) = std::cos(angle);
    transform(0, 1) = -std::sin(angle);
    transform(1, 0) = std::sin(angle);
    transform(1, 1) = std::cos(angle);
    transform(0, 3) = 5.0f;
    transform(1, 3) = -10.0f;
    
    transform_point_cloud_inplace(cloud_seq, transform);
    transform_point_cloud_inplace_parallel(cloud_par, transform);
    
    REQUIRE(cloud_seq.size() == cloud_par.size());
    for (std::size_t i = 0; i < cloud_seq.size(); ++i) {
      REQUIRE_THAT(cloud_par.points[i].x, WithinAbs(cloud_seq.points[i].x, 1e-5f));
      REQUIRE_THAT(cloud_par.points[i].y, WithinAbs(cloud_seq.points[i].y, 1e-5f));
      REQUIRE_THAT(cloud_par.points[i].z, WithinAbs(cloud_seq.points[i].z, 1e-5f));
    }
  }
  
  SECTION("In-place empty cloud")
  {
    point_cloud_t<float> empty_cloud;
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    
    // Should not crash
    transform_point_cloud_inplace(empty_cloud, transform);
    REQUIRE(empty_cloud.empty());
    
    transform_point_cloud_inplace_parallel(empty_cloud, transform);
    REQUIRE(empty_cloud.empty());
  }
}

TEST_CASE("Transform with double precision", "[types][point_utils][transform][double]")
{
  SECTION("Double precision transformation")
  {
    point_cloud_t<double> cloud;
    cloud.points.emplace_back(1.0, 2.0, 3.0);
    cloud.points.emplace_back(4.0, 5.0, 6.0);
    
    Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();
    transform(0, 3) = 0.5;
    transform(1, 3) = 1.5;
    transform(2, 3) = 2.5;
    
    auto transformed = transform_point_cloud(cloud, transform);
    
    REQUIRE(transformed.size() == 2);
    REQUIRE_THAT(transformed.points[0].x, WithinAbs(1.5, 1e-10));
    REQUIRE_THAT(transformed.points[0].y, WithinAbs(3.5, 1e-10));
    REQUIRE_THAT(transformed.points[0].z, WithinAbs(5.5, 1e-10));
    REQUIRE_THAT(transformed.points[1].x, WithinAbs(4.5, 1e-10));
    REQUIRE_THAT(transformed.points[1].y, WithinAbs(6.5, 1e-10));
    REQUIRE_THAT(transformed.points[1].z, WithinAbs(8.5, 1e-10));
  }
}

TEST_CASE("Performance characteristics", "[types][point_utils][transform][performance]")
{
  SECTION("Large cloud transformation")
  {
    // Create a large cloud for performance testing
    auto cloud = create_test_cloud<float>(10000);
    
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    // Complex transformation
    float angle = M_PI / 6;
    transform(0, 0) = std::cos(angle);
    transform(0, 1) = -std::sin(angle);
    transform(1, 0) = std::sin(angle);
    transform(1, 1) = std::cos(angle);
    transform(0, 3) = 100.0f;
    transform(1, 3) = 200.0f;
    transform(2, 3) = 300.0f;
    
    // Just verify it completes without error
    auto transformed_seq = transform_point_cloud(cloud, transform);
    auto transformed_par = transform_point_cloud_parallel(cloud, transform);
    
    REQUIRE(transformed_seq.size() == cloud.size());
    REQUIRE(transformed_par.size() == cloud.size());
    
    // Verify a few points match between sequential and parallel
    for (std::size_t i = 0; i < 10; ++i) {
      std::size_t idx = i * 1000;  // Sample every 1000th point
      REQUIRE_THAT(transformed_par.points[idx].x, WithinRel(transformed_seq.points[idx].x, 1e-5f));
      REQUIRE_THAT(transformed_par.points[idx].y, WithinRel(transformed_seq.points[idx].y, 1e-5f));
      REQUIRE_THAT(transformed_par.points[idx].z, WithinRel(transformed_seq.points[idx].z, 1e-5f));
    }
  }
}