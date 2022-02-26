#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Shape.h"
#include "JsonSerialisationHelper.h"

#include <nlohmann/json.hpp>
#include <fmt/format.h>

/**
 * Cheatsheet: https://www.alanzucconi.com/2016/02/10/tranfsormation-matrix/
 */
struct Transform {
public:
    Transform();
    Transform(const Transform& other);
    Transform(const Point& location);
    Transform(const std::array<double, 9>& values);

    static void ConfigureJsonSerialisationHelper(Tril::JsonSerialisationHelper<Transform>& helper);

    [[nodiscard]] Transform operator*(const Transform& other) const;
    Transform& operator*=(const Transform& other);
    [[nodiscard]] bool operator==(const Transform& other) const;
    Transform& operator=(const Transform& other);

    [[nodiscard]] std::array<double, 9> GetValues() const;

    [[nodiscard]] const Point& GetLocation() const;
    [[nodiscard]] const double& GetX() const;
    [[nodiscard]] const double& GetY() const;
    [[nodiscard]] const double GetScaleX() const;
    [[nodiscard]] const double GetScaleY() const;
    [[nodiscard]] const double GetRotationD() const; // degrees
    [[nodiscard]] const double GetRotationR() const; // radians

    // Transforms a point by *this
    const void Map(Point& point) const;
    [[nodiscard]] const Point Mapped(const Point& point) const;

    // Creates modified copy
    [[nodiscard]] Transform RotatedD(double degrees) const;
    [[nodiscard]] Transform RotatedD(double degrees, const Point& pivot) const;
    [[nodiscard]] Transform RotatedR(double radians) const;
    [[nodiscard]] Transform RotatedR(double radians, const Point& pivot) const;
    [[nodiscard]] Transform Translated(double xDelta, double yDelta) const;
    [[nodiscard]] Transform Translated(const Point& delta) const;
    [[nodiscard]] Transform ReflectedX(double axis = 0) const;
    [[nodiscard]] Transform ReflectedY(double axis = 0) const;

    // Adjusts *this
    void SetLocation(const Point& location);
    void SetRotationD(double degrees);
    void SetRotationR(double radians);
    void RotateD(double degrees);
    void RotateD(double degrees, const Point& pivot);
    void RotateR(double radians);
    void RotateR(double radians, const Point& pivot);
    void Translate(double xDelta, double yDelta);
    void Translate(const Point& delta);
    void ReflectX(double axis = 0);
    void ReflectY(double axis = 0);

private:
    Point position;

    /*
     * { { a1, a2, a3 },
     *   { b1, b2, b3 },
     *   { c1, c2, c3 } }
     */
    double a1;
    double a2;
    double& a3;
    double b1;
    double b2;
    double& b3;
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


#endif // TRANSFORM_H
