#include "pch.h"
#include "CapsuleCollider.h"

CapsuleCollider::CapsuleCollider(RigidBody* body, float radius, float height)
    : Collider(body), radius(radius), height(height) {
}

Collider::Type CapsuleCollider::getType() const {
    return Collider::Type::Capsule;
}

bool CapsuleCollider::checkCollision(Collider* other, Contact& contact) const {
    // Stub – You can implement Capsule-Sphere, Capsule-Plane, etc. here.
    return false;
}

float CapsuleCollider::getRadius() const {
    return radius;
}

float CapsuleCollider::getHeight() const {
    return height;
}

Vector3 CapsuleCollider::getStart() const {
    Vector3 center = body->getPosition();
    return center + Vector3(0, height * 0.5f, 0);
}

Vector3 CapsuleCollider::getEnd() const {
    Vector3 center = body->getPosition();
    return center - Vector3(0, height * 0.5f, 0);
}
