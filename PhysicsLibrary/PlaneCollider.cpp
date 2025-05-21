#include "pch.h"
#include "PlaneCollider.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "Contact.h"

PlaneCollider::PlaneCollider(const Vector3& normal, float offset, RigidBody* attachedBody)
    : normal(normal), offset(offset), Collider(attachedBody) {}

Collider::Type PlaneCollider::getType() const {
    return Type::Plane;
}

Vector3 PlaneCollider::getNormal() const {
    return normal;
}

float PlaneCollider::getOffset() const {
    return offset;
}

bool PlaneCollider::checkCollision(Collider* other, Contact& contact)
{
    if (other->getType() == Collider::Type::Sphere) {
        SphereCollider* sphere = static_cast<SphereCollider*>(other);
        Vector3 center = sphere->getCenter();
        float distance = Vector3::dot(normal, center) - offset;
        if (distance < sphere->getRadius()) {
            contact.body[0] = sphere->getBody();
            contact.body[1] = nullptr; // Static plane
            contact.contactNormal = normal;
            contact.penetration = sphere->getRadius() - distance;
            contact.contactPoint = center - normal * distance;
            contact.restitution = 0.5f;
            contact.friction = 0.3f;
            return true;
        }
    }
    return false;
}
