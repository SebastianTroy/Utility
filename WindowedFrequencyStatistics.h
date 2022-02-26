#ifndef WINDOWEDFREQUENCYSTATISTICS_H
#define WINDOWEDFREQUENCYSTATISTICS_H

#include "WindowedRollingStatistics.h"

#include <chrono>

namespace Tril {

/**
 * A means of tracking the frequency of an event in Hz by looking at the
 * durations between the last few events.
 */
class WindowedFrequencyStatistics {
public:
    using Timestamp = decltype(std::chrono::steady_clock::now());

    explicit WindowedFrequencyStatistics(size_t sampleCountWindowSize);

    void AddValue();

    double MeanHz() const;
    Timestamp GetTimestampOfLastEvent() const { return previous_; }

private:
    Timestamp previous_;
    WindowedRollingStatistics intervalStats_;
};

} // end namespace Tril

#endif // WINDOWEDFREQUENCYSTATISTICS_H
