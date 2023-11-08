#include "Colour.h"

namespace util {

AveragedColour::AveragedColour()
    : count_(0)
    , r_(0)
    , g_(0)
    , b_(0)
    , a_(0)
{
}

AveragedColour::AveragedColour(uint32_t rgba)
    : AveragedColour(Red(rgba), Green(rgba), Blue(rgba), Alpha(rgba))
{
}

AveragedColour::AveragedColour(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
    : count_(1)
    , r_(r & 0xFF)
    , g_(g & 0xFF)
    , b_(b & 0xFF)
    , a_(a & 0xFF)
{
}

uint32_t AveragedColour::Rgba() const
{
    return util::Rgba(r_ / count_, g_ / count_, b_ / count_, a_ / count_);
}

AveragedColour& AveragedColour::operator+=(const AveragedColour& other)
{
    *this = *this + other;
    return *this;
}

AveragedColour operator+(const AveragedColour& a, const AveragedColour& b)
{
    AveragedColour c = a;
    c.count_ += b.count_;
    c.r_ += b.r_;
    c.g_ += b.g_;
    c.b_ += b.b_;
    c.a_ += b.a_;
    return c;
}

} // end namespace util
