#include <Algorithm.h>

#include <catch2/catch.hpp>

#include <map>
#include <vector>
#include <deque>

TEST_CASE("Algorithm", "[algorithm]")
{
    SECTION("IterateBoth same value type, same length")
    {
        std::vector<int> a{ 1, 2, 3, 4, 5 };
        std::vector<int> b = a;

        // can const iterate non-const containers
        util::IterateBoth(a, b, [](const int& a, const int& b)
        {
            REQUIRE(a == b);
        });

        util::IterateBoth(a, b, [](const int& a, int& b)
        {
            REQUIRE(a == b);
        });

        util::IterateBoth(a, b, [](int& a, const int& b)
        {
            REQUIRE(a == b);
        });

        util::IterateBoth(a, b, [](int& a, int& b)
        {
            ++a;
            REQUIRE(a != b);
        });

        // ensure a has been modified
        util::IterateBoth(a, b, [](const int& a, const int& b)
        {
            REQUIRE(a != b);
        });
    }

    SECTION("IterateBoth different value type, same length")
    {
        std::vector<int> a{ 1, 2, 3, 4, 5 };
        const std::vector<double> b = { 1, 2, 3, 4, 5 };

        util::IterateBoth(a, b, [](const int& a, const double& b)
        {
            REQUIRE(a == static_cast<int>(b));
        });
    }

    SECTION("IterateBoth different container type, same length")
    {
        std::vector<int> a{ 1, 2, 3, 4, 5 };
        const std::map<int, double> b = { {1, 1.5}, {2, 2.5}, {3, 3.5}, {4, 4.5}, {5, 5.5} };

        util::IterateBoth(a, b, [](const int& a, const std::pair<int, double>& b)
        {
            REQUIRE(a == b.first);
        });
    }

    SECTION("IterateBoth different container type, different lengths")
    {
        std::vector<int> a{ 1, 2, 3, 4 };
        const std::map<int, double> b = { {1, 1.5}, {2, 2.5}, {3, 3.5}, {4, 4.5}, {5, 5.5} };

        util::IterateBoth(a, b, [](const int& a, const std::pair<int, double>& b)
        {
            REQUIRE(a == b.first);
        });

        a.push_back(5);
        a.push_back(6);

        util::IterateBoth(a, b, [](const int& a, const std::pair<int, double>& b)
        {
            REQUIRE(a == b.first);
        });
    }

    SECTION("Compare Containers")
    {
        std::vector<std::pair<int, double>> a{ {1, 1.5}, {2, 2.5}, {3, 3.5}, {4, 4.5}, {5, 5.5} };
        std::deque<std::pair<int, double>> b = { {1, 1.5}, {2, 2.5}, {3, 3.5}, {4, 4.5}, {5, 5.5} };

        REQUIRE(util::CompareContainers(a, b));
        a.front().first = 2;
        REQUIRE(!util::CompareContainers(a, b));
        a.front().first = 1;
        REQUIRE(util::CompareContainers(a, b));
        a.push_back(std::make_pair(6, 6.5));
        REQUIRE(!util::CompareContainers(a, b));
    }

    SECTION("CreateSeries")
    {
        std::vector<int> a = util::CreateSeries(-5, 11);
        std::vector<int> b(11);
        std::iota(std::begin(b), std::end(b), -5);

        REQUIRE(util::CompareContainers(a, b));
        a = util::CreateSeries(-5, 5, 11);
        REQUIRE(util::CompareContainers(a, b));
    }

    SECTION("Combine")
    {
        std::vector<int> a = util::CreateSeries(1, 5);
        std::vector<int> b = util::Combine<int>({1, 2, 3}, {4, 5});
        REQUIRE(util::CompareContainers(a, b));
    }
}
