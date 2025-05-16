#pragma once

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <random>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::io
{

/**
 * @brief 顺序采样策略/Sequential sampling policy
 */
struct sequential_policy_t
{
  void operator()(std::vector<std::size_t>& /*indices*/) const noexcept {}
};

/**
 * @brief 随机打乱采样策略/Shuffle sampling policy
 */
class shuffle_policy_t
{
public:
  explicit shuffle_policy_t(unsigned seed = std::random_device {}())
      : m_rng(seed)
  {
  }

  void operator()(std::vector<std::size_t>& indices)
  {
    std::shuffle(indices.begin(), indices.end(), m_rng);
  }

  void set_seed(unsigned seed) { m_rng.seed(seed); }

private:
  std::mt19937 m_rng;
};

/**
 * @brief 通用采样器/Generic sampler
 *
 * @tparam PolicyT 采样策略类型/Sampling policy type
 */
template<typename PolicyT = sequential_policy_t>
class CPP_TOOLBOX_EXPORT sampler_t
{
public:
  using index_type = std::size_t;

  sampler_t(const sampler_t& other)
      : m_dataset_size(other.m_dataset_size)
      , m_policy(other.m_policy)
      , m_indices(other.m_indices)
  {
    auto offset = other.m_iter - other.m_indices.begin();
    m_iter = m_indices.begin() + offset;
  }

  sampler_t& operator=(const sampler_t& other)
  {
    if (this != &other) {
      m_dataset_size = other.m_dataset_size;
      m_policy = other.m_policy;
      m_indices = other.m_indices;
      auto offset = other.m_iter - other.m_indices.begin();
      m_iter = m_indices.begin() + offset;
    }
    return *this;
  }

  explicit sampler_t(std::size_t dataset_size, PolicyT policy = PolicyT {})
      : m_dataset_size(dataset_size)
      , m_policy(std::move(policy))
  {
    m_indices.resize(m_dataset_size);
    std::iota(m_indices.begin(), m_indices.end(), index_type {0});
    reset();
  }

  /**
   * @brief 重置索引并重新应用策略/Reset indices and reapply policy
   */
  void reset()
  {
    std::iota(m_indices.begin(), m_indices.end(), index_type {0});
    m_policy(m_indices);
    m_iter = m_indices.begin();
  }

  /**
   * @brief 是否还有下一个索引/Whether there are more indices
   */
  [[nodiscard]] bool has_next() const noexcept
  {
    return m_iter != m_indices.end();
  }

  /**
   * @brief 获取下一个索引/Get the next index
   */
  index_type next() { return *m_iter++; }

  /**
   * @brief 获取批量索引/Get a batch of indices
   */
  std::vector<index_type> next_batch(std::size_t batch_size)
  {
    std::vector<index_type> batch;
    batch.reserve(batch_size);
    for (std::size_t i = 0; i < batch_size && has_next(); ++i) {
      batch.push_back(next());
    }
    return batch;
  }

private:
  std::size_t m_dataset_size;
  PolicyT m_policy;
  std::vector<index_type> m_indices;
  typename std::vector<index_type>::iterator m_iter;
};

}  // namespace toolbox::io