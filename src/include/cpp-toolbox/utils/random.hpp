#pragma once

#include <algorithm>  ///< 用于 std::shuffle, std::sample/For std::shuffle, std::sample
#include <chrono>  ///< 用于种子生成/For seeding if not using random_device for every construction
#include <iterator>  ///< 用于 std::back_inserter/For std::back_inserter
#include <limits>  ///< 用于 std::numeric_limits/For std::numeric_limits
#include <random>
#include <stdexcept>  ///< 用于 std::out_of_range/For std::out_of_range
#include <vector>

#include <cpp-toolbox/type_traits.hpp>

namespace toolbox::utils
{

/**
 * @brief 随机数工具类/Random number utility class
 *
 * 提供各种随机数生成、采样、洗牌等功能/Provides various random number
 * generation, sampling, shuffling, etc.
 *
 * @code
 * toolbox::utils::random_t rng;
 * int r = rng.randint(1, 10); // 生成1到10的随机整数/Generate random int in
 * [1,10] double f = rng.random<float>(0.0, 1.0); //
 * 生成0到1的随机浮点数/Generate random float in [0,1] std::vector<int> v =
 * {1,2,3,4,5}; rng.shuffle(v); // 打乱向量/Shuffle vector int c =
 * rng.choice(v); // 随机选一个元素/Randomly choose one element
 * @endcode
 */
class random_t
{
public:
  /**
   * @brief 构造函数，使用随机设备初始化种子/Constructor, seeds with a
   * random_device by default
   *
   * @code
   * toolbox::utils::random_t rng;
   * @endcode
   */
  random_t()
  {
    std::random_device rd;
    engine_.seed(rd());
  }

  /**
   * @brief 使用指定种子初始化/Constructor, seeds with a specific value
   * @param seed_val 种子值/Seed value
   * @code
   * toolbox::utils::random_t rng(1234);
   * @endcode
   */
  explicit random_t(unsigned int seed_val)
      : engine_(seed_val)
  {
  }

  /**
   * @brief 获取单例实例/Get singleton instance
   * @return random_t& 单例引用/Reference to singleton
   * @code
   * auto& rng = toolbox::utils::random_t::instance();
   * @endcode
   */
  static random_t& instance()
  {
    static random_t instance;
    return instance;
  }

  /**
   * @brief 重新使用随机设备设置种子/Reseed with random_device
   * @code
   * rng.seed();
   * @endcode
   */
  void seed()
  {
    std::random_device rd;
    engine_.seed(rd());
  }

  /**
   * @brief 使用指定种子设置/Reseed with specific value
   * @param s 种子值/Seed value
   * @code
   * rng.seed(42);
   * @endcode
   */
  void seed(unsigned int s) { engine_.seed(s); }

  /**
   * @brief 生成[a, b]范围内的随机整数/Generate random integer in [a, b]
   * @param a 下界/Lower bound
   * @param b 上界/Upper bound
   * @return int 随机整数/Random integer
   * @code
   * int r = rng.randint(1, 100);
   * @endcode
   */
  int randint(int a, int b)
  {
    if (a > b) {  // Python的randint行为/Python's randint behavior
      std::swap(a, b);
    }
    std::uniform_int_distribution<int> dist(a, b);
    return dist(engine_);
  }

  /**
   * @brief 生成指定整数类型的随机数/Generate random integer of given type
   * @tparam T 整数类型/Integral type
   * @param a 下界/Lower bound
   * @param b 上界/Upper bound
   * @return T 随机整数/Random integer
   * @code
   * auto r = rng.random_int<int16_t>(-10, 10);
   * @endcode
   */
  template<typename T>
  T random_int(T a, T b)
  {
    static_assert(
        std::is_integral_v<T>,
        "random_int<T>(T,T) 仅支持整数类型/only supports integral types");
    if (a > b) {  // Python的randint行为/Python's randint behavior
      std::swap(a, b);
    }
    std::uniform_int_distribution<T> dist(a, b);
    return dist(engine_);
  }

  /**
   * @brief 生成指定浮点类型的随机数/Generate random floating point number of
   * given type
   * @tparam T 浮点类型/Floating point type
   * @param a 下界/Lower bound
   * @param b 上界/Upper bound
   * @return T 随机浮点数/Random float
   * @code
   * double f = rng.random_float<double>(0.0, 1.0);
   * @endcode
   */
  template<typename T>
  T random_float(T a, T b)
  {
    static_assert(std::is_floating_point_v<T>,
                  "random_float<T>(T,T) 仅支持浮点类型/only supports floating "
                  "point types");
    std::uniform_real_distribution<T> dist(a, b);
    return dist(engine_);
  }

