#pragma once
#include "Collider.h"

class SphereCollider : public Collider {
private:
    Particle* particle;
    float radius;

public:
    SphereCollider(Particle* p, float r);

    Type getType() const override;
    Particle* getParticle() const override;
    float getRadius() const;
    Vector3 getCenter() const;
};
