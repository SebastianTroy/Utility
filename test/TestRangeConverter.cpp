#include <RangeConverter.h>
#include <Random.h>

#include <catch2/catch.hpp>

using namespace Tril;

TEST_CASE("RangeConverter", "[Range]")
{
    Random::Seed(42);

    const double testMin = -1000000000.0;
    const double testMax = 1000000000.0;

    std::vector testRanges{
        std::make_pair(0.0, 100.0),
        std::make_pair(100.0, 0.0),
        std::make_pair(-0.0, -100.0),
        std::make_pair(-100.0, -0.0),
        std::make_pair(Random::Number(testMin/2, testMax/2), Random::Number(testMin/2, testMax/2)),
    };

    SECTION("SameRange")
    {

        for (const auto& [ first, second ] : testRanges) {
            Range<double> both(first, second);
            RangeConverter bothTheSame(both, both);

            for (double testValue : Random::Numbers(10, both.Min(), both.Max())) {
                REQUIRE_THAT(testValue, Catch::Matchers::WithinAbs(bothTheSame.Convert(testValue), 0.000001));
                REQUIRE_THAT(testValue, Catch::Matchers::WithinAbs(bothTheSame.ConvertAndClamp(testValue), 0.000001));
            }
        }
    }

    SECTION("Clamping works")
    {
        for (const auto& [ first, second ] : testRanges) {
            Range<double> both(first, second);
            RangeConverter bothTheSame(both, both);

            for (double higherValue : Random::Numbers(10, both.Max(), testMax)) {
                REQUIRE(higherValue > both.Max());
                REQUIRE_THAT(higherValue, Catch::Matchers::WithinRel(bothTheSame.Convert(higherValue)));
                REQUIRE(higherValue > bothTheSame.ConvertAndClamp(higherValue));
            }

            for (double lowerValue : Random::Numbers(10, testMin, both.Min())) {
                REQUIRE(lowerValue < both.Min());
                REQUIRE_THAT(lowerValue, Catch::Matchers::WithinRel(bothTheSame.Convert(lowerValue)));
                REQUIRE(lowerValue < bothTheSame.ConvertAndClamp(lowerValue));
            }
        }
    }
}
