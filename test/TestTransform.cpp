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
        Transform expected({ 37.7222, -127.8225, -12.8413, -45.092, -29.4749, 47.1248, -17.1506, -113.0926, 17.9947 });

        CompareValues(result, expected);
    }

    SECTION("Translation")
    {
        Transform initial = Transform::Translation(Point{ -4, 4});
        Transform expected = Transform::Translation(Point{ 0, 0});
        initial.Translate(4, -4);

        CompareValues(initial.GetValues(), expected.GetValues());
    }

    SECTION("RotateD")
    {
        Transform initial;
        REQUIRE(initial.GetRotationD() == 0.0);

        initial.RotateD(30.0);
        REQUIRE_THAT(initial.GetRotationD(), Catch::Matchers::WithinAbs(30.0, 0.0000000000001));
        initial.RotateD(-30.0);
        REQUIRE(initial.GetRotationD() == 0.0);

        for (int i = 0; i < 360; ++i) {
            Transform t;
            t.RotateD(i);
            REQUIRE_THAT(t.GetRotationD(), Catch::Matchers::WithinAbs(static_cast<double>(i), 0.0000000000001));
        }

        Transform t;
        double angle = (360.0 / 12.0);
        double accumulated = 0;
        for (int i = 0; i < 10; ++i) {
            accumulated += angle;
            t.RotateD(angle);
            REQUIRE_THAT(t.GetRotationD(), Catch::Matchers::WithinAbs(accumulated, 0.0000000000001));
        }
    }

    SECTION("RotateR")
    {
        Transform initial;
        REQUIRE(initial.GetRotationR() == 0.0);

        initial.RotateR(1);
        REQUIRE(initial.GetRotationR() == 1.0);
        initial.RotateR(-1);
        REQUIRE(initial.GetRotationR() == 0.0);

        constexpr double toRadians = std::numbers::pi / 180.0;
        for (int i = 0; i < 360; ++i) {
            Transform t;
            double rotationRadians = static_cast<double>(i) * toRadians;
            t.RotateR(rotationRadians);
            REQUIRE_THAT(t.GetRotationR(), Catch::Matchers::WithinAbs(rotationRadians, 0.0000000000001));
        }

        Transform t;
        double angle = (360.0 / 12.0) * toRadians;
        double accumulated = 0;
        for (int i = 0; i < 10; ++i) {
            accumulated += angle;
            t.RotateD(angle);
            REQUIRE_THAT(t.GetRotationD(), Catch::Matchers::WithinAbs(accumulated, 0.0000000000001));
        }
    }

    SECTION("Rotation around {0, 0}")
    {
        const Point pivot{ 0, 0 };
        // +ve angle rotation actually results in anti-clockwise rotation
        const Point startPoint{ 4, 4 };
        const Point expected1{ -4, 4 };
        const Point expected2{ -4, -4 };
        const Point expected3{ 4, -4 };

        Transform t;

        t.RotateD(90, pivot);
        Point actual1 = t.Mapped(startPoint);
        ComparePoints(actual1, expected1);

        t.RotateD(90, pivot);
        Point actual2 = t.Mapped(startPoint);
        ComparePoints(actual2, expected2);

        t.RotateD(90, pivot);
        Point actual3 = t.Mapped(startPoint);
        ComparePoints(actual3, expected3);

        t.RotateD(-90, pivot);
        Point actual4 = t.Mapped(startPoint);
        ComparePoints(actual4, expected2);

        t.RotateD(-90, pivot);
        Point actual5 = t.Mapped(startPoint);
        ComparePoints(actual5, expected1);
    }

    SECTION("Rotation around {x, y}")
    {
        const Point pivot{ 2, 2 };
        const Point startPoint{ 4, 4 };
        // +ve angle rotation actually results in anti-clockwise rotation
        const Point expected1{ 0, 4 };
        const Point expected2{ 0, 0 };
        const Point expected3{ 4, 0 };

        Transform t;

        t.RotateD(90, pivot);
        Point actual1 = t.Mapped(startPoint);
        ComparePoints(actual1, expected1);

        t.RotateD(90, pivot);
        Point actual2 = t.Mapped(startPoint);
        ComparePoints(actual2, expected2);

        t.RotateD(90, pivot);
        Point actual3 = t.Mapped(startPoint);
        ComparePoints(actual3, expected3);

        t.RotateD(-90, pivot);
        Point actual4 = t.Mapped(startPoint);
        ComparePoints(actual4, expected2);

        t.RotateD(-90, pivot);
        Point actual5 = t.Mapped(startPoint);
        ComparePoints(actual5, expected1);
    }

    SECTION("ReflectAboutX")
    {
        {
            Transform t;
            t.ReflectX();
            Point unflipped{ -4.43278, -47832.542 };
            Point expected{ -4.43278, 47832.542 };
            Point actual = t.Mapped(unflipped);
            ComparePoints(actual, expected);
        }

        {
            Transform t;
            t.ReflectX(-2);
            Point unflipped{ -40.3278, 7832.009542 };
            Point expected{ -40.3278, -7836.009542 };
            Point actual = t.Mapped(unflipped);
            ComparePoints(actual, expected);
        }
    }

    SECTION("ReflectAboutY")
    {
        {
            Transform t;
            t.ReflectY();
            Point unflipped{ -4.43278, -47832.542 };
            Point expected{ 4.43278, -47832.542 };
            Point actual = t.Mapped(unflipped);
            ComparePoints(actual, expected);
        }

        {
            Transform t;
            t.ReflectY(-2);
            Point unflipped{ -4.43278, 7832.009542 };
            Point expected{ 0.43278, 7832.009542 };
            Point actual = t.Mapped(unflipped);
            ComparePoints(actual, expected);
        }
    }

    SECTION("ShearX")
    {
        {
            Transform t;
            t.ShearX(1);
            ComparePoints(t.Mapped({ 0, 0 }), { 0, 0 });
            ComparePoints(t.Mapped({ 432.234, 0 }), { 432.234, 0 });
            ComparePoints(t.Mapped({ -32.24, 0 }), { -32.24, 0 });
            ComparePoints(t.Mapped({ 0, 1 }), { 1, 1 });
            ComparePoints(t.Mapped({ 0, -1 }), { -1, -1 });
        }
        {
            Transform t;
            t.ShearX(2.5);
            ComparePoints(t.Mapped({ 0, 0 }), { 0, 0 });
            ComparePoints(t.Mapped({ 432.234, 0 }), { 432.234, 0 });
            ComparePoints(t.Mapped({ -32.24, 0 }), { -32.24, 0 });
            ComparePoints(t.Mapped({ 0, 1 }), { 2.5, 1 });
            ComparePoints(t.Mapped({ 0, -1 }), { -2.5, -1 });
        }
    }

    SECTION("ShearY")
    {
        {
            Transform t;
            t.ShearY(1);
            ComparePoints(t.Mapped({ 0, 0 }), { 0, 0 });
            ComparePoints(t.Mapped({ 0, 432.234 }), { 0, 432.234 });
            ComparePoints(t.Mapped({ 0, -32.24 }), { 0, -32.24 });
            ComparePoints(t.Mapped({ 1, 0 }), { 1, 1 });
            ComparePoints(t.Mapped({ -1, 0 }), { -1, -1 });
        }
        {
            Transform t;
            t.ShearY(2.5);
            ComparePoints(t.Mapped({ 0, 0 }), { 0, 0 });
            ComparePoints(t.Mapped({ 0, 432.234 }), { 0, 432.234 });
            ComparePoints(t.Mapped({ 0, -32.24 }), { 0, -32.24 });
            ComparePoints(t.Mapped({ 1, 0 }), { 1, 2.5 });
            ComparePoints(t.Mapped({ -1, 0 }), { -1, -2.5 });
        }
    }
}