  /**
   * @brief 通用随机数生成函数/General random number generator
   * @tparam T 类型/Type (整数或浮点/integral or floating point)
   * @param a 下界/Lower bound
   * @param b 上界/Upper bound
   * @return T 随机数/Random value
   * @code
   * auto r = rng.random<int>(1, 10);
   * auto f = rng.random<float>(0.0f, 1.0f);
   * @endcode
   */
  template<typename T>
  T random(T a, T b)
  {
    if constexpr (std::is_integral_v<T>) {
      return random_int(a, b);
    } else if constexpr (std::is_floating_point_v<T>) {
      return random_float(a, b);
    } else {
      static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>,
                    "random<T>(T,T) 仅支持整数和浮点类型/only supports "
                    "integral and floating point types");
      return T {};  // 这行代码永远不会执行，只是为了避免编译错误/This line will
                    // never execute, just to avoid compile error
    }
  }

  /**
   * @brief 无参数版本的随机数生成函数/Random number generator without
   * parameters
   * @tparam T 类型/Type (整数或浮点/integral or floating point)
   * @return T 随机数/Random value
   * @code
   * int r = rng.random<int>();
   * float f = rng.random<float>();
   * @endcode
   */
  template<typename T>
  T random()
  {
    if constexpr (std::is_floating_point_v<T>) {
      return random_float<T>(0, 1);
    } else if constexpr (std::is_integral_v<T>) {
      return random_int<T>(0, std::numeric_limits<T>::max());
    } else {
      static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>,
                    "random<T>() 仅支持浮点和整数类型/only supports floating "
                    "point and integral types");
      return T {};  // 这行代码永远不会执行，只是为了避免编译错误/This line will
                    // never execute, just to avoid compile error
    }
  }

  /**
   * @brief 生成[a, b]范围内的均匀分布浮点数/Generate uniform random float in
   * [a, b]
   * @tparam T 浮点类型/Floating point type
   * @param a 下界/Lower bound
   * @param b 上界/Upper bound
   * @return T 随机浮点数/Random float
   * @code
   * double f = rng.uniform<double>(0.0, 10.0);
   * @endcode
   */
  template<typename T>
  T uniform(T a, T b)
  {
    std::uniform_real_distribution<T> dist(a, b);
    return dist(engine_);
  }

  /**
   * @brief 生成高斯分布（正态分布）随机数/Generate Gaussian (normal)
   * distributed random number
   * @tparam T 浮点类型/Floating point type
   * @param mu 均值/Mean
   * @param sigma 标准差/Standard deviation
   * @return T 随机数/Random value
   * @code
   * float g = rng.gauss<float>(0.0f, 1.0f);
   * @endcode
   */
  template<typename T>
  T gauss(T mu, T sigma)
  {
    static_assert(
        std::is_floating_point_v<T>,
        "gauss<T>(T,T) 仅支持浮点类型/only supports floating point types");
    std::normal_distribution<T> dist(mu, sigma);
    return dist(engine_);
  }

  /**
   * @brief 从vector中随机选择一个元素/Randomly choose one element from vector
   * @tparam T 元素类型/Element type
   * @param vec 输入vector/Input vector
   * @return T 随机元素/Random element
   * @throws std::out_of_range 如果vector为空/If vector is empty
   * @code
   * std::vector<int> v = {1,2,3};
   * int c = rng.choice(v);
   * @endcode
   */
  template<typename T>
  T choice(const std::vector<T>& vec)
  {
    if (vec.empty()) {
      throw std::out_of_range(
          "无法从空vector中选择/Cannot choose from an empty vector");
    }
    // randint是闭区间[a, b]，vector索引是[0, size-1]/randint is inclusive [a,
    // b], vector index is [0, size-1]
    return vec[randint(0, static_cast<int>(vec.size()) - 1)];
  }

  /**
   * @brief 通用容器的随机选择/General random choice for containers supporting
   * iterators and size()
   * @tparam TContainer 容器类型/Container type
   * @param container 输入容器/Input container
   * @return typename TContainer::value_type 随机元素/Random element
   * @throws std::out_of_range 如果容器为空/If container is empty
   * @code
   * std::list<int> l = {1,2,3};
   * int c = rng.choice_general(l);
   * @endcode
   */
  template<typename TContainer>
  typename TContainer::value_type choice_general(const TContainer& container)
  {
    static_assert(toolbox::traits::is_iterable_v<TContainer>,
                  "choice_general<TContainer> 仅支持可迭代容器/only supports "
                  "iterable containers");
    if (container.empty()) {
      throw std::out_of_range(
          "无法从空容器中选择/Cannot choose from an empty container");
    }
    std::uniform_int_distribution<size_t> dist(0, container.size() - 1);
    auto it = container.begin();
    std::advance(it, dist(engine_));
    return *it;
  }

  /**
   * @brief 随机打乱容器/Shuffle the container randomly
   * @tparam TContainer 容器类型/Container type
   * @param container 输入容器/Input container
   * @code
   * std::vector<int> v = {1,2,3};
   * rng.shuffle(v);
   * @endcode
   */
  template<typename TContainer>
  void shuffle(TContainer& container)
  {
    std::shuffle(container.begin(), container.end(), engine_);
  }

  /**
   * @brief 从容器中随机采样k个元素/Randomly sample k elements from container
   * @tparam TContainer 容器类型/Container type
   * @param population 输入容器/Input container
   * @param k 采样数量/Number of samples
   * @return TContainer 采样结果/Sampled container
   * @throws std::out_of_range 如果k大于容器大小/If k > population size
   * @code
   * std::vector<int> v = {1,2,3,4,5};
   * auto s = rng.sample(v, 3);
   * @endcode
   */
  template<typename TContainer>
  TContainer sample(const TContainer& population, size_t k)
  {
    if (k > population.size()) {
      throw std::out_of_range(
          "k不能大于容器大小/k cannot be greater than population size");
    }
    TContainer result;
#if __cplusplus >= 201703L
    std::sample(population.begin(),
                population.end(),
                std::back_inserter(result),
                k,
                engine_);
#else
    TContainer shuffled_population = population;
    this->shuffle(
        shuffled_population);  // 使用自身的shuffle方法/Use own shuffle method
    std::copy_n(shuffled_population.begin(), k, std::back_inserter(result));
#endif
    return result;
  }

