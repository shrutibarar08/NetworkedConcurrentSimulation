#pragma once
#include "Collider.h"

class PlaneCollider : public Collider
{
public:
    PlaneCollider(const DirectX::XMFLOAT3& normal, float offset, RigidBody* attachedBody);

    Type GetType() const override;
    const DirectX::XMFLOAT3& GetNormal() const;
    float GetOffset() const;

protected:
    bool CheckCollisionWithBox(Collider* other, Contact& contact) const override;
    bool CheckCollisionWithSphere(Collider* other, Contact& contact) const override;
    bool CheckCollisionWithPlane(Collider* other, Contact& contact) const override;
    bool CheckCollisionWithCapsule(Collider* other, Contact& contact) const override;

private:
    DirectX::XMFLOAT3 Normal;
    float Offset;
};
