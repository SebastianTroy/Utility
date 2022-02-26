#include "WindowedFrequencyStatistics.h"

Tril::WindowedFrequencyStatistics::WindowedFrequencyStatistics(size_t sampleCountWindowSize)
    : previous_(std::chrono::steady_clock::now())
    , intervalStats_(sampleCountWindowSize)
{
}

void Tril::WindowedFrequencyStatistics::AddValue()
{
    auto now = std::chrono::steady_clock::now();
    double interval = std::chrono::duration<double>(now - previous_).count();
    intervalStats_.AddValue(interval);
    previous_ = now;
}

double Tril::WindowedFrequencyStatistics::MeanHz() const
{
    return 1.0 / intervalStats_.Mean();
}
