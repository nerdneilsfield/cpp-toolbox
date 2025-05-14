#include <cpp-toolbox/utils/random.hpp>

namespace toolbox::utils
{

// 为常见类型提供显式实例化
// 这些实例化使得这些类型的函数可以在其他编译单元中使用
// 而不需要重新编译模板

// 类成员函数的实例化
// random_t::random_int
template int random_t::random_int(int, int);
template long random_t::random_int(long, long);
template long long random_t::random_int(long long, long long);
template unsigned int random_t::random_int(unsigned int, unsigned int);
template unsigned long random_t::random_int(unsigned long, unsigned long);
template unsigned long long random_t::random_int(unsigned long long,
                                                 unsigned long long);

// random_t::random_float
template float random_t::random_float(float, float);
template double random_t::random_float(double, double);
template long double random_t::random_float(long double, long double);

// random_t::random 有参数版本
template int random_t::random(int, int);
template long random_t::random(long, long);
template long long random_t::random(long long, long long);
template unsigned int random_t::random(unsigned int, unsigned int);
template unsigned long random_t::random(unsigned long, unsigned long);
template unsigned long long random_t::random(unsigned long long,
                                             unsigned long long);
template float random_t::random(float, float);
template double random_t::random(double, double);
template long double random_t::random(long double, long double);

// random_t::random 无参数版本
template int random_t::random<int>();
template long random_t::random<long>();
template long long random_t::random<long long>();
template unsigned int random_t::random<unsigned int>();
template unsigned long random_t::random<unsigned long>();
template unsigned long long random_t::random<unsigned long long>();
template float random_t::random<float>();
template double random_t::random<double>();
template long double random_t::random<long double>();

// random_t::uniform
template float random_t::uniform(float, float);
template double random_t::uniform(double, double);
template long double random_t::uniform(long double, long double);

// random_t::gauss
template float random_t::gauss(float, float);
template double random_t::gauss(double, double);
template long double random_t::gauss(long double, long double);

// 容器相关函数的实例化
template std::vector<int> random_t::sample<std::vector<int>>(
    const std::vector<int>& population, size_t k);
template std::vector<float> random_t::sample<std::vector<float>>(
    const std::vector<float>& population, size_t k);
template std::vector<double> random_t::sample<std::vector<double>>(
    const std::vector<double>& population, size_t k);
template std::vector<std::string> random_t::sample<std::vector<std::string>>(
    const std::vector<std::string>& population, size_t k);

// 全局函数的实例化
// random
template int random<int>(int, int);
template long random<long>(long, long);
template long long random<long long>(long long, long long);
template unsigned int random<unsigned int>(unsigned int, unsigned int);
template unsigned long random<unsigned long>(unsigned long, unsigned long);
template unsigned long long random<unsigned long long>(unsigned long long,
                                                       unsigned long long);
template float random<float>(float, float);
template double random<double>(double, double);
template long double random<long double>(long double, long double);

// random无参数版本
template int random<int>();
template long random<long>();
template long long random<long long>();
template unsigned int random<unsigned int>();
template unsigned long random<unsigned long>();
template unsigned long long random<unsigned long long>();
template float random<float>();
template double random<double>();
template long double random<long double>();

// uniform
template float uniform<float>(float, float);
template double uniform<double>(double, double);
template long double uniform<long double>(long double, long double);

// gauss
template float gauss<float>(float, float);
template double gauss<double>(double, double);
template long double gauss<long double>(long double, long double);

// 全局容器函数的实例化
template std::vector<int> sample<int>(const std::vector<int>& population,
                                      size_t k);
template std::vector<float> sample<float>(const std::vector<float>& population,
                                          size_t k);
template std::vector<double> sample<double>(
    const std::vector<double>& population, size_t k);
template std::vector<std::string> sample<std::string>(
    const std::vector<std::string>& population, size_t k);

// choice_n函数的实例化
template std::vector<int> choice_n<int>(const std::vector<int>& container,
                                        size_t n);
template std::vector<float> choice_n<float>(const std::vector<float>& container,
                                            size_t n);
template std::vector<double> choice_n<double>(
    const std::vector<double>& container, size_t n);
template std::vector<std::string> choice_n<std::string>(
    const std::vector<std::string>& container, size_t n);

}  // namespace toolbox::utils
