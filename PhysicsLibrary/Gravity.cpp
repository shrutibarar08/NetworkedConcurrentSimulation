#include "pch.h"
#include "Gravity.h"
#include "RigidBody.h"

Gravity::Gravity(const DirectX::XMVECTOR& g)
: m_GravityForce(g)
{}

void Gravity::UpdateForce(RigidBody* body, float duration)
{
    if (!body->HasFiniteMass()) return;

    DirectX::XMVECTOR gravity = m_GravityForce;
    float mass = body->GetMass();

    DirectX::XMVECTOR force = DirectX::XMVectorScale(gravity, mass);

    // Apply the gravitational force to the particle
    body->AddForce(force);
}
