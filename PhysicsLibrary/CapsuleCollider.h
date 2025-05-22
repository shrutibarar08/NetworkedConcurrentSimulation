#pragma once
#include <DirectXMath.h>

#include "Collider.h"
#include "RigidBody.h"

class CapsuleCollider : public Collider
{
public:
    CapsuleCollider(RigidBody* body, float radius, float height);

	Type GetType() const override;

    float GetRadius() const;
    float GetHeight() const;
    DirectX::XMVECTOR GetStart() const;
    DirectX::XMVECTOR GetEnd() const;

protected:
    bool CheckCollisionWithBox(Collider* other, Contact& contact) const override;
    bool CheckCollisionWithSphere(Collider* other, Contact& contact) const override;
    bool CheckCollisionWithPlane(Collider* other, Contact& contact) const override;
    bool CheckCollisionWithCapsule(Collider* other, Contact& contact) const override;

private:
    const float EPSILON = 1e-6f;
    float Radius;
    float Height;
};
