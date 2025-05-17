#pragma once
#include "Vector3.h"

class Quaternion {
public:
    float r, i, j, k;
  
    Quaternion();
    Quaternion(float r, float i, float j, float k);

    Quaternion operator*(const Quaternion& q) const;
    Quaternion operator*(float scalar) const;
    Quaternion& operator+=(const Quaternion& q);

    void addScaledVector(const Vector3& vector, float scale);
    void rotateByVector(const Vector3& vector);
    void normalize();

    friend Quaternion operator*(float scalar, const Quaternion& q);
};
