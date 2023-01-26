#ifndef RANGECONVERTER_H
#define RANGECONVERTER_H

#include "Range.h"

#include <EasySerDes.h>

namespace util {

class RangeConverter {
public:
    RangeConverter(const RangeConverter& other) = default;
    RangeConverter(RangeConverter&& other) = default;
    RangeConverter(Range<double> from, Range<double> to);

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

template<>
class esd::Serialiser<util::RangeConverter> : public esd::ClassHelper<util::RangeConverter, util::Range<double>, util::Range<double>> {
public:
    static void Configure()
    {
        using This = ClassHelper<util::RangeConverter, util::Range<double>, util::Range<double>>;
        This::SetConstruction(
            This::CreateParameter(&util::RangeConverter::GetFrom, "from"),
            This::CreateParameter(&util::RangeConverter::GetTo, "to")
        );
    }
};

#endif // RANGECONVERTER_H
