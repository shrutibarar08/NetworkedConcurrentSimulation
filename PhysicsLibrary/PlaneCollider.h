#pragma once
#include "Collider.h"

class PlaneCollider : public Collider {
private:
    Vector3 normal;
    float offset;

public:
    PlaneCollider(const Vector3& normal, float offset);

    Type getType() const override;
    Vector3 getNormal() const override;
    float getOffset() const override;
};
