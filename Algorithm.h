#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <functional>
#include <cmath>
#include <ranges>
#include <cstdint>
#include <assert.h>

namespace util {

template <typename T1, typename T2, typename Function>
requires std::ranges::range<T1>
      && std::ranges::range<T2>
      && (!std::is_const_v<T1>)
      && (!std::is_const_v<T2>)
      && std::invocable<Function, typename T1::value_type&, typename T2::value_type&>
void IterateBoth(T1& a, T2& b, Function&& action)
{
    auto aIter = a.begin();
    auto bIter = b.begin();
    for (; aIter != a.end() && bIter != b.end() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2, typename Function>
requires std::ranges::range<T1>
      && std::ranges::range<T2>
      && (!std::is_const_v<T2>)
      && std::invocable<Function, const typename T1::value_type&, typename T2::value_type&>
void IterateBoth(const T1& a, T2& b, Function&& action)
{
    auto aIter = a.cbegin();
    auto bIter = b.begin();
    for (; aIter != a.cend() && bIter != b.end() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2, typename Function>
requires std::ranges::range<T1>
      && std::ranges::range<T2>
      && (!std::is_const_v<T1>)
      && std::invocable<Function, typename T1::value_type&, const typename T2::value_type&>
void IterateBoth(T1& a, const T2& b, Function&& action)
{
    auto aIter = a.begin();
    auto bIter = b.cbegin();
    for (; aIter != a.end() && bIter != b.cend() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2, typename Function>
requires std::ranges::range<T1>
      && std::ranges::range<T2>
      && std::invocable<Function, const typename T1::value_type&, const typename T2::value_type&>
void IterateBoth(const T1& a, const T2& b, Function&& action)
{
    auto aIter = a.cbegin();
    auto bIter = b.cbegin();
    for (; aIter != a.cend() && bIter != b.cend() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

/**
 * Compares the content of the containers using operator== ensuring
 * that each element is equal to the element in the other container
 * at the same index.
 */
template <typename T1, typename T2>
requires std::ranges::range<T1>
      && std::ranges::range<T2>
      && std::is_same_v<std::decay_t<typename T1::value_type>, std::decay_t<typename T2::value_type>>
bool CompareContainers(const T1& a, const T2& b)
{
    return a.size() == b.size() && [&]() -> bool
    {
        bool match = true;
        IterateBoth(a, b, [&](const typename T1::value_type& a, const typename T2::value_type& b)
        {
            match = match && a == b;
        });
        return match;
    }();
}


template <typename T>
std::vector<T> CreateSeries(T firstValue, uint64_t count, std::function<T(const T& previous)>&& nextValue = [](const T& previous){ return previous + 1; })
{
    std::vector<T> series;
    series.resize(count);
    std::generate_n(std::begin(series), count, [&]() -> T
    {
        T temp = firstValue;
        firstValue = nextValue(temp);
        return temp;
    });
    return series;
}

template <typename T>
std::vector<T> CreateSeries(T firstValue, T lastValue, uint64_t count)
{
    assert(count >= 2);
    long double soFar = firstValue;
    long double increment = (lastValue - firstValue) / (count - 1);
    return CreateSeries<T>(soFar, count, [&](const T& /*prev*/) -> T
    {
        soFar += increment;
        if constexpr (std::is_integral<T>::value) {
            return static_cast<T>(std::round(soFar));
        } else {
            return static_cast<T>(soFar);
        }
    });
}

template <typename T>
std::vector<T> Combine(std::vector<T>&& first, std::vector<T>&& second)
{
    std::vector<T> combined;
    combined.swap(first);
    combined.reserve(combined.size() + second.size());
    std::move(std::begin(second), std::end(second), std::back_inserter(combined));
    return combined;
}

} // namespace util

#endif // ALGORITHM_H
