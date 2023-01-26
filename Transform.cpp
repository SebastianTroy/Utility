#include "Transform.h"

#include "MathConstants.h"

Transform::Transform()
    : Transform({ 1, 0, 0, 0, 1, 0, 0, 0, 1 })
{
}

Transform::Transform(const Transform& other)
    : a1(other.a1)
    , a2(other.a2)
    , a3(other.a3)
    , b1(other.b1)
    , b2(other.b2)
    , b3(other.b3)
    , c1(other.c1)
    , c2(other.c2)
    , c3(other.c3)
{
}

Transform::Transform(const std::array<double, 9>& values)
    : a1(values[0])
    , a2(values[1])
    , a3(values[2])
    , b1(values[3])
    , b2(values[4])
    , b3(values[5])
    , c1(values[6])
    , c2(values[7])
    , c3(values[8])
{
}

Transform Transform::Translation(const Point& location)
{
    return Transform{}.Translated(location.x, location.y);
}

Transform Transform::Translation(const double& x, const double& y)
{
    return Transform{}.Translated(x, y);
}

Transform Transform::RotationD(const double& d)
{
    return Transform{}.RotateD(d);
}

Transform Transform::RotationR(const double& r)
{
    return Transform{}.RotateR(r);
}

Transform Transform::ReflectionX(const double& axis)
{
    return Transform{}.ReflectX(axis);
}

Transform Transform::ReflectionY(const double& axis)
{
    return Transform{}.ReflectY(axis);
}

Transform Transform::Reflection(const double& xAxis, const double& yAxis)
{
    return Transform{}.ReflectX(xAxis).ReflectY(yAxis);
}

Transform Transform::Shear(const double& x, const double& y)
{
    return Transform{}.ShearX(x).ShearY(y);
}

Transform Transform::operator*(const Transform& other) const
{
    Transform result {};
    result.a1 = (a1 * other.a1) + (a2 * other.b1) + (a3 * other.c1);
    result.a2 = (a1 * other.a2) + (a2 * other.b2) + (a3 * other.c2);
    result.a3 = (a1 * other.a3) + (a2 * other.b3) + (a3 * other.c3);

    result.b1 = (b1 * other.a1) + (b2 * other.b1) + (b3 * other.c1);
    result.b2 = (b1 * other.a2) + (b2 * other.b2) + (b3 * other.c2);
    result.b3 = (b1 * other.a3) + (b2 * other.b3) + (b3 * other.c3);

    result.c1 = (c1 * other.a1) + (c2 * other.b1) + (c3 * other.c1);
    result.c2 = (c1 * other.a2) + (c2 * other.b2) + (c3 * other.c2);
    result.c3 = (c1 * other.a3) + (c2 * other.b3) + (c3 * other.c3);
    return result;
}

Transform& Transform::operator*=(const Transform& other)
{
    *this = *this * other;
    return *this;
}

bool Transform::operator==(const Transform& other) const
{
    return a1 == other.a1
        && a2 == other.a2
        && a3 == other.a3
        && b1 == other.b1
        && b2 == other.b2
        && b3 == other.b3
        && c1 == other.c1
        && c2 == other.c2
        && c3 == other.c3;
}

Transform& Transform::operator=(const Transform& other)
{
    a1 = other.a1;
    a2 = other.a2;
    a3 = other.a3;
    b1 = other.b1;
    b2 = other.b2;
    b3 = other.b3;
    c1 = other.c1;
    c2 = other.c2;
    c3 = other.c3;
    return *this;
}

std::array<double, 9> Transform::GetValues() const
{
    return { a1, a2, a3, b1, b2, b3, c1, c2, c3 };
}

Point Transform::GetTranslation() const
{
    return { a3, b3 };
}

const double& Transform::GetTranslationX() const
{
    return a3;
}

const double& Transform::GetTranslationY() const
{
    return b3;
}

double Transform::GetScaleX() const
{
    return std::sqrt(std::pow(a1, 2.) + std::pow(b1, 2.));
}

double Transform::GetScaleY() const
{
    return std::sqrt(std::pow(a2, 2.) + std::pow(b2, 2.));
}

double Transform::GetRotationD() const
{
    return util::ToDegrees(GetRotationR());
}

double Transform::GetRotationR() const
{
    assert(std::atan2(-a2, a1) == std::atan2(b1, b2));
    return std::fmod(util::Tau + std::atan2(b1, b2), util::Tau);
}