private:
  /**
   * @brief 随机数引擎/Mersenne Twister 随机数引擎/Mersenne Twister engine
   */
  std::mt19937 engine_;
};

/**
 * @brief 生成[a, b]范围内的随机整数/Generate random integer in [a, b]
 * @param a 下界/Lower bound
 * @param b 上界/Upper bound
 * @return int 随机整数/Random integer
 * @code
 * int r = toolbox::utils::randint(1, 10);
 * @endcode
 */
inline int randint(int a, int b)
{
  return random_t::instance().randint(a, b);
}

/**
 * @brief 通用随机数生成函数/General random number generator
 * @tparam T 类型/Type
 * @param a 下界/Lower bound
 * @param b 上界/Upper bound
 * @return T 随机数/Random value
 * @code
 * auto r = toolbox::utils::random<int>(1, 10);
 * auto f = toolbox::utils::random<float>(0.0f, 1.0f);
 * @endcode
 */
template<typename T>
inline T random(T a, T b)
{
  return random_t::instance().random(a, b);
}

/**
 * @brief 无参数版本的随机数生成函数/Random number generator without parameters
 * @tparam T 类型/Type
 * @return T 随机数/Random value
 * @code
 * int r = toolbox::utils::random<int>();
 * float f = toolbox::utils::random<float>();
 * @endcode
 */
template<typename T>
inline T random()
{
  if constexpr (std::is_floating_point_v<T>) {
    return random_t::instance().random<T>(0, 1);
  } else if constexpr (std::is_integral_v<T>) {
    return random_t::instance().random<T>(0, std::numeric_limits<T>::max());
  } else {
    static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>,
                  "random<T>() 仅支持浮点和整数类型/only supports floating "
                  "point and integral types");
    return T {};  // 这行代码永远不会执行，只是为了避免编译错误/This line will
                  // never execute, just to avoid compile error
  }
}

/**
 * @brief 生成[a, b]范围内的均匀分布浮点数/Generate uniform random float in [a,
 * b]
 * @tparam T 浮点类型/Floating point type
 * @param a 下界/Lower bound
 * @param b 上界/Upper bound
 * @return T 随机浮点数/Random float
 * @code
 * double f = toolbox::utils::uniform<double>(0.0, 10.0);
 * @endcode
 */
template<typename T>
inline T uniform(T a, T b)
{
  return random_t::instance().uniform(a, b);
}

