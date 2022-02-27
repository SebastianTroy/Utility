#include <RollingStatistics.h>
#include <Random.h>

#include <catch2/catch.hpp>

using namespace util;

namespace {
constexpr int REPEAT_COUNT = 10;
}

TEST_CASE("RollingStatistics", "[stats]")
{
    Random::Seed(42);

    SECTION("Single Value")
    {
        for (int count = 0; count < REPEAT_COUNT; ++count) {
            RollingStatistics stats;
            double testValue = Random::Number<double>(-1e6, 1e6);
            stats.AddValue(testValue);
            REQUIRE(stats.Count() == 1);
            REQUIRE_THAT(stats.Mean(), Catch::Matchers::WithinRel(testValue));
            REQUIRE(stats.Min() == testValue);
            REQUIRE(stats.Max() == testValue);
        }
    }

    SECTION("Multiple of the Same Value")
    {
        for (int count = 0; count < REPEAT_COUNT; ++count) {
            RollingStatistics stats;
            double testValue = Random::Number<double>(-1e6, 1e6);
            for (size_t i = 1; i <= 10; ++i) {
                stats.AddValue(testValue);
                REQUIRE(stats.Count() == i);
                REQUIRE_THAT(stats.Mean(), Catch::Matchers::WithinRel(testValue));
                REQUIRE(stats.Min() == testValue);
                REQUIRE(stats.Max() == testValue);
            }
        }
    }

    SECTION("Average 0")
    {
        for (int count = 0; count < REPEAT_COUNT; ++count) {
            RollingStatistics stats;
            double testValue = Random::Number<double>(-1e6, 1e6);
            for (size_t i = 1; i <= 10; ++i) {
                stats.AddValue(testValue);
                stats.AddValue(-testValue);
                REQUIRE(stats.Count() == 2 * i);
                REQUIRE(stats.Mean() == 0.0);
            }
        }
    }

    SECTION("StdDev")
    {
        std::initializer_list<double> values{ 51.72660295,
                                              9.404373315,
                                              10.012679  ,
                                              65.92971394,
                                              23.39341994,
                                              4.596297412,
                                              64.18117664,
                                              63.74995874,
                                              75.4904214 ,
                                              76.60159993,
                                              36.39213087,
                                              48.60836611,
                                            };

        constexpr double expectedStdDev = 25.49590914;

        RollingStatistics stats;
        for (auto value : values) {
            stats.AddValue(value);
        }
        REQUIRE_THAT(stats.StandardDeviation(), Catch::Matchers::WithinRel(expectedStdDev, 0.000001));
    }
}
