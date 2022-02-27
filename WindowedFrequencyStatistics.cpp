#include "WindowedFrequencyStatistics.h"

util::WindowedFrequencyStatistics::WindowedFrequencyStatistics(size_t sampleCountWindowSize)
    : previous_(std::chrono::steady_clock::now())
    , intervalStats_(sampleCountWindowSize)
{
}

void util::WindowedFrequencyStatistics::AddValue()
{
    auto now = std::chrono::steady_clock::now();
    double interval = std::chrono::duration<double>(now - previous_).count();
    intervalStats_.AddValue(interval);
    previous_ = now;
}

double util::WindowedFrequencyStatistics::MeanHz() const
{
    return 1.0 / intervalStats_.Mean();
}
