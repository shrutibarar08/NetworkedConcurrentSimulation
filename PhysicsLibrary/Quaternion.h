#pragma once
#include "Vector3.h"

class Quaternion {
public:
    float r, i, j, k;

    Quaternion();
    Quaternion(float r, float i, float j, float k);

    void normalize();

    Quaternion operator*(const Quaternion& q) const;

    void addScaledVector(const Vector3& vector, float scale);

    void rotateByVector(const Vector3& vector);
};

