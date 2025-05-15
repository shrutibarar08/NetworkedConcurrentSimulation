#include "pch.h"
#include "PlaneCollider.h"

PlaneCollider::PlaneCollider(const Vector3& normal, float offset)
    : normal(normal), offset(offset) {}

Collider::Type PlaneCollider::getType() const {
    return Type::Plane;
}

Vector3 PlaneCollider::getNormal() const {
    return normal;
}

float PlaneCollider::getOffset() const {
    return offset;
}
