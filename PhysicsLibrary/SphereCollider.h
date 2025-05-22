#pragma once
#include "Collider.h"
#include "RigidBody.h"

class SphereCollider : public Collider
{
public:
    SphereCollider(RigidBody* body, float radius);

    float GetRadius() const;
	DirectX::XMVECTOR GetCenter() const;
    Type GetType() const override;

protected:
    bool CheckCollisionWithBox(Collider* other, Contact& contact) const override;
    bool CheckCollisionWithSphere(Collider* other, Contact& contact) const override;
    bool CheckCollisionWithPlane(Collider* other, Contact& contact) const override;
    bool CheckCollisionWithCapsule(Collider* other, Contact& contact) const override;

private:
    float Radius;
};
