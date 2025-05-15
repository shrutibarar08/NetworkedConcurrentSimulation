#include "pch.h"
#include "BoxCollide.h"

BoxCollider::BoxCollider(RigidBody* body, const Vector3& halfExtents)
    : Collider(body), halfExtents(halfExtents) {
}

Collider::Type BoxCollider::getType() const {
    return Collider::Type::Box;
}

bool BoxCollider::checkCollision(Collider* other, Contact& contact) const {
    // Stub – Implement Box-Box, Box-Sphere, Box-Plane, etc.
    return false;
}

Vector3 BoxCollider::getHalfExtents() const {
    return halfExtents;
}

Vector3 BoxCollider::getCenter() const {
    return body->getPosition();
}