void Transform::Map(Point& point) const
{
    Transform result = *this * Transform::Translation(point);
    point = result.GetTranslation();
}

Point Transform::Mapped(const Point& point) const
{
    Transform result = *this * Transform::Translation(point);
    return result.GetTranslation();
}

Transform Transform::RotatedD(double degrees) const
{
    Transform copy(*this);
    copy.RotateD(degrees);
    return copy;
}

Transform Transform::RotatedD(double degrees, const Point& pivot) const
{
    Transform copy(*this);
    copy.RotateD(degrees, pivot);
    return copy;
}

Transform Transform::RotatedR(double radians) const
{
    Transform copy(*this);
    copy.RotateR(radians);
    return copy;
}

Transform Transform::RotatedR(double radians, const Point& pivot) const
{
    Transform copy(*this);
    copy.RotateR(radians, pivot);
    return copy;
}

Transform Transform::Translated(double xDelta, double yDelta) const
{
    Transform copy(*this);
    copy.Translate(xDelta, yDelta);
    return copy;
}

Transform Transform::Translated(const Point& delta) const
{
    Transform copy(*this);
    copy.Translate(delta);
    return copy;
}

Transform Transform::ReflectedX(double axis) const
{
    Transform copy(*this);
    copy.ReflectX(axis);
    return copy;
}

Transform Transform::ReflectedY(double axis) const
{
    Transform copy(*this);
    copy.ReflectY(axis);
    return copy;
}

Transform& Transform::RotateD(double degrees)
{
    RotateR(util::ToRadians(degrees));
    return *this;
}

Transform& Transform::RotateD(double degrees, const Point& pivot)
{
    RotateR(util::ToRadians(degrees), pivot);
    return *this;
}

Transform& Transform::RotateR(double radians)
{
    // [ cos(theta), -sin(theta), 0 ]
    // [ sin(theta),  cos(theta), 0 ]
    // [  0         , 0         , 1 ]

    double sinTheta = std::sin(radians);
    double cosTheta = std::cos(radians);

    Transform rotation({ cosTheta, -sinTheta, 0, sinTheta, cosTheta, 0, 0, 0, 1 });
    *this = rotation * *this;
    return *this;
}

Transform& Transform::RotateR(double radians, const Point& pivot)
{
    Translate(-pivot);
    RotateR(radians);
    Translate(pivot);
    return *this;
}

Transform& Transform::Translate(double xDelta, double yDelta)
{
    // [ 1, 0, x ]
    // [ 0, 1, y ]
    // [ 0, 0, 1 ]

    Transform translation({ 1, 0, xDelta, 0, 1, yDelta, 0, 0, 1 });
    *this = translation * *this;
    return *this;
}

Transform& Transform::Translate(const Point& delta)
{
    Translate(delta.x, delta.y);
    return *this;
}

Transform& Transform::ReflectX()
{
    // [ 1,  0, 0 ]
    // [ 0, -1, 0 ]
    // [ 0,  0, 1 ]

    Transform reflection({ 1, 0, 0, 0, -1, 0, 0, 0, 1 });
    *this = reflection * *this;
    return *this;
}

Transform& Transform::ReflectX(double axis)
{
    Translate(0, -axis);
    ReflectX();
    Translate(0, axis);
    return *this;
}

Transform& Transform::ReflectY()
{
    // [ -1, 0, 0 ]
    // [  0, 1, 0 ]
    // [  0, 0, 1 ]

    Transform reflection({ -1, 0, 0, 0, 1, 0, 0, 0, 1 });
    *this = reflection * *this;
    return *this;
}

Transform& Transform::ReflectY(double axis)
{
    Translate(-axis, 0);
    ReflectY();
    Translate(axis, 0);
    return *this;
}

Transform& Transform::ShearX(double factor)
{
    // [ 1, F, 0 ]
    // [ 0, 1, 0 ]
    // [ 0, 0, 1 ]

    Transform shear({ 1, factor, 0, 0, 1, 0, 0, 0, 1 });
    *this = shear * *this;
    return *this;
}

Transform& Transform::ShearY(double factor)
{
    // [ 1, 0, 0 ]
    // [ F, 1, 0 ]
    // [ 0, 0, 1 ]

    Transform shear({ 1, 0, 0, factor, 1, 0, 0, 0, 1 });
    *this = shear * *this;
    return *this;
}
