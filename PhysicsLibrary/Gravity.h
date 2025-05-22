#pragma once
#include "ForceGenerator.h"
#include <DirectXMath.h>

class Gravity : public ForceGenerator
{
public:
    Gravity(const DirectX::XMVECTOR& g);
    void UpdateForce(RigidBody* body, float duration) override;

private:
    DirectX::XMVECTOR m_GravityForce;
};