/**
 * @brief 生成高斯分布（正态分布）随机数/Generate Gaussian (normal) distributed
 * random number
 * @tparam T 浮点类型/Floating point type
 * @param mu 均值/Mean
 * @param sigma 标准差/Standard deviation
 * @return T 随机数/Random value
 * @code
 * float g = toolbox::utils::gauss<float>(0.0f, 1.0f);
 * @endcode
 */
template<typename T>
inline T gauss(T mu, T sigma)
{
  return random_t::instance().gauss(mu, sigma);
}

/**
 * @brief 从vector中随机选择一个元素/Randomly choose one element from vector
 * @tparam T 元素类型/Element type
 * @param vec 输入vector/Input vector
 * @return T 随机元素/Random element
 * @code
 * std::vector<int> v = {1,2,3};
 * int c = toolbox::utils::choice(v);
 * @endcode
 */
template<typename T>
inline T choice(const std::vector<T>& vec)
{
  return random_t::instance().choice(vec);
}

/**
 * @brief 从vector中随机选择n个元素/Randomly choose n elements from vector
 * @tparam T 元素类型/Element type
 * @param vec 输入vector/Input vector
 * @param n 选择数量/Number of elements to choose
 * @return std::vector<T> 结果/Result vector
 * @code
 * std::vector<int> v = {1,2,3,4,5};
 * auto s = toolbox::utils::choice_n(v, 3);
 * @endcode
 */
template<typename T>
inline std::vector<T> choice_n(const std::vector<T>& vec, size_t n)
{
  return random_t::instance().sample(vec, n);
}

/**
 * @brief 从vector中随机采样k个元素/Randomly sample k elements from vector
 * @tparam T 元素类型/Element type
 * @param population 输入vector/Input vector
 * @param k 采样数量/Number of samples
 * @return std::vector<T> 采样结果/Sampled vector
 * @code
 * std::vector<int> v = {1,2,3,4,5};
 * auto s = toolbox::utils::sample(v, 2);
 * @endcode
 */
template<typename T>
inline std::vector<T> sample(const std::vector<T>& population, size_t k)
{
  return random_t::instance().sample(population, k);
}

/**
 * @brief 通用容器的随机选择/Randomly choose one element from general container
 * @tparam TContainer 容器类型/Container type
 * @param container 输入容器/Input container
 * @return typename TContainer::value_type 随机元素/Random element
 * @code
 * std::list<int> l = {1,2,3};
 * int c = toolbox::utils::choice(l);
 * @endcode
 */
template<typename TContainer>
inline typename TContainer::value_type choice(const TContainer& container)
{
  static_assert(
      toolbox::traits::is_iterable_v<TContainer>,
      "choice<TContainer> 仅支持可迭代容器/only supports iterable containers");
  return random_t::instance().choice_general(container);
}

/**
 * @brief 通用容器的随机选择n个元素/Randomly choose n elements from general
 * container
 * @tparam TContainer 容器类型/Container type
 * @param container 输入容器/Input container
 * @param n 选择数量/Number of elements to choose
 * @return TContainer 结果/Result container
 * @code
 * std::set<int> s = {1,2,3,4,5};
 * auto r = toolbox::utils::choice_n(s, 2);
 * @endcode
 */
template<typename TContainer>
inline TContainer choice_n(const TContainer& container, size_t n)
{
  static_assert(toolbox::traits::is_iterable_v<TContainer>,
                "choice_n<TContainer> 仅支持可迭代容器/only supports iterable "
                "containers");
  return random_t::instance().sample(container, n);
}

/**
 * @brief 通用容器的随机采样n个元素/Randomly sample n elements from general
 * container
 * @tparam TContainer 容器类型/Container type
 * @param container 输入容器/Input container
 * @param n 采样数量/Number of samples
 * @return TContainer 采样结果/Sampled container
 * @code
 * std::vector<int> v = {1,2,3,4,5};
 * auto s = toolbox::utils::sample(v, 3);
 * @endcode
 */
template<typename TContainer>
inline TContainer sample(const TContainer& container, size_t n)
{
  static_assert(
      toolbox::traits::is_iterable_v<TContainer>,
      "sample<TContainer> 仅支持可迭代容器/only supports iterable containers");
  return random_t::instance().sample(container, n);
}

/**
 * @brief 随机打乱容器/Shuffle the container randomly
 * @tparam TContainer 容器类型/Container type
 * @param container 输入容器/Input container
 * @code
 * std::vector<int> v = {1,2,3};
 * toolbox::utils::shuffle(v);
 * @endcode
 */
template<typename TContainer>
inline void shuffle(TContainer& container)
{
  random_t::instance().shuffle(container);
}

}  // namespace toolbox::utils