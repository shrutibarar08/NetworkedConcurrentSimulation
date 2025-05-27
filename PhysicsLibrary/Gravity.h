#pragma once
#include "ForceGenerator.h"
#include <DirectXMath.h>

#include "CapsuleCollider.h"

class Gravity : public ForceGenerator
{
public:
    Gravity(const DirectX::XMVECTOR& g);
    void UpdateForce(ICollider* collider, float duration) override;

    bool IsGravityOn() const;
    void SetGravity(bool flag);

private:
    mutable SRWLOCK m_Lock{ SRWLOCK_INIT };
    bool m_GravityOn{ true };
    DirectX::XMVECTOR m_GravityForce;
};
