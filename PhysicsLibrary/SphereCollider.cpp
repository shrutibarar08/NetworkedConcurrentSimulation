#include "pch.h"
#include "SphereCollider.h"

SphereCollider::SphereCollider(Particle* p, float r)
    : particle(p), radius(r) {
}

Collider::Type SphereCollider::getType() const {
    return Collider::Type::Sphere;
}

Particle* SphereCollider::getParticle() const {
    return particle;
}

float SphereCollider::getRadius() const {
    return radius;
}

Vector3 SphereCollider::getCenter() const {
    return particle->getPosition();
}
