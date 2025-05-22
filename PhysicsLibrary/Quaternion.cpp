#include "pch.h"
#include "Quaternion.h"
#include <cmath>

Quaternion::Quaternion() : R(1), I(0), J(0), K(0) {}
Quaternion::Quaternion(float r, float i, float j, float k)
: R(r), I(i), J(j), K(k)
{}

void Quaternion::Normalize()
{
    float d = R * R + I * I + J * J + K * K;
    if (d == 0.f)
    {
        R = 1; I = J = K = 0;
        return;
    }
    float inv = 1.0f / std::sqrt(d);
    R *= inv; I *= inv; J *= inv; K *= inv;
}

DirectX::XMVECTOR Quaternion::ToXmVector() const
{
    return DirectX::XMVectorSet(I, J, K, R);
}

Quaternion Quaternion::operator*(const Quaternion& q) const
{
    return Quaternion(
        R * q.R - I * q.I - J * q.J - K * q.K,
        R * q.I + I * q.R + J * q.K - K * q.J,
        R * q.J + J * q.R + K * q.I - I * q.K,
        R * q.K + K * q.R + I * q.J - J * q.I
    );
}

void Quaternion::AddScaledVector(const DirectX::XMVECTOR& vector, float scale)
{
    DirectX::XMVECTOR scaled = DirectX::XMVectorScale(vector, scale);

    float x = DirectX::XMVectorGetX(scaled);
    float y = DirectX::XMVectorGetY(scaled);
    float z = DirectX::XMVectorGetZ(scaled);

    Quaternion q(0, x, y, z);
    q = q * (*this);

    R += q.R * 0.5f;
    I += q.I * 0.5f;
    J += q.J * 0.5f;
    K += q.K * 0.5f;
}

void Quaternion::RotateByVector(const DirectX::XMVECTOR& vector)
{
    float x = DirectX::XMVectorGetX(vector);
    float y = DirectX::XMVectorGetY(vector);
    float z = DirectX::XMVectorGetZ(vector);

    Quaternion q(0, x, y, z);
    *this = (*this) * q;
}

Quaternion Quaternion::operator*(float scalar) const
{
    return Quaternion(R * scalar, I * scalar, J * scalar, K * scalar);
}

Quaternion& Quaternion::operator+=(const Quaternion& q)
{
    R += q.R;
    I += q.I;
    J += q.J;
    K += q.K;
    return *this;
}
Quaternion operator*(float scalar, const Quaternion& q)
{
    return q * scalar;
}
