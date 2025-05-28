#include "pch.h"
#include "Quaternion.h"
#include <cmath>

Quaternion::Quaternion() : R(1), I(0), J(0), K(0) {}
Quaternion::Quaternion(float r, float i, float j, float k)
: R(r), I(i), J(j), K(k)
{}

DirectX::XMVECTOR Quaternion::ToXmVector() const
{
    return DirectX::XMVectorSet(I, J, K, R);
}

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

DirectX::XMVECTOR Quaternion::RotateVector(const DirectX::XMVECTOR& v) const
{
    using namespace DirectX;

    // Convert quaternion to XMVECTOR
    XMVECTOR q = XMVectorSet(I, J, K, R);
    XMVECTOR qInv = XMQuaternionInverse(q);
    XMVECTOR vec = XMVectorSetW(v, 0.0f); // Ensure it's a vector (not point)

    // Rotate: v' = q * v * q^-1
    XMVECTOR result = XMQuaternionMultiply(
        XMQuaternionMultiply(q, vec),
        qInv
    );

    return result;
}

float Quaternion::GetR()
{
    return R;
}

float Quaternion::GetI()
{
    return I;
}

float Quaternion::GetJ()
{
    return J;
}

float Quaternion::GetK()
{
    return K;
}

Quaternion Quaternion::operator*(const Quaternion& q) const
{
    DirectX::XMVECTOR q1 = ToXmVector();
    DirectX::XMVECTOR q2 = q.ToXmVector();

    DirectX::XMVECTOR result = DirectX::XMQuaternionMultiply(q1, q2);

    DirectX::XMFLOAT4 f;
    DirectX::XMStoreFloat4(&f, result);
    return Quaternion(f.w, f.x, f.y, f.z);
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

Quaternion& Quaternion::operator=(const Quaternion& other)
{
    if (this == &other) return *this;

    R = other.R;
    I = other.I;
    J = other.J;
    K = other.K;

    return *this;
}

Quaternion::Quaternion(const Quaternion& other)
{
    R = other.R;
    I = other.I;
    J = other.J;
    K = other.K;
}

DirectX::XMMATRIX Quaternion::ToRotationMatrix() const
{
    using namespace DirectX;
    return XMMatrixRotationQuaternion(ToXmVector());
}

Quaternion operator*(float scalar, const Quaternion& q)
{
    return q * scalar;
}