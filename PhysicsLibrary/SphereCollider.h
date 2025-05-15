#pragma once
#include "Collider.h"
#include "RigidBody.h"

class SphereCollider : public Collider {
public:
    SphereCollider(RigidBody* body, float radius);

    float getRadius() const;
    Vector3 getCenter() const override;
    Type getType() const override;

    bool checkCollision(Collider* other, Contact& contact) const override;

private:
    RigidBody* body;
    float radius;};