#pragma once
#include "Collider.h"
#include "RigidBody.h"

class BoxCollider : public Collider {
private:
    Vector3 halfExtents;

public:
    BoxCollider(RigidBody* attachedBody, const Vector3& halfExtents);

    Collider::Type getType() const override;
    bool checkCollision(Collider* other, Contact& contact) const override;

    Vector3 getHalfExtents() const;
    Vector3 getCenter() const;
};

