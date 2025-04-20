#include "pch.h"
#include "Vector3.h"

Vector3::Vector3() : x(0), y(0), z(0) {}

Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

void Vector3::clear() {
    x = y = z = 0.0f;
}

float Vector3::magnitude() const
{
    return std::sqrt(x*x + y*y + z*z);
}

float Vector3::squaredMagnitude() const
{
    return x * x + y * y + z * z;
}

void Vector3::normalize()
{
	float mag = magnitude();
	if (mag > 0.0f) {
		x /= mag;
		y /= mag;
		z /= mag;
	}
}

Vector3 Vector3::normalized() const
{
	Vector3 result = *this;
    result.normalize();
    return result;
}

float Vector3::dot(const Vector3& other) const
{
    return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::cross(const Vector3& other) const
{
    return Vector3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

//operators
Vector3 Vector3::operator+(const Vector3& other) const
{
    return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3 Vector3::operator-(const Vector3& other) const
{
    return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3 Vector3::operator*(float scalar) const
{
    return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3 Vector3::operator/(float scalar) const
{
    return Vector3(x / scalar, y / scalar, z / scalar);
}

Vector3& Vector3::operator+=(const Vector3& other)
{
    x += other.x; y += other.y; z += other.z;
    return *this;
}

Vector3& Vector3::operator-=(const Vector3& other)
{
    x -= other.x; y -= other.y; z -= other.z;
    return *this;
}

Vector3& Vector3::operator*=(float scalar)
{
    x *= scalar; y *= scalar; z *= scalar;
    return *this;
}

Vector3& Vector3::operator/=(float scalar)
{
    x /= scalar; y /= scalar; z /= scalar;
    return *this;
}

Vector3 Vector3::operator-() const {
    return Vector3(-x, -y, -z);
}

bool Vector3::isZero() const
{
    return x == 0 && y == 0 && z == 0;
}
