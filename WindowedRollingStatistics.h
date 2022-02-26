#ifndef WINDOWEDROLLINGSTATISTICS_H
#define WINDOWEDROLLINGSTATISTICS_H

#include "CircularBuffer.h"

#include <cmath>

namespace Tril {

/**
 * For continuously monitoring a value with statistics covering a number of
 * recent samples.
 */
class WindowedRollingStatistics {
public:
    explicit WindowedRollingStatistics(size_t windowSize);

    void AddValue(double newValue);
    void Reset();

    // Non-const for performance reasons
    double Min();
    double Max();

    size_t WindowSize() const { return values_.Capacity(); }
    size_t Count() const { return values_.Size(); }
    double Mean() const { return sumOfValues_ / Count(); }
    double StandardDeviation() const;

private:
    CircularBuffer<double> values_;
    double sumOfValues_;
    double sumOfValuesSquared_;
    double min_;
    double max_;
    bool updateMinMax_;

    void UpdateMinMax();
};

} // end namespace Tril

#endif // WINDOWEDROLLINGSTATISTICS_H
