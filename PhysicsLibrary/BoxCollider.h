#pragma once
#include <DirectXMath.h>

#include "Collider.h"
#include "RigidBody.h"

class BoxCollider : public Collider
{
public:
    BoxCollider(RigidBody* attachedBody, const DirectX::XMVECTOR& halfExtents);

	Type GetType() const override;

    const DirectX::XMVECTOR& GetHalfExtents() const;
    const DirectX::XMVECTOR& GetCenter() const;

protected:
    bool CheckCollisionWithBox(Collider* other, Contact& contact) const override;
    bool CheckCollisionWithSphere(Collider* other, Contact& contact) const override;
    bool CheckCollisionWithPlane(Collider* other, Contact& contact) const override;
    bool CheckCollisionWithCapsule(Collider* other, Contact& contact) const override;

private:
    DirectX::XMVECTOR HalfExtents;
};
