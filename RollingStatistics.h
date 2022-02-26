#ifndef ROLLINGSTATISTICS_H
#define ROLLINGSTATISTICS_H

#include <limits>
#include <cstdint>

namespace Tril {

class RollingStatistics {
public:
    void AddValue(const double& value);

    uint64_t Count() const { return count_; }
    double Mean() const { return sumOfValues_ / count_; }
    double StandardDeviation() const;
    double Min() const { return min_; };
    double Max() const { return max_; };

    void Reset();

private:
    uint64_t count_ = 0;
    double sumOfValues_ = 0.0;
    double sumOfValuesSquared_ = 0.0;
    double min_ = std::numeric_limits<double>::max();
    double max_ = std::numeric_limits<double>::lowest();
};

} // namespace Tril

#endif // ROLLINGSTATISTICS_H
