#pragma once
#include "DirectXMath.h"
#include <atomic>

class Quaternion
{
public:
    Quaternion();
    Quaternion(float r, float i, float j, float k);

    Quaternion operator*(const Quaternion& q) const;
    Quaternion operator*(float scalar) const;
    Quaternion& operator+=(const Quaternion& q);
    Quaternion& operator=(const Quaternion& other);
    Quaternion(const Quaternion& other);

    DirectX::XMMATRIX ToRotationMatrix() const;

    void AddScaledVector(const DirectX::XMVECTOR& vector, float scale);
    void RotateByVector(const DirectX::XMVECTOR& vector);
    void Normalize();
    DirectX::XMVECTOR ToXmVector() const;
    DirectX::XMVECTOR RotateVector(const DirectX::XMVECTOR& v) const;

    friend Quaternion operator*(float scalar, const Quaternion& q);

    float GetR();
    float GetI();
    float GetJ();
    float GetK();

private:
    float R, I, J, K;
};