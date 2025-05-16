#include "pch.h"
#include "Matrix3.h"
#include "Quaternion.h"

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

Matrix3 Matrix3::identity() {
    return Matrix3(1, 0, 0,
        0, 1, 0,
        0, 0, 1);
}

void Matrix3::setIdentity() {
    *this = identity();
}

Matrix3 Matrix3::transpose() const {
    return Matrix3(
        data[0], data[3], data[6],
        data[1], data[4], data[7],
        data[2], data[5], data[8]
    );
}

Vector3 Matrix3::operator*(const Vector3& v) const {
    return Vector3(
        data[0] * v.x + data[1] * v.y + data[2] * v.z,
        data[3] * v.x + data[4] * v.y + data[5] * v.z,
        data[6] * v.x + data[7] * v.y + data[8] * v.z
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

Vector3 Matrix3::transform(const Vector3& vec) const {
    return (*this) * vec;
}
