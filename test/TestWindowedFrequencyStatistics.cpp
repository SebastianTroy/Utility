#include <WindowedFrequencyStatistics.h>
#include <Random.h>

#include <catch2/catch.hpp>
#include <fmt/core.h>

#include <thread>

using namespace util;

namespace {
    // These tests can run for a long time, increase these if retesting is required
    inline constexpr size_t WINDOW_SIZE = 3;
    inline std::chrono::milliseconds SHORT_WAIT = std::chrono::milliseconds(1);
    inline std::chrono::milliseconds LONG_WAIT = std::chrono::milliseconds(10);

    template <typename T>
    double GetHz(const T& start, const T& end, size_t count)
    {
        double secondsPassed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
        double frequency = count / secondsPassed;
        return frequency;
    }

}

TEST_CASE("WindowedFrequencyStatistics", "[stats]")
{
    SECTION("Evenly spaced events")
    {
        for (size_t eventCount = 1; eventCount < WINDOW_SIZE * 2; ++eventCount) {
            WindowedFrequencyStatistics testStats(WINDOW_SIZE);
            auto startTime = testStats.GetTimestampOfLastEvent();

            for (size_t i = 0; i < eventCount; ++i) {
                std::this_thread::sleep_for(LONG_WAIT);
                testStats.AddValue();

                // We don't want to include events that are no longer in our buffer
                // in the comparison, startTime -> endTime should be the events
                // in the buffer only or the check will be unfair and could fail
                // due to the inherent unreliableness of sleeping a thread for an
                // exact time period
                if (eventCount > WINDOW_SIZE && i < eventCount - WINDOW_SIZE) {
                    startTime = testStats.GetTimestampOfLastEvent();
                }
            }

            auto endTime = testStats.GetTimestampOfLastEvent();
            double expectedFrequency = GetHz(startTime, endTime, std::min(eventCount, WINDOW_SIZE));
            REQUIRE_THAT(testStats.MeanHz(), Catch::Matchers::WithinRel(expectedFrequency, 0.000001));
        }
    }

    SECTION("Randomly spaced events")
    {
        for (size_t eventCount = 1; eventCount < WINDOW_SIZE * 2; ++eventCount) {
            WindowedFrequencyStatistics testStats(WINDOW_SIZE);
            auto startTime = testStats.GetTimestampOfLastEvent();

            for (size_t i = 0; i < eventCount; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds{Random::Number(SHORT_WAIT.count(), LONG_WAIT.count())});
                testStats.AddValue();

                // We don't want to include events that are no longer in our buffer
                // in the comparison, startTime -> endTime should be the events
                // in the buffer only or the check will be unfair and could fail
                // due to the inherent unreliableness of sleeping a thread for an
                // exact time period
                if (eventCount > WINDOW_SIZE && i < eventCount - WINDOW_SIZE) {
                    startTime = testStats.GetTimestampOfLastEvent();
                }
            }

            auto endTime = testStats.GetTimestampOfLastEvent();
            double expectedFrequency = GetHz(startTime, endTime, std::min(eventCount, WINDOW_SIZE));
            REQUIRE_THAT(testStats.MeanHz(), Catch::Matchers::WithinRel(expectedFrequency, 0.000001));
        }
    }

    SECTION("Clustered events")
    {
        for (size_t eventCount = 2; eventCount < WINDOW_SIZE * 2; ++eventCount) {
            WindowedFrequencyStatistics testStats(WINDOW_SIZE);
            auto startTime = testStats.GetTimestampOfLastEvent();

            for (size_t i = 0; i < eventCount - 1; ++i) {
                std::this_thread::sleep_for(SHORT_WAIT);
                testStats.AddValue();

                // We don't want to include events that are no longer in our buffer
                // in the comparison, startTime -> endTime should be the events
                // in the buffer only or the check will be unfair and could fail
                // due to the inherent unreliableness of sleeping a thread for an
                // exact time period
                if (eventCount > WINDOW_SIZE && i < eventCount - WINDOW_SIZE) {
                    startTime = testStats.GetTimestampOfLastEvent();
                }
            }
            std::this_thread::sleep_for(LONG_WAIT * eventCount);
            testStats.AddValue();

            auto endTime = testStats.GetTimestampOfLastEvent();
            double expectedFrequency = GetHz(startTime, endTime, std::min(eventCount, WINDOW_SIZE));
            REQUIRE_THAT(testStats.MeanHz(), Catch::Matchers::WithinRel(expectedFrequency, 0.000001));
        }
    }
}
