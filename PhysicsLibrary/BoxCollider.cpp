#include "pch.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "PlaneCollider.h"
#include <algorithm>

BoxCollider::BoxCollider(RigidBody* body, const Vector3& halfExtents)
    : Collider(body), halfExtents(halfExtents) {
}

Collider::Type BoxCollider::getType() const {
    return Collider::Type::Box;
}

bool BoxCollider::checkCollision(Collider* other, Contact& contact) const {
    if (other->getType() == Collider::Type::Box) {
        // TODO: Implement Box-Box collision
    }
    else if (other->getType() == Collider::Type::Sphere) {
        SphereCollider* sphere = static_cast<SphereCollider*>(other);
        Vector3 boxMin = getCenter() - halfExtents;
        Vector3 boxMax = getCenter() + halfExtents;
        Vector3 sphereCenter = sphere->getCenter();

        Vector3 closestPoint = Vector3(
            std::max(boxMin.x, std::min(sphereCenter.x, boxMax.x)),
            std::max(boxMin.y, std::min(sphereCenter.y, boxMax.y)),
            std::max(boxMin.z, std::min(sphereCenter.z, boxMax.z)));

        Vector3 diff = sphereCenter - closestPoint;
        float distanceSq = diff.squaredMagnitude();
        float radius = sphere->getRadius();

        if (distanceSq <= radius * radius) {
            float distance = std::sqrt(distanceSq);
            contact.body[0] = getBody();
            contact.body[1] = sphere->getBody();
            contact.contactNormal = diff.normalized();
            contact.penetration = radius - distance;
            contact.contactPoint = closestPoint;
            contact.restitution = 0.5f;
            contact.friction = 0.3f;
            return true;
        }
    }
    else if (other->getType() == Collider::Type::Plane) {
        PlaneCollider* plane = static_cast<PlaneCollider*>(other);
        Vector3 center = getCenter();
        float projection = halfExtents.x * std::abs(Vector3(1, 0, 0).dot(plane->getNormal())) +
            halfExtents.y * std::abs(Vector3(0, 1, 0).dot(plane->getNormal())) +
            halfExtents.z * std::abs(Vector3(0, 0, 1).dot(plane->getNormal()));

        float boxDistance = center.dot(plane->getNormal()) - plane->getOffset();

        if (boxDistance <= projection) {
            contact.body[0] = getBody();
            contact.body[1] = plane->getBody();
            contact.contactNormal = plane->getNormal();
            contact.penetration = projection - boxDistance;
            contact.contactPoint = center - plane->getNormal() * boxDistance;
            contact.restitution = 0.5f;
            contact.friction = 0.3f;
            return true;
        }
    }
    return false;
}

Vector3 BoxCollider::getHalfExtents() const {
    return halfExtents;
}

Vector3 BoxCollider::getCenter() const {
    return body->getPosition();
}
