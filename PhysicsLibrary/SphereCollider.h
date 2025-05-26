#pragma once

#include "ICollider.h"
#include <DirectXMath.h>

class SphereCollider final: public ICollider
{
public:
    explicit SphereCollider(RigidBody* body);
    ~SphereCollider() override = default;

    bool CheckCollision(ICollider* other, Contact& outContact) override;
    ColliderType GetColliderType() const override;
    RigidBody* GetRigidBody() const override;

    // Sphere-specific
    void SetRadius(float radius);
    float GetRadius() const;

    void SetScale(const DirectX::XMVECTOR& vector) override;
    DirectX::XMVECTOR GetScale() const override;

private:
    bool CheckCollisionWithSphere(ICollider* other, Contact& outContact);
    bool CheckCollisionWithCube(ICollider* other, Contact& outContact);
    bool CheckCollisionWithCapsule(ICollider* other, Contact& outContact);

    float ClosestPtPointSegment(DirectX::XMVECTOR p, DirectX::XMVECTOR a, DirectX::XMVECTOR b);

private:
    mutable SRWLOCK m_Lock{ SRWLOCK_INIT };
    float m_Radius{ 1.f };
    DirectX::XMVECTOR m_Scale{ 1.f, 1.f, 1.f };
};
