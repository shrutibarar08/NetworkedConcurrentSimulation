#pragma once
#include "Vector3.h"

class Matrix3
{
public:
    float data[9]; // Row-major order

    Matrix3(); 
    Matrix3(float d0, float d1, float d2,
        float d3, float d4, float d5,
        float d6, float d7, float d8);

    // Multiply matrix by vector
    Vector3 operator*(const Vector3& v) const;

    // Multiply matrix by another matrix
    Matrix3 operator*(const Matrix3& m) const;

    // Set to identity
    void setIdentity();

    // Transpose
    Matrix3 transpose() const;

    // Inverse (only for symmetric inertia tensors)
    Matrix3 inverse() const;
};

