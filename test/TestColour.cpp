#include <catch2/catch.hpp>

#include <Colour.h>
#include <Random.h>

using namespace util;

TEST_CASE("AverageColour", "[colour]")
{
    SECTION("Single Colour Maintained")
    {
        AveragedColour a(0xAAAAAAAA);
        AveragedColour a2(170, 170, 170, 170);
        AveragedColour b(0x99999999);
        AveragedColour b2(153, 153, 153, 153);
        REQUIRE(a.Rgba() == 0xAAAAAAAA);
        REQUIRE(a2.Rgba() == 0xAAAAAAAA);
        REQUIRE(b.Rgba() == 0x99999999);
        REQUIRE(b2.Rgba() == 0x99999999);
    }

    SECTION("Counts Maintained")
    {
        AveragedColour a;
        REQUIRE(a.Count() == 0);
        AveragedColour b(0x00000000);
        REQUIRE(b.Count() == 1);

        for (uint32_t i = 0; i < 23; ++i) {
            if (i % 2 == 0) {
                a += 0x00000000;
                REQUIRE(a.Count() == 1 + (i / 2));
            }
            b += 0x00000000;
            REQUIRE(b.Count() == i + 2);
        }

        uint32_t expectedTotal = a.Count() + b.Count();
        REQUIRE((a + b).Count() == expectedTotal);
    }

    SECTION("Colour Components Extracted Correctly")
    {
        std::vector<uint32_t> testValues { 0x00, 0xFF, 0xAA, 0x99, 0xF0, 0x0F, 0x9A, 0xA9 };
        uint32_t noRed = 0x00FFFFFF;
        uint32_t noGreen = 0xFF00FFFF;
        uint32_t noBlue = 0xFFFF00FF;
        uint32_t noAlpha = 0xFFFFFF00;
        for (uint32_t v : testValues) {
            REQUIRE(Red(noRed | (v << 24)) == v);
            REQUIRE(Green(noGreen | (v << 16)) == v);
            REQUIRE(Blue(noBlue | (v << 8)) == v);
            REQUIRE(Alpha(noAlpha | (v << 0)) == v);
            REQUIRE(Red(Rgba(v, 0, 0, 0)) == v);
            REQUIRE(Green(Rgba(0, v, 0, 0)) == v);
            REQUIRE(Blue(Rgba(0, 0, v, 0)) == v);
            REQUIRE(Alpha(Rgba(0, 0, 0, v)) == v);
        }
    }

    SECTION("Colour Averaging")
    {
        SECTION("Same Colour")
        {
            uint32_t testRgba = 0x8CF103DE;
            AveragedColour c(testRgba);
            REQUIRE(c.Rgba() == testRgba);
            for (int i = 0; i < 15; ++i) {
                c += testRgba;
                REQUIRE(c.Rgba() == testRgba);
                REQUIRE(c.R() == Red(testRgba));
                REQUIRE(c.G() == Green(testRgba));
                REQUIRE(c.B() == Blue(testRgba));
                REQUIRE(c.A() == Alpha(testRgba));
            }
        }

        SECTION("Black and White")
        {
            AveragedColour a(0x00000000);
            AveragedColour b(0xFFFFFFFF);
            REQUIRE((a + b).Rgba() == 0x7F7F7F7F);
        }

        SECTION("Sixteenths")
        {
            for (int blackCount = 0; blackCount <= 16; ++blackCount) {
                AveragedColour black(0x00000000);
                AveragedColour white(0xFFFFFFFF);
                AveragedColour target;
                for (int i = 0; i < 16; ++i) {
                    target += (i < blackCount) ? black : white;
                }
                uint32_t hexPair = (0xFF * (16 - blackCount)) / 16;
                uint32_t expectedColour = (hexPair << 24) | (hexPair << 16) | (hexPair << 8) | (hexPair << 0);
                REQUIRE(target.Rgba() == expectedColour);
            }
        }

        SECTION("Random")
        {
            Random::Seed(0x45673829);
            std::vector<uint32_t> colours = Random::Numbers<uint32_t>(37, 0x00000000, 0xFFFFFFFF);

            AveragedColour mean;
            uint32_t rTotal = 0;
            uint32_t gTotal = 0;
            uint32_t bTotal = 0;
            uint32_t aTotal = 0;
            for (uint32_t c : colours) {
                mean += c;
                // Ok to test against these, they were checked earlier!
                rTotal += Red(c);
                gTotal += Green(c);
                bTotal += Blue(c);
                aTotal += Alpha(c);
            }
            REQUIRE(mean.R() == rTotal / colours.size());
            REQUIRE(mean.G() == gTotal / colours.size());
            REQUIRE(mean.B() == bTotal / colours.size());
            REQUIRE(mean.A() == aTotal / colours.size());
        }
    }
}
