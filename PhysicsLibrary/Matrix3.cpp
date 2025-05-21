#include "pch.h"
#include "Matrix3.h"
#include <cmath>

Matrix3::Matrix3() {
    setIdentity();
}

Matrix3::Matrix3(float d0, float d1, float d2,
    float d3, float d4, float d5,
    float d6, float d7, float d8) {
    data[0] = d0; data[1] = d1; data[2] = d2;
    data[3] = d3; data[4] = d4; data[5] = d5;
    data[6] = d6; data[7] = d7; data[8] = d8;
}

void Matrix3::setIdentity() {
    data[0] = 1.0f; data[1] = 0.0f; data[2] = 0.0f;
    data[3] = 0.0f; data[4] = 1.0f; data[5] = 0.0f;
    data[6] = 0.0f; data[7] = 0.0f; data[8] = 1.0f;
}

Matrix3 Matrix3::identity() {
    return Matrix3(
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    );
}

Matrix3 Matrix3::rotationMatrix(const Quaternion& q) {
    float r = q.r, i = q.i, j = q.j, k = q.k;

    return Matrix3(
        1 - 2 * j * j - 2 * k * k, 2 * i * j - 2 * r * k, 2 * i * k + 2 * r * j,
        2 * i * j + 2 * r * k, 1 - 2 * i * i - 2 * k * k, 2 * j * k - 2 * r * i,
        2 * i * k - 2 * r * j, 2 * j * k + 2 * r * i, 1 - 2 * i * i - 2 * j * j
    );
}

Matrix3 Matrix3::transpose() const {
    return Matrix3(
        data[0], data[3], data[6],
        data[1], data[4], data[7],
        data[2], data[5], data[8]
    );
}

Matrix3 Matrix3::operator*(const Matrix3& other) const {
    Matrix3 result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            result.data[row * 3 + col] =
                data[row * 3 + 0] * other.data[0 * 3 + col] +
                data[row * 3 + 1] * other.data[1 * 3 + col] +
                data[row * 3 + 2] * other.data[2 * 3 + col];
        }
    }
    return result;
}

Matrix3 Matrix3::inverse() const {
    float det = data[0] * (data[4] * data[8] - data[5] * data[7]) -
        data[1] * (data[3] * data[8] - data[5] * data[6]) +
        data[2] * (data[3] * data[7] - data[4] * data[6]);

    if (std::abs(det) < 1e-9f) return Matrix3(); // Fallback identity

    float invDet = 1.0f / det;

    Matrix3 result;
    result.data[0] = (data[4] * data[8] - data[5] * data[7]) * invDet;
    result.data[1] = -(data[1] * data[8] - data[2] * data[7]) * invDet;
    result.data[2] = (data[1] * data[5] - data[2] * data[4]) * invDet;
    result.data[3] = -(data[3] * data[8] - data[5] * data[6]) * invDet;
    result.data[4] = (data[0] * data[8] - data[2] * data[6]) * invDet;
    result.data[5] = -(data[0] * data[5] - data[2] * data[3]) * invDet;
    result.data[6] = (data[3] * data[7] - data[4] * data[6]) * invDet;
    result.data[7] = -(data[0] * data[7] - data[1] * data[6]) * invDet;
    result.data[8] = (data[0] * data[4] - data[1] * data[3]) * invDet;

    return result;
}

Vector3 Matrix3::operator*(const Vector3& vec) const {
    return Vector3(
        data[0] * vec.x + data[1] * vec.y + data[2] * vec.z,
        data[3] * vec.x + data[4] * vec.y + data[5] * vec.z,
        data[6] * vec.x + data[7] * vec.y + data[8] * vec.z
    );
}

Vector3 Matrix3::transform(const Vector3& vec) const {
    return (*this) * vec;
}
