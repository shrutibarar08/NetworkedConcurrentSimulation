#pragma once
#include "Vector3.h"

class BoundingSphere
{
public:
	Vector3 center;
	float radius;

	BoundingSphere();
    BoundingSphere(const Vector3& center, float radius);

    bool overlaps(const BoundingSphere& other) const;
    static BoundingSphere merge(const BoundingSphere& a, const BoundingSphere& b);
    float getSize() const;
};

