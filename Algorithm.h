#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <vector>
#include <functional>
#include <cmath>
#include <cstdint>
#include <assert.h>

namespace Tril {
template <typename T1, typename T2>
void IterateBoth(std::vector<T1>& a, std::vector<T2>& b, std::function<void(T1& a, T2& b)>&& action)
{
    auto aIter = a.begin();
    auto bIter = b.begin();
    for (; aIter != a.end() && bIter != b.end() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2>
void IterateBoth(const std::vector<T1>& a, const std::vector<T2>& b, std::function<void(const T1& a, const T2& b)>&& action)
{
    auto aIter = a.cbegin();
    auto bIter = b.cbegin();
    for (; aIter != a.cend() && bIter != b.cend() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2>
void IterateBoth(const std::vector<T1>& a, std::vector<T2>& b, std::function<void(const T1& a, T2& b)>&& action)
{
    auto aIter = a.cbegin();
    auto bIter = b.begin();
    for (; aIter != a.cend() && bIter != b.end() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2>
void IterateBoth(std::vector<T1>& a, const std::vector<T2>& b, std::function<void(T1& a, const T2& b)>&& action)
{
    auto aIter = a.begin();
    auto bIter = b.cbegin();
    for (; aIter != a.end() && bIter != b.cend() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2, size_t LEN>
void IterateBoth(std::array<T1, LEN>& a, std::array<T2, LEN>& b, std::function<void(T1& a, T2& b)>&& action)
{
    auto aIter = a.begin();
    auto bIter = b.cbegin();
    for (; aIter != a.end() && bIter != b.cend() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2, size_t LEN>
void IterateBoth(const std::array<T1, LEN>& a, const std::array<T2, LEN>& b, std::function<void(const T1& a, const T2& b)>&& action)
{
    auto aIter = a.begin();
    auto bIter = b.cbegin();
    for (; aIter != a.end() && bIter != b.cend() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2, size_t LEN>
void IterateBoth(const std::array<T1, LEN>& a, std::array<T2, LEN>& b, std::function<void(const T1& a, T2& b)>&& action)
{
    auto aIter = a.begin();
    auto bIter = b.cbegin();
    for (; aIter != a.end() && bIter != b.cend() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2, size_t LEN>
void IterateBoth(std::array<T1, LEN>& a, const std::array<T2, LEN>& b, std::function<void(T1& a, const T2& b)>&& action)
{
    auto aIter = a.begin();
    auto bIter = b.cbegin();
    for (; aIter != a.end() && bIter != b.cend() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

/**
 * Compares the content of the containers using operator== ensuring
 * that each element is equal to the element in the other container
 * at the same index.
 */
template <typename T>
bool CompareContainers(const std::vector<T>& a, const std::vector<T>& b)
{
    return a.size() == b.size() && [&]() -> bool
    {
        bool match = true;
        IterateBoth<T, T>(a, b, [&](const T& a, const T& b)
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

} // namespace Tril

#endif // ALGORITHM_H
