#include "pch.h"
#include "PlaneCollider.h"

PlaneCollider::PlaneCollider(const Vector3& n, float o)
    : normal(n.normalized()), offset(o) {
}

Collider::Type PlaneCollider::getType() const {
    return Collider::Type::Plane;
}

Particle* PlaneCollider::getParticle() const {
    return nullptr;  // planes are static
}

Vector3 PlaneCollider::getNormal() const {
    return normal;
}

float PlaneCollider::getOffset() const {
    return offset;
}