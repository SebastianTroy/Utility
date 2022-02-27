#ifndef RANGECONVERTER_H
#define RANGECONVERTER_H

#include "Range.h"

#include <nlohmann/json.hpp>

#include <optional>

namespace util {

class RangeConverter {
public:
    RangeConverter(const RangeConverter& other) = default;
    RangeConverter(RangeConverter&& other) = default;
    RangeConverter(Range<double> from, Range<double> to);

    static void ConfigureJsonSerialisationHelper(util::JsonSerialisationHelper<RangeConverter>& helper);

    RangeConverter& operator=(const RangeConverter& other) = default;
    RangeConverter& operator=(RangeConverter&& other) = default;
    double Convert(const double& value) const;
    double ConvertAndClamp(const double& value) const;
    const Range<double>& GetFrom() const { return from_; }
    const Range<double>& GetTo() const { return to_; }

private:
    Range<double> from_;
    Range<double> to_;
};

} // end namespace util

#endif // RANGECONVERTER_H
