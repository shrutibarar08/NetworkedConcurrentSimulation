#pragma once

#include "ICollider.h"
#include <DirectXMath.h>

class SphereCollider : public ICollider
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

private:
    bool CheckCollisionWithSphere(ICollider* other, Contact& outContact);
    bool CheckCollisionWithCube(ICollider* other, Contact& outContact);
private:
    SRWLOCK m_Lock{ SRWLOCK_INIT };
    float m_Radius{ 1.f };
};
