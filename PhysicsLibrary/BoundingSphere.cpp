#include "pch.h"
#include "BoundingSphere.h"
#include <cmath>

BoundingSphere::BoundingSphere() :
Radius(1.0f)
{}

BoundingSphere::BoundingSphere(const DirectX::XMVECTOR& center, float r)
    : Center(center), Radius(r)
{}

bool BoundingSphere::Overlaps(const BoundingSphere& other) const
{
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(Center, other.Center);
	DirectX::XMVECTOR distSqVec = DirectX::XMVector3LengthSq(diff);

    float radiusSum = Radius + other.Radius;
    float radiusSumSq = radiusSum * radiusSum;

    float distSq = DirectX::XMVectorGetX(distSqVec);
	return distSq < radiusSumSq;
}

float BoundingSphere::GetGrowth(const BoundingSphere& other) const
{
    BoundingSphere merged = BoundingSphere::Merge(*this, other);
    return merged.Radius * merged.Radius - Radius * Radius;
}

BoundingSphere BoundingSphere::Merge(
    const BoundingSphere& a,
    const BoundingSphere& b)
{
	DirectX::XMVECTOR centerOffset = DirectX::XMVectorSubtract(b.Center, a.Center);
	DirectX::XMVECTOR distSqVec = DirectX::XMVector3LengthSq(centerOffset);

	float distSq = DirectX::XMVectorGetX(distSqVec);
	float dist = std::sqrt(distSq);

    if (a.Radius >= dist + b.Radius) return a;
    if (b.Radius >= dist + a.Radius) return b;

    float newRadius = (dist + a.Radius + b.Radius) * 0.5f;

	DirectX::XMVECTOR direction = DirectX::XMVector3Normalize(centerOffset);
	DirectX::XMVECTOR offset = DirectX::XMVectorScale(direction, newRadius - a.Radius);
	DirectX::XMVECTOR newCenter = DirectX::XMVectorAdd(a.Center, offset);

    return { newCenter, newRadius };
}
