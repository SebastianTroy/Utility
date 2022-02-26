#include <WindowedRollingStatistics.h>
#include <RollingStatistics.h>
#include <Random.h>

#include <catch2/catch.hpp>

#include <thread>

using namespace Tril;

TEST_CASE("WindowedRollingStatistics", "[stats]")
{
    SECTION("Unwindowed results match RollingStatistics")
    {
        constexpr size_t windowSize = 100;
        RollingStatistics referenceStats;
        WindowedRollingStatistics testStats(windowSize);

        for (size_t i = 0; i < windowSize; ++i) {
            double testValue = Random::Number<double>(-100000, 100000);

            referenceStats.AddValue(testValue);
            testStats.AddValue(testValue);

            REQUIRE(testStats.Count() == i + 1);
            REQUIRE(testStats.Count() == referenceStats.Count());
            REQUIRE(testStats.Mean() == referenceStats.Mean());
            REQUIRE(testStats.Min() == referenceStats.Min());
            REQUIRE(testStats.Max() == referenceStats.Max());
            REQUIRE(testStats.StandardDeviation() == referenceStats.StandardDeviation());
        }
    }

    SECTION("New values correctly overwrite old values")
    {
        constexpr size_t windowSize = 100;
        RollingStatistics referenceStats;
        WindowedRollingStatistics testStats(windowSize);

        // Write some values that will be overwritten
        for (size_t i = 0; i < windowSize / 4; ++i) {
            testStats.AddValue(Random::Number<double>(-100000, 100000));
        }

        // Stats will disagree until the entire window has ben overwritten with new values
        for (size_t i = 0; i < windowSize; ++i) {
            double testValue = Random::Number<double>(-100000, 100000);
            referenceStats.AddValue(testValue);
            testStats.AddValue(testValue);
        }

        REQUIRE(testStats.Mean() == referenceStats.Mean());
        REQUIRE(testStats.Min() == referenceStats.Min());
        REQUIRE(testStats.Max() == referenceStats.Max());
        REQUIRE(testStats.StandardDeviation() == referenceStats.StandardDeviation());
    }

    SECTION("Windows contains the expected values as it rolls over")
    {
        constexpr size_t windowSize = 50;
        std::vector<double> testValues =  Random::Numbers<double>(windowSize * 3, -100000, 100000);
        WindowedRollingStatistics testStats(windowSize);

        unsigned beginIndex = 0;
        for (double testValue : testValues) {
            testStats.AddValue(testValue);

            // Ignore partial fills, they have already been tested
            if (testStats.Count() > windowSize) {
                // Setup a reference stats that contains the same values as out windowed rolling stats
                RollingStatistics referenceStats;
                for (unsigned index = beginIndex; index < beginIndex + windowSize; ++index) {
                    referenceStats.AddValue(testValues.at(index));
                }
                REQUIRE(testStats.Mean() == referenceStats.Mean());
                REQUIRE(testStats.Min() == referenceStats.Min());
                REQUIRE(testStats.Max() == referenceStats.Max());
                REQUIRE(testStats.StandardDeviation() == referenceStats.StandardDeviation());
            }
            ++beginIndex;
        }
    }
}
