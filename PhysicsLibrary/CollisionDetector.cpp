#include "CollisionDetector.h"
#include <cmath>

// Sphere-Sphere
bool CollisionDetector::detectSphereSphere(const SphereCollider& a, const SphereCollider& b, Contact& contactOut) {
    Vector3 midline = a.getCenter() - b.getCenter();
    float distance = midline.magnitude();
    float penetration = (a.getRadius() + b.getRadius()) - distance;

    if (penetration <= 0) return false;

    contactOut.particle[0] = a.getParticle();
    contactOut.particle[1] = b.getParticle();
    contactOut.contactNormal = midline.normalized();
    contactOut.penetration = penetration;
    contactOut.restitution = 0.6f;

    return true;
}

// Sphere-Plane
bool CollisionDetector::detectSpherePlane(const SphereCollider& sphere, const PlaneCollider& plane, Contact& contactOut) {
    float distance = sphere.getCenter().dot(plane.getNormal()) - plane.getOffset();

    if (distance > sphere.getRadius()) return false;

    contactOut.particle[0] = sphere.getParticle();
    contactOut.particle[1] = nullptr;
    contactOut.contactNormal = plane.getNormal();
    contactOut.penetration = sphere.getRadius() - distance;
    contactOut.restitution = 0.4f;

    return true;
}

// Dispatcher
bool CollisionDetector::dispatch(Collider* a, Collider* b, Contact& contactOut) {
    auto typeA = a->getType();
    auto typeB = b->getType();

    if (typeA == Collider::Type::Sphere && typeB == Collider::Type::Sphere) {
        return detectSphereSphere(
            *static_cast<SphereCollider*>(a),
            *static_cast<SphereCollider*>(b),
            contactOut
        );
    }

    if (typeA == Collider::Type::Sphere && typeB == Collider::Type::Plane) {
        return detectSpherePlane(
            *static_cast<SphereCollider*>(a),
            *static_cast<PlaneCollider*>(b),
            contactOut
        );
    }

    if (typeA == Collider::Type::Plane && typeB == Collider::Type::Sphere) {
        // Flip normal and particles
        bool result = detectSpherePlane(
            *static_cast<SphereCollider*>(b),
            *static_cast<PlaneCollider*>(a),
            contactOut
        );
        if (result) {
            contactOut.contactNormal *= -1;
            std::swap(contactOut.particle[0], contactOut.particle[1]);
        }
        return result;
    }

    return false; 
}
