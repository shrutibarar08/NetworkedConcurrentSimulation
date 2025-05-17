#pragma once
#include "Quaternion.h"
#include "Vector3.h"

class Matrix3
{
public:
    float data[9]; // Row-major order

    Matrix3(); 
    Matrix3(float d0, float d1, float d2,
        float d3, float d4, float d5,
        float d6, float d7, float d8);
  
    static Matrix3 rotationMatrix(const Quaternion& q);
    static Matrix3 identity();

    void setIdentity();
    Matrix3 transpose() const;
    Matrix3 operator*(const Matrix3& other) const;
    Matrix3 inverse() const;

    Vector3 operator*(const Vector3& vec) const;
    Vector3 transform(const Vector3& vec) const;
};