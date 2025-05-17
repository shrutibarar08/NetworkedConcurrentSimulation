#include "pch.h"
#include "SphereCollider.h"

SphereCollider::SphereCollider(RigidBody* body, float radius)
    : Collider(body), radius(radius)
{}

float SphereCollider::getRadius() const {
    return radius;
}

Vector3 SphereCollider::getCenter() const {
    return body->getPosition(); // Assumes RigidBody has getPosition()
}

Collider::Type SphereCollider::getType() const {
    return Type::Sphere;
}

bool SphereCollider::checkCollision(Collider* other, Contact& contact) const
{
    if (other->getType() == Type::Sphere) {
        SphereCollider* sphere = static_cast<SphereCollider*>(other);
        Vector3 centerA = getCenter();
        Vector3 centerB = sphere->getCenter();
        Vector3 delta = centerB - centerA;
        float distance = delta.magnitude();
        float totalRadius = radius + sphere->getRadius();

        if (distance < totalRadius) {
            contact.body[0] = body;
            contact.body[1] = sphere->body;
            contact.contactNormal = delta.normalized();
            contact.penetration = totalRadius - distance;
            contact.contactPoint = centerA + delta * 0.5f;
            contact.restitution = 0.9f;
            contact.friction = 0.0f;
            return true;
        }
    }
    return false;
}
