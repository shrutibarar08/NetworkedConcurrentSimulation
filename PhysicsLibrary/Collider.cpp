#include "pch.h"
#include "Collider.h"

bool Collider::CheckCollision(Collider* other, Contact& contact) const
{
    switch (other->GetType())
    {
    case Type::BOX:
        return CheckCollisionWithBox(other, contact);
    case Type::PLANE:
        return CheckCollisionWithSphere(other, contact);
    case Type::SPHERE:
        return CheckCollisionWithPlane(other, contact);
    case Type::CAPSULE:
        return CheckCollisionWithCapsule(other, contact);
    default:
        return false;
    }
}
