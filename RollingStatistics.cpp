#include "RollingStatistics.h"

#include <algorithm>
#include <cmath>

namespace Tril {

void RollingStatistics::AddValue(const double& value)
{
    count_++;
    sumOfValues_ += value;
    sumOfValuesSquared_ += std::pow(value, 2);
    min_ = std::min(min_, value);
    max_ = std::max(max_, value);
}

double RollingStatistics::StandardDeviation() const
{
    if (count_ > 1) {
        return std::sqrt((sumOfValuesSquared_ - (std::pow(sumOfValues_, 2) / count_)) / count_);
    } else {
        return 0;
    }
}

void RollingStatistics::Reset()
{
    count_ = 0;
    sumOfValues_ = 0.0;
    sumOfValuesSquared_ = 0.0;
    min_ = std::numeric_limits<double>::max();
    max_ = std::numeric_limits<double>::lowest();
}

} // namespace Tril
