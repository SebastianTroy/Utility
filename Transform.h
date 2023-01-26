#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Shape.h"

#include <esd/ClassHelper.h>

#include <nlohmann/json.hpp>
#include <fmt/format.h>

/**
 * Cheatsheet: https://www.alanzucconi.com/2016/02/10/tranfsormation-matrix/
 *
 * The main point of this class was for me to learn 2D transformations using a
 * 3x3 matrix. It in no way represents best practice, or optimal performance.
 */
struct Transform {
public:
    Transform();
    Transform(const Transform& other);
    Transform(const std::array<double, 9>& values);

    static Transform Translation(const Point& location);
    static Transform Translation(const double& x, const double& y);
    static Transform RotationD(const double& d); // degrees
    static Transform RotationR(const double& r); // radians
    static Transform ReflectionX(const double& axis = 0);
    static Transform ReflectionY(const double& axis = 0);
    static Transform Reflection(const double& xAxis = 0, const double& yAxis = 0);
    static Transform Shear(const double& x, const double& y);

    [[nodiscard]] Transform operator*(const Transform& other) const;
    Transform& operator*=(const Transform& other);
    [[nodiscard]] bool operator==(const Transform& other) const;
    Transform& operator=(const Transform& other);

    [[nodiscard]] std::array<double, 9> GetValues() const;

    [[nodiscard]] Point GetTranslation() const;
    [[nodiscard]] const double& GetTranslationX() const;
    [[nodiscard]] const double& GetTranslationY() const;
    [[nodiscard]] double GetScaleX() const;
    [[nodiscard]] double GetScaleY() const;
    [[nodiscard]] double GetRotationD() const; // degrees
    [[nodiscard]] double GetRotationR() const; // radians

    // Transforms a point by *this
    void Map(Point& point) const;
    [[nodiscard]] Point Mapped(const Point& point) const;

    // Creates modified copy
    [[nodiscard]] Transform RotatedD(double degrees) const;
    [[nodiscard]] Transform RotatedD(double degrees, const Point& pivot) const;
    [[nodiscard]] Transform RotatedR(double radians) const;
    [[nodiscard]] Transform RotatedR(double radians, const Point& pivot) const;
    [[nodiscard]] Transform Translated(double xDelta, double yDelta) const;
    [[nodiscard]] Transform Translated(const Point& delta) const;
    [[nodiscard]] Transform ReflectedX(double axis = 0) const;
    [[nodiscard]] Transform ReflectedY(double axis = 0) const;

    Transform& RotateD(double degrees);
    Transform& RotateD(double degrees, const Point& pivot);
    Transform& RotateR(double radians);
    Transform& RotateR(double radians, const Point& pivot);

    Transform& Translate(double xDelta, double yDelta);
    Transform& Translate(const Point& delta);

    /// Reflect about line where x=0
    Transform& ReflectX();
    /// Reflect about line where x=axis
    Transform& ReflectX(double axis);
    /// Reflect about line where y=0
    Transform& ReflectY();
    /// Reflect about line where y=axis
    Transform& ReflectY(double axis);

    Transform& ShearX(double factor);
    Transform& ShearY(double factor);

private:
    /*
     * { { a1, a2, a3 },
     *   { b1, b2, b3 },
     *   { c1, c2, c3 } }
     */
    double a1;
    double a2;
    double a3;
    double b1;
    double b2;
    double b3;
    double c1;
    double c2;
    double c3;
};

template<>
struct fmt::formatter<Transform> : fmt::formatter<double>
{
    template <typename FormatContext>
    auto format(const Transform& transform, FormatContext& context)
    {
        auto&& out= context.out();
        auto values = transform.GetValues();
        format_to(out, "{ ");
        fmt::formatter<double>::format(values[0], context);
        format_to(out, ", ");
        fmt::formatter<double>::format(values[1], context);
        format_to(out, ", ");
        fmt::formatter<double>::format(values[2], context);
        format_to(out, " }, ");

        format_to(out, "{ ");
        fmt::formatter<double>::format(values[3], context);
        format_to(out, ", ");
        fmt::formatter<double>::format(values[4], context);
        format_to(out, ", ");
        fmt::formatter<double>::format(values[5], context);
        format_to(out, " }, ");

        format_to(out, "{ ");
        fmt::formatter<double>::format(values[6], context);
        format_to(out, ", ");
        fmt::formatter<double>::format(values[7], context);
        format_to(out, ", ");
        fmt::formatter<double>::format(values[8], context);
        return format_to(out, " }");
    }
};

template<>
class esd::Serialiser<Transform> : public esd::ClassHelper<Transform, std::array<double, 9>> {
public:
    static void Configure()
    {
        SetConstruction(
            CreateParameter(&Transform::GetValues, "values")
        );
    }
};

#endif // TRANSFORM_H
