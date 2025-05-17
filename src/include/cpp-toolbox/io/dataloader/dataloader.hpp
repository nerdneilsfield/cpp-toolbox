#pragma once

#include <cstddef>
#include <deque>
#include <future>
#include <utility>
#include <vector>

#include <cpp-toolbox/base/thread_pool.hpp>
#include <cpp-toolbox/cpp-toolbox_export.hpp>

#include "cpp-toolbox/io/dataloader/sampler.hpp"

namespace toolbox::io
{

/**
 * @brief 创建一个已完成的future/Creates a ready future
 * @tparam T 值类型/Value type
 * @param value 要存储的值/The value to store in the future
 * @return std::future<T> 已完成的future/A ready future containing the value
 * @code
 * // 示例：创建一个已完成的future/Example: create a ready future
 * auto fut = make_ready_future<int>(42);
 * int result = fut.get(); // result为42/result is 42
 * @endcode
 */
template<typename T>
static std::future<T> make_ready_future(T value)
{
  std::promise<T> p;
  auto f = p.get_future();
  p.set_value(std::move(value));
  return f;
}

/**
 * @brief 通用数据加载器/Generic data loader
 *
 * @tparam DatasetT 数据集类型/Dataset type
 * @tparam SamplerT 采样器类型/Sampler type
 *
 * @code
 * // 示例：创建并使用dataloader_t/Example: create and use dataloader_t
 * kitti_pcd_dataset_t dataset("/path/to/velodyne");
 * random_sampler_t sampler(dataset.size());
 * dataloader_t<kitti_pcd_dataset_t, random_sampler_t> loader(dataset, sampler,
 * 4); for (auto batch : loader) {
 *     // 处理每个batch/Process each batch
 * }
 * @endcode
 */
template<typename DatasetT, typename SamplerT>
class CPP_TOOLBOX_EXPORT dataloader_t
{
public:
  /**
   * @brief 数据类型/Data type
   */
  using data_type = typename DatasetT::data_type;
  /**
   * @brief 批次类型/Batch type
   */
  using batch_type = std::vector<data_type>;
  /**
   * @brief 索引类型/Index type
   */
  using index_type = typename SamplerT::index_type;

  /**
   * @brief 构造函数/Constructor
   * @param dataset 数据集引用/Reference to dataset
   * @param sampler 采样器/Sampler
   * @param batch_size 批次大小/Batch size
   * @param prefetch_batches 预取批次数/Number of prefetch batches
   * @param pool 线程池指针/Pointer to thread pool
   * @param drop_last 是否丢弃最后不足batch的样本/Whether to drop last
   * incomplete batch
   * @code
   * // 示例：带线程池和预取/Example: with thread pool and prefetch
   * thread_pool_t pool(4);
   * dataloader_t<kitti_pcd_dataset_t, random_sampler_t> loader(dataset,
   * sampler, 8, 2, &pool, true);
   * @endcode
   */
  dataloader_t(DatasetT& dataset,
               SamplerT sampler,
               std::size_t batch_size,
               std::size_t prefetch_batches = 0,
               toolbox::base::thread_pool_t* pool = nullptr,
               bool drop_last = false)
      : m_dataset(dataset)
      , m_sampler(std::move(sampler))
      , m_batch_size(batch_size)
      , m_prefetch_batches(prefetch_batches)
      , m_pool(pool)
      , m_drop_last(drop_last)
  {
  }

  /**
   * @brief 数据加载器迭代器/Iterator for dataloader
   *
   * 支持多线程预取和批量加载/Supports multi-threaded prefetch and batch loading
   */
  class iterator
  {
  public:
    /**
     * @brief 默认构造函数/Default constructor
     */
    iterator()
        : m_dataset_ptr(nullptr)
        , m_sampler(0)
        , m_batch_size(0)
        , m_prefetch_batches(0)
        , m_pool(nullptr)
        , m_drop_last(false)
    {
    }

    /**
     * @brief 构造函数/Constructor
     * @param dataset 数据集指针/Pointer to dataset
     * @param sampler 采样器/Sampler
     * @param batch_size 批次大小/Batch size
     * @param prefetch_batches 预取批次数/Number of prefetch batches
     * @param pool 线程池指针/Pointer to thread pool
     * @param drop_last 是否丢弃最后不足batch的样本/Whether to drop last
     * incomplete batch
     */
    iterator(DatasetT* dataset,
             SamplerT sampler,
             std::size_t batch_size,
             std::size_t prefetch_batches,
             toolbox::base::thread_pool_t* pool,
             bool drop_last)
        : m_dataset_ptr(dataset)
        , m_sampler(std::move(sampler))
        , m_batch_size(batch_size)
        , m_prefetch_batches(prefetch_batches)
        , m_pool(pool)
        , m_drop_last(drop_last)
    {
      for (std::size_t i = 0; i < m_prefetch_batches && m_sampler.has_next();
           ++i)
      {
        enqueue_fetch();
      }
      if (m_prefetch_batches == 0 && m_sampler.has_next()) {
        enqueue_fetch();
      }
      ++(*this);
    }

