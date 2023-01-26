#include <RangeConverter.h>
#include <Random.h>

#include <catch2/catch.hpp>

using namespace util;

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


TEST_CASE("RangeConverter serialisation", "[serialisation]")
{
    // Ensure tests are reproducable
    Random::Seed(42);

    auto randRange = []() -> Range<double>
    {
        auto rand = []() -> double
        {
            return Random::Number<double>(std::numeric_limits<double>::min() / 2.0, std::numeric_limits<double>::max() / 2.0);
        };
        return { rand(), rand() };
    };

    auto test = [](const RangeConverter& toTest)
    {
        using TestType = RangeConverter;
        auto serialised = esd::Serialise<TestType>(toTest);

        REQUIRE(esd::Validate<TestType>(serialised));

        auto deserialised = esd::DeserialiseWithoutChecks<TestType>(serialised);
        auto reserialised = esd::Serialise<TestType>(deserialised);

        REQUIRE(serialised == reserialised);
        REQUIRE(deserialised.GetFrom() == toTest.GetFrom());
        REQUIRE(deserialised.GetTo() == toTest.GetTo());
    };


    for (int i = 0; i < 5; ++i) {
        test(RangeConverter(randRange(), randRange()));
    }
}
