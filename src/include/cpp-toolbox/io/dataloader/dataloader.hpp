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
 */
template<typename DatasetT, typename SamplerT>
class CPP_TOOLBOX_EXPORT dataloader_t
{
public:
  using data_type = typename DatasetT::data_type;
  using batch_type = std::vector<data_type>;
  using index_type = typename SamplerT::index_type;

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

  class iterator
  {
  public:
    iterator()
        : m_dataset_ptr(nullptr)
        , m_sampler(0)
        , m_batch_size(0)
        , m_prefetch_batches(0)
        , m_pool(nullptr)
        , m_drop_last(false)
    {
    }

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

    const batch_type& operator*() const { return m_current_batch; }
    const batch_type* operator->() const { return &m_current_batch; }

    bool operator!=(const iterator& other) const
    {
      return m_dataset_ptr != other.m_dataset_ptr;
    }

  private:
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

    DatasetT* m_dataset_ptr;
    SamplerT m_sampler;
    std::size_t m_batch_size;
    std::size_t m_prefetch_batches;
    toolbox::base::thread_pool_t* m_pool;
    bool m_drop_last;
    std::deque<std::future<batch_type>> m_queue;
    batch_type m_current_batch;
  };

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

  iterator end() { return iterator(); }

private:
  DatasetT& m_dataset;
  SamplerT m_sampler;
  std::size_t m_batch_size;
  std::size_t m_prefetch_batches;
  toolbox::base::thread_pool_t* m_pool;
  bool m_drop_last;
};

}  // namespace toolbox::io