    /**
     * @brief 前置自增运算符/Prefix increment operator
     * @return 迭代器引用/Reference to iterator
     */
    iterator& operator++()
    {
      if (!m_dataset_ptr) {
        return *this;
      }

      if (m_queue.empty()) {
        m_dataset_ptr = nullptr;
        return *this;
      }

      auto fut = std::move(m_queue.front());
      m_queue.pop_front();
      m_current_batch = fut.get();

      if (m_sampler.has_next()) {
        enqueue_fetch();
      }

      if (m_current_batch.size() < m_batch_size && m_drop_last) {
        m_dataset_ptr = nullptr;
      }

      if (m_queue.empty() && !m_sampler.has_next()) {
        if (m_current_batch.empty()
            || (m_drop_last && m_current_batch.size() < m_batch_size))
        {
          m_dataset_ptr = nullptr;
        }
      }
      return *this;
    }

    /**
     * @brief 解引用运算符/Dereference operator
     * @return 当前批次的常量引用/Const reference to current batch
     */
    const batch_type& operator*() const { return m_current_batch; }

    /**
     * @brief 指针运算符/Pointer operator
     * @return 当前批次的常量指针/Const pointer to current batch
     */
    const batch_type* operator->() const { return &m_current_batch; }

    /**
     * @brief 不等于运算符/Not-equal operator
     * @param other 另一个迭代器/Other iterator
     * @return 是否不等/Whether not equal
     */
    bool operator!=(const iterator& other) const
    {
      return m_dataset_ptr != other.m_dataset_ptr;
    }

  private:
    /**
     * @brief 异步预取下一个批次/Asynchronously prefetch next batch
     */
    void enqueue_fetch()
    {
      auto indices = m_sampler.next_batch(m_batch_size);
      if (indices.empty()) {
        return;
      }

      DatasetT* dataset_ptr = m_dataset_ptr;
      auto task = [dataset_ptr, indices = std::move(indices)]()
      {
        batch_type batch;
        batch.reserve(indices.size());
        for (auto idx : indices) {
          auto item = dataset_ptr->get_item(idx);
          if (item) {
            batch.push_back(std::move(*item));
          }
        }
        return batch;
      };

      if (m_pool) {
        m_queue.push_back(m_pool->submit(task));
      } else {
        m_queue.push_back(make_ready_future(task()));
      }
    }

    /**
     * @brief 数据集指针/Pointer to dataset
     */
    DatasetT* m_dataset_ptr;
    /**
     * @brief 采样器/Sampler
     */
    SamplerT m_sampler;
    /**
     * @brief 批次大小/Batch size
     */
    std::size_t m_batch_size;
    /**
     * @brief 预取批次数/Number of prefetch batches
     */
    std::size_t m_prefetch_batches;
    /**
     * @brief 线程池指针/Pointer to thread pool
     */
    toolbox::base::thread_pool_t* m_pool;
    /**
     * @brief 是否丢弃最后不足batch的样本/Whether to drop last incomplete batch
     */
    bool m_drop_last;
    /**
     * @brief 批次future队列/Queue of batch futures
     */
    std::deque<std::future<batch_type>> m_queue;
    /**
     * @brief 当前批次/Current batch
     */
    batch_type m_current_batch;
  };

  /**
   * @brief 获取迭代起始点/Get begin iterator
   * @return 迭代器/Iterator
   * @code
   * // 示例：遍历所有批次/Example: iterate all batches
   * for (auto it = loader.begin(); it != loader.end(); ++it) {
   *     auto& batch = *it;
   * }
   * @endcode
   */
  iterator begin()
  {
    m_sampler.reset();
    return iterator(&m_dataset,
                    m_sampler,
                    m_batch_size,
                    m_prefetch_batches,
                    m_pool,
                    m_drop_last);
  }

  /**
   * @brief 获取迭代终止点/Get end iterator
   * @return 迭代器/Iterator
   */
  iterator end() { return iterator(); }

private:
  /**
   * @brief 数据集引用/Reference to dataset
   */
  DatasetT& m_dataset;
  /**
   * @brief 采样器/Sampler
   */
  SamplerT m_sampler;
  /**
   * @brief 批次大小/Batch size
   */
  std::size_t m_batch_size;
  /**
   * @brief 预取批次数/Number of prefetch batches
   */
  std::size_t m_prefetch_batches;
  /**
   * @brief 线程池指针/Pointer to thread pool
   */
  toolbox::base::thread_pool_t* m_pool;
  /**
   * @brief 是否丢弃最后不足batch的样本/Whether to drop last incomplete batch
   */
  bool m_drop_last;
};

}  // namespace toolbox::io