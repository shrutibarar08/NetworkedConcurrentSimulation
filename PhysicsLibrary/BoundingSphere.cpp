#include "pch.h"
#include "BoundingSphere.h"
#include <cmath>

BoundingSphere::BoundingSphere() : center(Vector3()), radius(1.0f) {}

BoundingSphere::BoundingSphere(const Vector3& c, float r)
    : center(c), radius(r) {
}

bool BoundingSphere::overlaps(const BoundingSphere& other) const {
    float distSq = (center - other.center).squaredMagnitude();
    float radiusSum = radius + other.radius;
    return distSq < radiusSum * radiusSum;
}

float BoundingSphere::getGrowth(const BoundingSphere& other) const {
    BoundingSphere merged = BoundingSphere::merge(*this, other);
    return merged.radius * merged.radius - radius * radius;
}

BoundingSphere BoundingSphere::merge(const BoundingSphere& a, const BoundingSphere& b) {
    Vector3 centerOffset = b.center - a.center;
    float dist = centerOffset.magnitude();

    if (a.radius >= dist + b.radius) return a;
    if (b.radius >= dist + a.radius) return b;

    float newRadius = (dist + a.radius + b.radius) * 0.5f;
    Vector3 direction = centerOffset.normalized();
    Vector3 newCenter = a.center + direction * (newRadius - a.radius);

    return BoundingSphere(newCenter, newRadius);
}
