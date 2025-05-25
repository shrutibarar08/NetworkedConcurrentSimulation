#pragma once
#include "DirectXMath.h"

class Quaternion
{
public:
    Quaternion();
    Quaternion(float r, float i, float j, float k);

    Quaternion operator*(const Quaternion& q) const;
    Quaternion operator*(float scalar) const;
    Quaternion& operator+=(const Quaternion& q);

    DirectX::XMMATRIX ToRotationMatrix() const;

    void AddScaledVector(const DirectX::XMVECTOR& vector, float scale);
    void RotateByVector(const DirectX::XMVECTOR& vector);
    void Normalize();
    DirectX::XMVECTOR ToXmVector() const;

    friend Quaternion operator*(float scalar, const Quaternion& q);

    float R, I, J, K;
};
