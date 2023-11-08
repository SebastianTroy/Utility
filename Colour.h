#ifndef COLOUR_H
#define COLOUR_H

#include <cstdint>

namespace util {

inline uint32_t Red(uint32_t rgba) { return rgba >> 24; }
inline uint32_t Green(uint32_t rgba) { return (rgba >> 16) & 0xFF; }
inline uint32_t Blue(uint32_t rgba) { return (rgba >> 8) & 0xFF; }
inline uint32_t Alpha(uint32_t rgba) { return rgba & 0xFF; }
inline uint32_t Rgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    return ((r & 0xFF) << 24) | ((g & 0xFF) << 16) | ((b & 0xFF) << 8) | (a & 0xFF);
}

class AveragedColour {
public:
    AveragedColour();
    AveragedColour(const AveragedColour& other) = default;
    AveragedColour(uint32_t rgba);
    AveragedColour(uint32_t r, uint32_t g, uint32_t b, uint32_t a);

    uint32_t Count() const { return count_; }
    uint32_t R() const { return r_ / count_; }
    uint32_t G() const { return g_ / count_; }
    uint32_t B() const { return b_ / count_; }
    uint32_t A() const { return a_ / count_; }
    uint32_t Rgb() const { return Rgba() | 0xFF; }
    uint32_t Rgba() const;

    AveragedColour& operator=(const AveragedColour& other) = default;
    AveragedColour& operator+=(const AveragedColour& other);
    friend AveragedColour operator+(const AveragedColour& a, const AveragedColour&  b);

private:
    uint32_t count_;
    uint32_t r_;
    uint32_t g_;
    uint32_t b_;
    uint32_t a_;
};

} // end namespace util

#endif // COLOUR_H
