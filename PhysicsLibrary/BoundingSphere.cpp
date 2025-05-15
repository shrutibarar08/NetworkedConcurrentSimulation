#include "pch.h"
#include "BoundingSphere.h"
#include <cmath>

BoundingSphere::BoundingSphere() : center(), radius(1.0f) {}

BoundingSphere::BoundingSphere(const Vector3& c, float r) : center(c), radius(r) {}

bool BoundingSphere::overlaps(const BoundingSphere& other) const
{
	float distSquared = (center - other.center).squaredMagnitude();
	float radiusSum = radius + other.radius;
	return distSquared < (radiusSum * radiusSum);
}

BoundingSphere BoundingSphere::merge(const BoundingSphere& a, const BoundingSphere& b)
{
    Vector3 centerOffset = b.center - a.center;
    float distance = centerOffset.magnitude();

    if (a.radius >= distance + b.radius) return a;
    if (b.radius >= distance + a.radius) return b;

    float newRadius = (distance + a.radius + b.radius) * 0.5f;
    Vector3 newCenter = a.center;

    if (distance > 0) {
        newCenter += centerOffset * ((newRadius - a.radius) / distance);
    }

    return BoundingSphere(newCenter, newRadius);
}

float BoundingSphere::getSize() const
{
    return radius * radius;
}
