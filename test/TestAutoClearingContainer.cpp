#include <AutoClearingContainer.h>

#include <catch2/catch.hpp>

TEST_CASE("AutoClearingContainer", "[container]")
{
    util::AutoClearingContainer<int> a;

    {
        auto handle = a.PushBack(42);

        unsigned count = 0;
        a.ForEach([&](int& value)
        {
            REQUIRE(value == 42);
            ++count;
        });
        REQUIRE(count == 1);
    }

    unsigned count = 0;
    a.ForEach([&](int& /*value*/)
    {
        ++count;
    });
    REQUIRE(count == 0);
}
