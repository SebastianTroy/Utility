#ifndef MATHCONSTANTS_H
#define MATHCONSTANTS_H

#include <numbers>

namespace util {

constexpr double Tau = std::numbers::pi * 2;

constexpr inline double ToRadians(double degrees)
{
    return degrees * std::numbers::pi / 180.0;
}

constexpr inline double ToDegrees(double radians)
{
    return radians / (std::numbers::pi / 180.0);
}

} // end namespace util

#endif // MATHCONSTANTS_H
