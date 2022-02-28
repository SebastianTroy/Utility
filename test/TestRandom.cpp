#include <Random.h>

#include <catch2/catch.hpp>

TEST_CASE("Merge", "[random]")
{
    Random::Seed(42);
    size_t count = 100;

    SECTION("Same Size")
    {
        std::vector<int> a(count, 5);
        std::vector<int> b(count, 0);

        std::vector<int> c = Random::Merge(a, b);

        REQUIRE(c.size() == count);
        int zeros = 0;
        int fives = 0;
        for (const auto& item : c) {
            if (item == 0) {
                ++zeros;
            } else if (item == 5) {
                ++fives;
            }
            REQUIRE((item == 5 || item == 0));
        }
        REQUIRE(std::abs(zeros - fives) < static_cast<int>(count / 10));
    }

    SECTION("Different Sizes")
    {
        std::vector<int> a(count, 5);
        std::vector<int> b(count / 2, 0);

        std::vector<int> c = Random::Merge(a, b);

        REQUIRE(c.size() == count);
        size_t index = 0;
        for (const auto& item : c) {
            if (index < count / 2) {
                REQUIRE((item == 5 || item == 0));
            } else {
                REQUIRE(item == 5);
            }
            ++index;
        }
    }
}
