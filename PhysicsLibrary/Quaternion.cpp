#include "pch.h"
#include "Quaternion.h"
#include <cmath>

Quaternion::Quaternion() : r(1), i(0), j(0), k(0) {}
<<<<<<< Updated upstream
<<<<<<< Updated upstream
Quaternion::Quaternion(float r, float i, float j, float k)
    : r(r), i(i), j(j), k(k) {
}
=======
Quaternion::Quaternion(float r, float i, float j, float k) : r(r), i(i), j(j), k(k) {}
>>>>>>> Stashed changes
=======
Quaternion::Quaternion(float r, float i, float j, float k) : r(r), i(i), j(j), k(k) {}
>>>>>>> Stashed changes

void Quaternion::normalize() {
    float d = r * r + i * i + j * j + k * k;
    if (d == 0) {
        r = 1; i = j = k = 0;
        return;
    }
    float inv = 1.0f / std::sqrt(d);
    r *= inv; i *= inv; j *= inv; k *= inv;
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
    *this = (*this) * q;
}
Quaternion Quaternion::operator*(float scalar) const {
    return Quaternion(r * scalar, i * scalar, j * scalar, k * scalar);
}

Quaternion& Quaternion::operator+=(const Quaternion& q) {
    r += q.r;
    i += q.i;
    j += q.j;
    k += q.k;
    return *this;
}
Quaternion operator*(float scalar, const Quaternion& q) {
    return q * scalar;
}
