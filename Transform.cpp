#include "Transform.h"

#include "MathConstants.h"

Transform::Transform()
    : Transform(Point{ 0, 0 })
{
}

Transform::Transform(const Transform& other)
    : position(other.position)
    , a1(other.a1)
    , a2(other.a2)
    , a3(position.x)
    , b1(other.b1)
    , b2(other.b2)
    , b3(position.y)
    , c1(other.c1)
    , c2(other.c2)
    , c3(other.c3)
{
}

Transform::Transform(const Point& location)
    : Transform({ 1, 0, location.x, 0, 1, location.y, 0, 0, 1 })
{
}

Transform::Transform(const std::array<double, 9>& values)
    : position{ values[2], values[5] }
    , a1(values[0])
    , a2(values[1])
    , a3(position.x)
    , b1(values[3])
    , b2(values[4])
    , b3(position.y)
    , c1(values[6])
    , c2(values[7])
    , c3(values[8])
{
}

void Transform::ConfigureJsonSerialisationHelper(Tril::JsonSerialisationHelper<Transform>& helper)
{
    helper.RegisterVariable("a1", &Transform::a1);
    helper.RegisterVariable("a2", &Transform::a2);
    helper.RegisterVariable("ab3", &Transform::position);

    helper.RegisterVariable("b1", &Transform::b1);
    helper.RegisterVariable("b2", &Transform::b2);

    helper.RegisterVariable("c1", &Transform::c1);
    helper.RegisterVariable("c2", &Transform::c2);
    helper.RegisterVariable("c3", &Transform::c3);
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
    *this = other * *this;
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

const Point& Transform::GetLocation() const
{
    return position;
}

const double& Transform::GetX() const
{
    return position.x;
}

const double& Transform::GetY() const
{
    return position.y;
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
    constexpr double toDegrees = 180.0 / std::numbers::pi;
    return GetRotationR() * toDegrees;
}

double Transform::GetRotationR() const
{
    assert(std::atan(-a2/a1) == std::atan(b1/b2));
    return std::atan(b1/b2);
}

void Transform::Map(Point& point) const
{
    // TODO this can be optimised greatly, there is no need to do a full matrix multiplication here
    Transform result = Transform(point) * *this;
    point = result.GetLocation();
}

const Point Transform::Mapped(const Point& point) const
{
    // TODO this can be optimised greatly, there is no need to do a full matrix multiplication here
    Transform result = Transform(point) * *this;
    return result.GetLocation();
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

void Transform::SetLocation(const Point& location)
{
    position = location;
}

void Transform::SetRotationD(double degrees)
{
    constexpr double toRadians = std::numbers::pi / 180.0;
    SetRotationR(degrees * toRadians);
}

void Transform::SetRotationR(double radians)
{
    // [  cos(theta), sin(theta), x ]
    // [ -sin(theta), cos(theta), y ]
    // [ 0          , 0         , 1 ]

    double sinTheta = std::sin(radians);
    double cosTheta = std::cos(radians);

    a1 = cosTheta;
    a2 = sinTheta;
    b1 = -sinTheta;
    b2 = cosTheta;
}

void Transform::RotateD(double degrees)
{
    constexpr double toRadians = std::numbers::pi / 180.0;
    RotateR(degrees * toRadians);
}

void Transform::RotateD(double degrees, const Point& pivot)
{
    Translate(-pivot.x, -pivot.y);
    RotateD(degrees);
    Translate(pivot.x, pivot.y);
}

void Transform::RotateR(double radians)
{
    // [  cos(theta), sin(theta), 0 ]
    // [ -sin(theta), cos(theta), 0 ]
    // [  0         , 0         , 1 ]

    double sinTheta = std::sin(radians);
    double cosTheta = std::cos(radians);

    // TODO this can be optimised greatly, there is no need to do a full matrix multiplication here
    Transform rotation({ cosTheta, sinTheta, 0, -sinTheta, cosTheta, 0, 0, 0, 1 });
    *this = rotation * *this;
}

void Transform::RotateR(double radians, const Point& pivot)
{
    double x = a3;
    double y = b3;

    a3 = 0;
    b3 = 0;

    // the translation before and after is somehow setting the x, y to zero
    // in the case of entities where the rotation is happening "in place", could perhaps fix it by setti9ng the rotation instead?
    // maybe Talk to Tris about this?

    RotateR(radians);

    a3 = x;
    b3 = y;
}

void Transform::Translate(double xDelta, double yDelta)
{
    // [ 1, 0, x ]
    // [ 0, 1, y ]
    // [ 0, 0, 1 ]

    // TODO this can be optimised greatly, there is no need to do a full matrix multiplication here
    Transform translation({ 1, 0, xDelta, 0, 1, yDelta, 0, 0, 1 });
    *this = translation * *this;
}

void Transform::Translate(const Point& delta)
{
    Translate(delta.x, delta.y);
}

void Transform::ReflectX(double axis)
{
    // [ 1,  0, 0 ]
    // [ 0, -1, 0 ]
    // [ 0,  0, 1 ]

    // TODO this can be optimised greatly, there is no need to do a full matrix multiplication here
    Transform translation({ 1, 0, 0, 0, -1, 0, 0, 0, 1 });
    *this = translation * *this;
}

void Transform::ReflectY(double axis)
{
    // [ -1, 0, 0 ]
    // [  0, 1, 0 ]
    // [  0, 0, 1 ]

    // TODO this can be optimised greatly, there is no need to do a full matrix multiplication here
    Transform translation({ -1, 0, 0, 0, 1, 0, 0, 0, 1 });
    *this = translation * *this;
}
