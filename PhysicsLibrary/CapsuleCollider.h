#pragma once
#include "Collider.h"
#include "RigidBody.h"

class CapsuleCollider : public Collider {
private:
    float radius;
    float height;

public:
    CapsuleCollider(RigidBody* body, float radius, float height);

    Collider::Type getType() const override;
    bool checkCollision(Collider* other, Contact& contact) const override;

    float getRadius() const;
    float getHeight() const;
    Vector3 getStart() const;
    Vector3 getEnd() const;
};