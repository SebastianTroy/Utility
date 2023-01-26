#ifndef RANGE_H
#define RANGE_H

#include <EasySerDes.h>

#include <algorithm>
#include <optional>

namespace util {

template <typename T>
class Range {
public:
    Range(const T& first, const T& last);

    const T& Min() const;
    const T& Max() const;
    T ValueRange() const;
    T ValueDifference() const;
    const T& First() const;
    const T& Last() const;

    bool Contains(const T& value) const;

    double Similarity(const Range<T>& other) const;

    void SetFirst(const T& newFirst);
    void SetLast(const T& newLast);
    void SetRange(const T& first, const T& last);

    bool operator>(const Range<T>& other) const;
    bool operator<(const Range<T>& other) const;
    bool operator==(const Range<T>& other) const;
    bool operator>=(const Range<T>& other) const;
    bool operator<=(const Range<T>& other) const;

private:
    T first_;
    T last_;
};

template<typename T>
Range<T>::Range(const T& first, const T& last)
    : first_(first)
    , last_(last)
{
}

template<typename T>
const T& Range<T>::Min() const
{
    return std::min(first_, last_);
}

template<typename T>
const T& Range<T>::Max() const
{
    return std::max(first_, last_);
}

template<typename T>
T Range<T>::ValueRange() const
{
    return Max() - Min();
}

template<typename T>
T Range<T>::ValueDifference() const
{
    return first_ - last_;
}

template<typename T>
const T& Range<T>::First() const
{
    return first_;
}

template<typename T>
const T& Range<T>::Last() const
{
    return last_;
}

template<typename T>
bool Range<T>::Contains(const T& value) const
{
    return value >= Min() && value <= Max();
}

template<typename T>
double Range<T>::Similarity(const Range<T>& other) const
{
    double overlap = static_cast<double>(std::min(Max(), other.Max()) - std::max(Min(), other.Min()));
    if (overlap > 0.0) {
        double combinedRange = static_cast<double>(std::max(Max(), other.Max()) - std::min(Min(), other.Min()));
        return overlap / combinedRange;
    } else {
        return 1.0;
    }
}

template<typename T>
void Range<T>::SetFirst(const T& newFirst)
{
    first_ = newFirst;
}

template<typename T>
void Range<T>::SetLast(const T& newLast)
{
    last_ = newLast;
}

template<typename T>
void Range<T>::SetRange(const T& first, const T& last)
{
    first_ = first;
    last_ = last;
}

template<typename T>
bool Range<T>::operator>(const Range<T>& other) const
{
    return Min() > other.Min() || (Min() == other.Min() && Max() > other.Max());
}

template<typename T>
bool Range<T>::operator<(const Range<T>& other) const
{
    return Min() < other.Min() || (Min() == other.Min() && Max() < other.Max());
}

template<typename T>
bool Range<T>::operator==(const Range<T>& other) const
{
    return first_ == other.first_ && last_ == other.last_;
}

template<typename T>
bool Range<T>::operator>=(const Range<T>& other) const
{
    return *this > other || *this == other;
}

template<typename T>
bool Range<T>::operator<=(const Range<T>& other) const
{
    return *this < other || *this == other;
}

} // end namespace util

template<typename T>
class esd::Serialiser<util::Range<T>> : public esd::ClassHelper<util::Range<T>, T, T> {
public:
    static void Configure()
    {
        using This = ClassHelper<util::Range<T>, T, T>;
        This::SetConstruction(
            This::CreateParameter(&util::Range<T>::First, "First"),
            This::CreateParameter(&util::Range<T>::Last, "Last")
        );
    }
};

#endif // RANGE_H
