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
 * @details 不对索引做任何变换，保持顺序/Does not modify indices, keeps them in
 * order
 * @code
 * std::vector<std::size_t> indices = {0, 1, 2, 3};
 * toolbox::io::sequential_policy_t policy;
 * policy(indices); // indices 仍为 {0, 1, 2, 3}/indices remains {0, 1, 2, 3}
 * @endcode
 */
struct sequential_policy_t
{
  /**
   * @brief 顺序采样操作符/Sequential sampling operator
   * @param indices 索引数组/Indices array
   */
  void operator()(std::vector<std::size_t>& /*indices*/) const noexcept {}
};

/**
 * @brief 随机打乱采样策略/Shuffle sampling policy
 * @details 使用给定种子对索引进行随机打乱/Randomly shuffles indices with given
 * seed
 * @code
 * std::vector<std::size_t> indices = {0, 1, 2, 3};
 * toolbox::io::shuffle_policy_t policy(42);
 * policy(indices); // indices 顺序被打乱/indices are shuffled
 * @endcode
 */
class shuffle_policy_t
{
public:
  /**
   * @brief 构造函数/Constructor
   * @param seed 随机种子/Random seed
   */
  explicit shuffle_policy_t(unsigned seed = std::random_device {}())
      : m_rng(seed)
  {
  }

  /**
   * @brief 打乱索引/Shuffle indices
   * @param indices 待打乱的索引数组/Indices array to shuffle
   */
  void operator()(std::vector<std::size_t>& indices)
  {
    std::shuffle(indices.begin(), indices.end(), m_rng);
  }

  /**
   * @brief 设置随机种子/Set random seed
   * @param seed 新的随机种子/New random seed
   */
  void set_seed(unsigned seed) { m_rng.seed(seed); }

private:
  /**
   * @brief 随机数生成器/Random number generator
   */
  std::mt19937 m_rng;
};

/**
 * @brief 通用采样器/Generic sampler
 * @details 支持顺序采样和随机采样等策略/Supports sequential and shuffle
 * sampling policies
 * @tparam PolicyT 采样策略类型/Sampling policy type
 * @code
 * // 顺序采样/Sequential sampling
 * toolbox::io::sampler_t<> sampler(5);
 * while (sampler.has_next()) {
 *   auto idx = sampler.next();
 * }
 * // 随机采样/Shuffle sampling
 * toolbox::io::sampler_t<toolbox::io::shuffle_policy_t> shuffle_sampler(5,
 * toolbox::io::shuffle_policy_t(123)); shuffle_sampler.reset(); auto batch =
 * shuffle_sampler.next_batch(2);
 * @endcode
 */
template<typename PolicyT = sequential_policy_t>
class CPP_TOOLBOX_EXPORT sampler_t
{
public:
  /**
   * @brief 索引类型/Index type
   */
  using index_type = std::size_t;

  /**
   * @brief 拷贝构造函数/Copy constructor
   * @param other 另一个采样器/Other sampler
   */
  sampler_t(const sampler_t& other)
      : m_dataset_size(other.m_dataset_size)
      , m_policy(other.m_policy)
      , m_indices(other.m_indices)
  {
    // 计算迭代器偏移/Calculate iterator offset
    auto offset = other.m_iter - other.m_indices.begin();
    m_iter = m_indices.begin() + offset;
  }

  /**
   * @brief 拷贝赋值操作符/Copy assignment operator
   * @param other 另一个采样器/Other sampler
   * @return 当前采样器引用/Reference to this sampler
   */
  sampler_t& operator=(const sampler_t& other)
  {
    if (this != &other) {
      m_dataset_size = other.m_dataset_size;
      m_policy = other.m_policy;
      m_indices = other.m_indices;
      // 计算迭代器偏移/Calculate iterator offset
      auto offset = other.m_iter - other.m_indices.begin();
      m_iter = m_indices.begin() + offset;
    }
    return *this;
  }

  /**
   * @brief 构造函数/Constructor
   * @param dataset_size 数据集大小/Dataset size
   * @param policy 采样策略/Sampling policy
   */
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
   * @details 重新生成索引并应用采样策略/Regenerates indices and applies
   * sampling policy
   * @code
   * sampler_t<> sampler(10);
   * sampler.reset(); // 重新洗牌/Reshuffle or reorder
   * @endcode
   */
  void reset()
  {
    std::iota(m_indices.begin(), m_indices.end(), index_type {0});
    m_policy(m_indices);
    m_iter = m_indices.begin();
  }

  /**
   * @brief 是否还有下一个索引/Whether there are more indices
   * @return 如果还有未采样索引则为true/True if there are more indices to sample
   * @code
   * while (sampler.has_next()) {
   *   auto idx = sampler.next();
   * }
   * @endcode
   */
  [[nodiscard]] bool has_next() const noexcept
  {
    return m_iter != m_indices.end();
  }

  /**
   * @brief 获取下一个索引/Get the next index
   * @return 下一个索引/Next index
   * @code
   * if (sampler.has_next()) {
   *   auto idx = sampler.next();
   * }
   * @endcode
   */
  index_type next() { return *m_iter++; }

  /**
   * @brief 获取批量索引/Get a batch of indices
   * @param batch_size 批量大小/Batch size
   * @return 批量索引数组/Batch of indices
   * @code
   * auto batch = sampler.next_batch(4);
   * @endcode
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
  /**
   * @brief 数据集大小/Dataset size
   */
  std::size_t m_dataset_size;
  /**
   * @brief 采样策略/Sampling policy
   */
  PolicyT m_policy;
  /**
   * @brief 索引数组/Indices array
   */
  std::vector<index_type> m_indices;
  /**
   * @brief 当前迭代器/Current iterator
   */
  typename std::vector<index_type>::iterator m_iter;
};

}  // namespace toolbox::io