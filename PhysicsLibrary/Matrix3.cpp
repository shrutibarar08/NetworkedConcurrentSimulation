#include "pch.h"
#include "Matrix3.h"
#include <stdexcept>

Matrix3::Matrix3()
{
	setIdentity();
}

Matrix3::Matrix3(float d0, float d1, float d2, 
	             float d3, float d4, float d5, 
	             float d6, float d7, float d8)
{
	data[0] = d0; data[1] = d1; data[2] = d2;
	data[3] = d3; data[4] = d4; data[5] = d5;
	data[6] = d6; data[7] = d7; data[8] = d8;
}

void Matrix3::setIdentity()
{
	data[0] = 1; data[1] = 0; data[2] = 0;
	data[3] = 0; data[4] = 1; data[5] = 0;
	data[6] = 0; data[7] = 0; data[8] = 1;
}

Vector3 Matrix3::operator*(const Vector3& v) const
{
	return Vector3(data[0] * v.x + data[1] * v.y + data[2] * v.z,
		           data[3] * v.x + data[4] * v.y + data[5] * v.z,
		           data[6] * v.x + data[7] * v.y + data[8] * v.z);
}

Matrix3 Matrix3::operator*(const Matrix3& m) const
{
	Matrix3 result;
	for (int row = 0; row < 3; ++row)
		for (int col = 0; col < 3; ++col)
			result.data[row * 3 + col] =
			data[row * 3 + 0] * m.data[0 * 3 + col] +
			data[row * 3 + 1] * m.data[1 * 3 + col] +
			data[row * 3 + 2] * m.data[2 * 3 + col];
	return result;
}


Matrix3 Matrix3::transpose() const
{
	return Matrix3(
		data[0], data[3], data[6],
		data[1], data[4], data[7],
		data[2], data[5], data[8]
	);
}

Matrix3 Matrix3::inverse() const
{
	float t0 = data[4] * data[8] - data[5] * data[7];
	float t1 = data[2] * data[7] - data[1] * data[8];
	float t2 = data[1] * data[5] - data[2] * data[4];

	float det = data[0] * t0 + data[3] * t1 + data[6] * t2;

	if (det == 0.0f)
		throw std::runtime_error("Matrix not invertible");

	float invDet = 1.0f / det;

	Matrix3 result;
	result.data[0] = t0 * invDet;
	result.data[1] = t1 * invDet;
	result.data[2] = t2 * invDet;

	result.data[3] = (data[5] * data[6] - data[3] * data[8]) * invDet;
	result.data[4] = (data[0] * data[8] - data[2] * data[6]) * invDet;
	result.data[5] = (data[2] * data[3] - data[0] * data[5]) * invDet;

	result.data[6] = (data[3] * data[7] - data[4] * data[6]) * invDet;
	result.data[7] = (data[1] * data[6] - data[0] * data[7]) * invDet;
	result.data[8] = (data[0] * data[4] - data[1] * data[3]) * invDet;

	return result;
}
