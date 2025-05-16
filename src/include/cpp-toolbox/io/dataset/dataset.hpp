#pragma once

#include <cstddef>
#include <optional>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::io
{

/**
 * @brief 数据集基类/Abstract base class for datasets
 *
 * @tparam Derived 派生类类型/Derived class type
 * @tparam DataType 数据类型/Data type
 *
 * @code
 * // 示例：自定义一个点云数据集/Example: Custom point cloud dataset
 * class my_dataset : public dataset_t<my_dataset, int> {
 * public:
 *   std::size_t size_impl() const { return 100; }
 *   std::optional<int> at_impl(std::size_t idx) const { return idx < 100 ?
 * std::optional<int>(idx) : std::nullopt; }
 * };
 * my_dataset ds;
 * auto v = ds[5]; // 获取第5个元素/Get the 5th element
 * auto next = ds.get_next(); // 获取下一个元素/Get next element
 * ds.reset_iterator(); // 重置迭代器/Reset iterator
 * @endcode
 */
template<typename Derived, typename DataType>
class CPP_TOOLBOX_EXPORT dataset_t
{
public:
  /**
   * @brief 数据类型别名/Type alias for data type
   */
  using data_type = DataType;

  /**
   * @brief 获取数据集大小/Get the size of the dataset
   * @return 数据集中的元素数量/Number of elements in the dataset
   *
   * @code
   * my_dataset ds;
   * std::size_t n = ds.size(); // 获取数据集大小/Get dataset size
   * @endcode
   */
  std::size_t size() const
  {
    return static_cast<const Derived*>(this)->size_impl();
  }

  /**
   * @brief 通过下标访问数据/Access data by index
   * @param index 元素索引/Element index
   * @return 对应索引的数据（可选）/Data at the given index (optional)
   *
   * @code
   * auto item = ds[3]; // 获取第3个元素/Get the 3rd element
   * @endcode
   */
  std::optional<DataType> operator[](std::size_t index) const
  {
    return static_cast<const Derived*>(this)->at_impl(index);
  }

  /**
   * @brief 通过下标访问数据/Access data by index
   * @param index 元素索引/Element index
   * @return 对应索引的数据（可选）/Data at the given index (optional)
   *
   * @code
   * auto item = ds.at(10); // 获取第10个元素/Get the 10th element
   * @endcode
   */
  std::optional<DataType> at(std::size_t index) const
  {
    return static_cast<const Derived*>(this)->at_impl(index);
  }

  /**
   * @brief 获取指定索引的数据/Get data at the specified index
   * @param index 元素索引/Element index
   * @return 对应索引的数据（可选）/Data at the given index (optional)
   *
   * @code
   * auto item = ds.get_item(0); // 获取第0个元素/Get the 0th element
   * @endcode
   */
  std::optional<DataType> get_item(std::size_t index) const
  {
    return static_cast<const Derived*>(this)->at_impl(index);
  }

  /**
   * @brief 获取下一个元素并推进内部索引/Get the next element and advance the
   * internal index
   * @return 下一个元素（可选）/Next element (optional)
   *
   * @note 修改为非const方法，因为它改变了对象状态/Changed to non-const because
   * it modifies object state
   *
   * @code
   * ds.reset_iterator();
   * auto first = ds.get_next(); // 获取第一个元素/Get the first element
   * auto second = ds.get_next(); // 获取第二个元素/Get the second element
   * @endcode
   */
  std::optional<DataType> get_next()
  {
    return static_cast<const Derived*>(this)->at_impl(m_current_index++);
  }

  /**
   * @brief 查看下一个元素但不推进索引/Peek the next element without advancing
   * the index
   * @return 下一个元素（可选）/Next element (optional)
   *
   * @code
   * ds.reset_iterator();
   * auto peeked = ds.peek_next(); // 查看下一个元素/Peek next element
   * @endcode
   */
  std::optional<DataType> peek_next() const
  {
    return static_cast<const Derived*>(this)->at_impl(m_current_index);
  }

  /**
   * @brief 重置内部迭代器索引/Reset the internal iterator index
   *
   * @code
   * ds.reset_iterator(); // 重置到第一个元素/Reset to the first element
   * @endcode
   */
  void reset_iterator() { m_current_index = 0; }

  /**
   * @brief 获取当前迭代器索引/Get the current iterator index
   * @return 当前索引/Current index
   *
   * @code
   * std::size_t idx = ds.current_index(); // 获取当前索引/Get current index
   * @endcode
   */
  std::size_t current_index() const { return m_current_index; }

protected:
  /**
   * @brief 构造函数/Constructor
   */
  dataset_t() = default;
  /**
   * @brief 析构函数/Destructor
   */
  ~dataset_t() = default;

public:
  /**
   * @brief 禁用拷贝构造/Copy constructor is disabled
   */
  dataset_t(const dataset_t&) = delete;
  /**
   * @brief 禁用拷贝赋值/Copy assignment is disabled
   */
  dataset_t& operator=(const dataset_t&) = delete;
  /**
   * @brief 禁用移动构造/Move constructor is disabled
   */
  dataset_t(dataset_t&&) = delete;
  /**
   * @brief 禁用移动赋值/Move assignment is disabled
   */
  dataset_t& operator=(dataset_t&&) = delete;

private:
  /**
   * @brief 当前迭代器索引/Current iterator index
   */
  std::size_t m_current_index = 0;
};

}  // namespace toolbox::io
