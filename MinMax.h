#ifndef MINMAX_H
#define MINMAX_H

#include "Range.h"

#include <limits>
#include <algorithm>

namespace util {

template <typename T>
class MinMax {
public:
    MinMax()
        : min_(std::numeric_limits<T>::max())
        , max_(std::numeric_limits<T>::lowest())
    {}
    MinMax(const T& a, const T& b)
        : min_(std::min(a, b))
        , max_(std::max(a, b))
    {}

    bool IsValid() const { return min_ <= max_; }
    T Min() const { return IsValid() ? min_ : T{ 0 }; }
    T Max() const { return IsValid() ? max_ : T{ 0 }; }
    T Range() const { return IsValid() ? max_ - min_ : T{ 0 }; }
    bool Contains(const T& value) const { return value >= min_ && value <= max_; }
    operator util::Range<T>() const { return util::Range(Min(), Max()); }

    void ExpandToContain(const T& newValue)
    {
        if (IsValid()) {
            if (newValue < min_) {
                min_ = newValue;
            }
            if (newValue > max_) {
                max_ = newValue;
            }
        } else {
            max_ = newValue;
            min_ = newValue;
        }
    }
    void SetMin(const T& newMin)
    {
        min_ = newMin;
        max_ = std::max(max_, newMin);
    }
    void SetMax(const T& newMax)
    {
        min_ = std::min(min_, newMax);
        max_ = newMax;
    }
    void SetRange(const T& a, const T& b)
    {
        min_ = std::min(a, b);
        max_ = std::max(a, b);
    }
    void Reset()
    {
        min_ = std::numeric_limits<T>::max();
        max_ = std::numeric_limits<T>::lowest();
    }

private:
    T min_;
    T max_;
};

} // end namespace util

#endif // MINMAX_H
