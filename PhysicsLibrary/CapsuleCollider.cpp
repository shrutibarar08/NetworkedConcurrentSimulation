#include "pch.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include "PlaneCollider.h"
#include <cmath>
#include <algorithm>

CapsuleCollider::CapsuleCollider(RigidBody* body, float radius, float height)
    : Collider(body), radius(radius), height(height) {
}

Collider::Type CapsuleCollider::getType() const {
    return Collider::Type::Capsule;
}

bool CapsuleCollider::checkCollision(Collider* other, Contact& contact) const {
    if (other->getType() == Collider::Type::Capsule) {
    }
    else if (other->getType() == Collider::Type::Sphere) {
        SphereCollider* sphere = static_cast<SphereCollider*>(other);
        Vector3 capsuleStart = getStart();
        Vector3 capsuleEnd = getEnd();
        Vector3 sphereCenter = sphere->getCenter();

        Vector3 ab = capsuleEnd - capsuleStart;
        Vector3 ac = sphereCenter - capsuleStart;
        float t = ac.dot(ab) / ab.dot(ab);
        t = std::max(0.0f, std::min(1.0f, t));
        Vector3 closestPoint = capsuleStart + ab * t;

        Vector3 diff = sphereCenter - closestPoint;
        t = ac.dot(ab) / ab.dot(ab);
        float distanceSq = diff.squaredMagnitude();
        float radiusSum = radius + sphere->getRadius();

        if (distanceSq <= radiusSum * radiusSum) {
            float distance = std::sqrt(distanceSq);
            float t = ac.dot(ab) / ab.dot(ab);
            contact.body[0] = getBody();
            contact.body[1] = sphere->getBody();
            contact.contactNormal = diff.normalized();
            contact.penetration = radiusSum - distance;
            contact.contactPoint = closestPoint + contact.contactNormal * (radius * 0.5f);
            contact.restitution = 0.5f;
            contact.friction = 0.3f;
            return true;
        }
    }
    else if (other->getType() == Collider::Type::Plane) {
        PlaneCollider* plane = static_cast<PlaneCollider*>(other);
        Vector3 capsuleStart = getStart();
        Vector3 capsuleEnd = getEnd();

        float dStart = plane->getNormal().dot(capsuleStart) - plane->getOffset();
        float dEnd = plane->getNormal().dot(capsuleEnd) - plane->getOffset();

        if (dStart <= radius || dEnd <= radius) {
            contact.body[0] = getBody();
            contact.body[1] = plane->getBody();
            contact.contactNormal = plane->getNormal();
            contact.penetration = radius - std::min(dStart, dEnd);
            contact.contactPoint = (dStart < dEnd) ? capsuleStart : capsuleEnd;
            contact.restitution = 0.5f;
            contact.friction = 0.3f;
            return true;
        }
    }
    return false;
}

float CapsuleCollider::getRadius() const {
    return radius;
}

float CapsuleCollider::getHeight() const {
    return height;
}

Vector3 CapsuleCollider::getStart() const {
    Vector3 center = getBody()->getPosition();
    return center + Vector3(0, height * 0.5f, 0);
}

Vector3 CapsuleCollider::getEnd() const {
    Vector3 center = getBody()->getPosition();
    return center - Vector3(0, height * 0.5f, 0);}