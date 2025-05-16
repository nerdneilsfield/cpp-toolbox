#include <numeric>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <cpp-toolbox/io/dataloader/dataloader.hpp>
#include <cpp-toolbox/io/dataset/dataset.hpp>

using namespace toolbox::io;

// Simple integer dataset
class int_dataset_t : public dataset_t<int_dataset_t, int>
{
public:
  explicit int_dataset_t(std::size_t n)
      : m_size(n)
  {
  }

  [[nodiscard]] std::size_t size_impl() const { return m_size; }

  [[nodiscard]] std::optional<int> at_impl(std::size_t index) const
  {
    if (index < m_size) {
      return static_cast<int>(index);
    }
    return std::nullopt;
  }

private:
  std::size_t m_size;
};

TEST_CASE("DataLoader sequential sampler", "[io][dataloader]")
{
  int_dataset_t dataset(10);
  sampler_t<sequential_policy_t> sampler(dataset.size());
  dataloader_t loader(dataset, sampler, 3);

  std::vector<int> collected;
  for (const auto& batch : loader) {
    collected.insert(collected.end(), batch.begin(), batch.end());
  }

  REQUIRE(collected.size() == dataset.size());
  std::vector<int> expected(10);
  std::iota(expected.begin(), expected.end(), 0);
  REQUIRE(collected == expected);
}

TEST_CASE("DataLoader shuffle sampler", "[io][dataloader]")
{
  int_dataset_t dataset(10);
  shuffle_policy_t policy(123);
  sampler_t<shuffle_policy_t> sampler(dataset.size(), policy);
  dataloader_t loader(dataset, sampler, 4);

  std::vector<int> collected;
  for (const auto& batch : loader) {
    collected.insert(collected.end(), batch.begin(), batch.end());
  }

  std::vector<int> sorted = collected;
  std::sort(sorted.begin(), sorted.end());
  std::vector<int> expected(10);
  std::iota(expected.begin(), expected.end(), 0);
  REQUIRE(sorted == expected);
}