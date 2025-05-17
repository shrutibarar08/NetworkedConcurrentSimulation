#pragma once
#include "Collider.h"

class PlaneCollider : public Collider {
private:
    Vector3 normal;
    float offset;

public:
    PlaneCollider(const Vector3& normal, float offset, RigidBody* attachedBody);

    Type getType() const override;
    Vector3 getNormal() const;
    float getOffset() const;
};
