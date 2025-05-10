#pragma once
#include "Collider.h"

class PlaneCollider : public Collider {
private:
    Vector3 normal;  // Normal vector of the plane
    float offset;    // Distance from origin along normal

public:
    PlaneCollider(const Vector3& normal, float offset);

    Type getType() const override;
    Particle* getParticle() const override;
    Vector3 getNormal() const;
    float getOffset() const;
};
