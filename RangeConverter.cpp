#include "RangeConverter.h"

#include "JsonHelpers.h"

using namespace nlohmann;

namespace util {

RangeConverter::RangeConverter(Range<double> from, Range<double> to)
    : from_(from)
    , to_(to)
{
}

void RangeConverter::ConfigureJsonSerialisationHelper(util::JsonSerialisationHelper<RangeConverter>& helper)
{
    helper.RegisterConstructor(helper.CreateParameter("From", &RangeConverter::from_),
                               helper.CreateParameter("To", &RangeConverter::to_)
                               );
}

double RangeConverter::Convert(const double& value) const
{
    double proportion = (value - from_.First()) / from_.ValueDifference();
    return to_.First() + (proportion * to_.ValueDifference());
}

double RangeConverter::ConvertAndClamp(const double& value) const
{
    return std::clamp(Convert(value), to_.Min(), to_.Max());
}

} // end namespace util
