#pragma once
#include <DirectXMath.h>

class BoundingSphere
{
public:
    BoundingSphere();
    BoundingSphere(const DirectX::XMVECTOR& center, float radius);

    bool Overlaps(const BoundingSphere& other) const;
    float GetGrowth(const BoundingSphere& other) const;

    // Combine two bounding spheres into a new one
    static BoundingSphere Merge(const BoundingSphere& a, const BoundingSphere& b);

    //~ Members
    DirectX::XMVECTOR Center{};
    float Radius;
};
