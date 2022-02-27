#include <Transform.h>

#include <FormatHelpers.h>
#include <Algorithm.h>

#include <catch2/catch.hpp>

using namespace util;

inline void ComparePoints(const Point& a, const Point& b)
{
    REQUIRE_THAT(a.x, Catch::Matchers::WithinAbs(b.x, 0.0000000000001));
    REQUIRE_THAT(a.y, Catch::Matchers::WithinAbs(b.y, 0.0000000000001));
}

inline void CompareValues(const Transform& a, const Transform& b)
{
    util::IterateBoth(a.GetValues(), b.GetValues(), [](const double& a, const double& b)
    {
        REQUIRE_THAT(a, Catch::Matchers::WithinAbs(b, 0.0000000000001));
    });
}

TEST_CASE("Transform", "[]")
{
    SECTION("Multiplication")
    {
        // [  9.37, 4.77, -2.21 ]
        // [ -2.01, 2.38, -7.20 ]
        // [  6.88, 2.27, -6.61 ]
        // x
        // [ 2.72, -9.81, -2.90 ]
        // [ 6.04, -5.15,  0.41 ]
        // [ 7.50,  5.13, -5.60 ]
        // =
        // [  37.7222, -127.8225, -12.8413 ]
        // [ -45.0920, -29.47489,  47.1248 ]
        // [ -17.1506, -113.0926,  17.9947 ]

         Transform a({ 9.37, 4.77, -2.21, -2.01, 2.38, -7.20, 6.88, 2.27, -6.61 });
         Transform b({ 2.72, -9.81, -2.90, 6.04, -5.15, 0.41, 7.50,  5.13, -5.60 });
         Transform result = a * b;
         Transform expected({ 37.7222, -127.82249999999999, -12.841299999999999, -45.092, -29.474899999999998, 47.1248, -17.150600000000004, -113.0926, 17.9947 });

         REQUIRE(result == expected);
    }

    SECTION("Translation")
    {
        Transform initial(Point{ -4, 4});
        Transform expected(Point{ 0, 0});
        initial.Translate(4, -4);

        CompareValues(initial.GetValues(), expected.GetValues());
    }

    SECTION("Rotation around {0, 0}")
    {
        const Point origin{ 0, 0 };
        Transform initial(Point{ 4, 4});
        Point expected1{ 4, -4};
        Point expected2{ -4, -4};
        Point expected3{ -4, 4};
        Point expected4 = initial.GetLocation();
        initial.RotateD(90, origin);
        ComparePoints(initial.GetLocation(), expected1);
        initial.RotateD(90, origin);
        ComparePoints(initial.GetLocation(), expected2);
        initial.RotateD(90, origin);
        ComparePoints(initial.GetLocation(), expected3);
        initial.RotateD(-90, origin);
        ComparePoints(initial.GetLocation(), expected2);
        initial.RotateD(-90, origin);
        ComparePoints(initial.GetLocation(), expected1);
    }

    SECTION("Rotation around {x, y}")
    {
        const Point origin{ 0, 0 };
        const Point pivot{ 2, 2 };
        Transform initial(Point{ 4, 4});
        Point expected1{ 4, 0};
        Point expected2{ 0, 0};
        Point expected3{ 0, 4};
        Point expected4(initial.GetLocation());
        initial.RotateD(90, pivot);
        ComparePoints(initial.GetLocation(), expected1);
        initial.RotateD(90, pivot);
        ComparePoints(initial.GetLocation(), expected2);
        initial.RotateD(90, pivot);
        ComparePoints(initial.GetLocation(), expected3);
        initial.RotateD(-90, pivot);
        ComparePoints(initial.GetLocation(), expected2);
        initial.RotateD(-90, pivot);
        ComparePoints(initial.GetLocation(), expected1);
    }
}
