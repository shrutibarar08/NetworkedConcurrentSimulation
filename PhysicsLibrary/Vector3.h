#pragma once
#include <cmath>

class Vector3
{
public:
	float x, y, z;

	Vector3();
	Vector3(float x, float y, float z);

	float getX() const { return x; }
	float getY() const { return y; }
	float getZ() const { return z; }

	void clear();
	float magnitude() const;
	float squaredMagnitude() const;
	void normalize();
	Vector3 normalized() const;

	float dot(const Vector3& other) const;
	Vector3 cross(const Vector3& other) const;

	//operator overloaders

	Vector3 operator+(const Vector3& other) const;
	Vector3 operator-(const Vector3& other) const;
	Vector3 operator*(float scalar) const;
	Vector3 operator/(float scalar) const;
	Vector3& operator+=(const Vector3& other);
	Vector3& operator-=(const Vector3& other);
	Vector3& operator*=(float scalar);
	Vector3& operator/=(float scalar);
	Vector3 operator-() const;

	bool isZero() const;

};

