#include "pch.h"
#include "Quaternion.h"
#include <cmath>

Quaternion::Quaternion() : r(1), i(0), j(0), k(0) {}
Quaternion::Quaternion(float r, float i, float j, float k)
    : r(r), i(i), j(j), k(k) {
}

void Quaternion::normalize() {
    float d = r * r + i * i + j * j + k * k;
    if (d == 0.0f) {
        r = 1.0f; i = j = k = 0.0f;
        return;
    }

    float scale = 1.0f / std::sqrt(d);
    r *= scale;
    i *= scale;
    j *= scale;
    k *= scale;
}

Quaternion Quaternion::operator*(const Quaternion& q) const {
    return Quaternion(
        r * q.r - i * q.i - j * q.j - k * q.k,
        r * q.i + i * q.r + j * q.k - k * q.j,
        r * q.j + j * q.r + k * q.i - i * q.k,
        r * q.k + k * q.r + i * q.j - j * q.i
    );
}

void Quaternion::addScaledVector(const Vector3& vector, float scale) {
    Quaternion q(0, vector.x * scale, vector.y * scale, vector.z * scale);
    q = q * (*this);

    r += q.r * 0.5f;
    i += q.i * 0.5f;
    j += q.j * 0.5f;
    k += q.k * 0.5f;
}

void Quaternion::rotateByVector(const Vector3& vector) {
    Quaternion q(0, vector.x, vector.y, vector.z);
    *this = q * (*this);
}
