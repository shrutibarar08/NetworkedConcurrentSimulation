#include "pch.h"
#include "Gravity.h"
#include "RigidBody.h"

Gravity::Gravity(const DirectX::XMVECTOR& g)
: m_GravityForce(g)
{}

void Gravity::UpdateForce(ICollider* collider, float duration)
{
    //~ Pre checks
    if (!IsGravityOn() || !collider) return;
    RigidBody* rigidBody = collider->GetRigidBody();
    if (!rigidBody->HasFiniteMass() || collider->GetColliderState() == ColliderSate::Static) return;

    //~ Thread safe access
    AcquireSRWLockShared(&m_Lock);
    DirectX::XMVECTOR gravity = m_GravityForce;
    ReleaseSRWLockShared(&m_Lock);

    //~ apply it hehe
    float mass = rigidBody->GetMass();

    DirectX::XMVECTOR force = DirectX::XMVectorScale(gravity, mass);

    // Apply the gravitational force to the particle
    rigidBody->AddForce(force);
}

bool Gravity::IsGravityOn() const
{
    AcquireSRWLockShared(&m_Lock);
    bool result = m_GravityOn;
    ReleaseSRWLockShared(&m_Lock);
    return result;
}

void Gravity::SetGravity(bool flag)
{
    AcquireSRWLockExclusive(&m_Lock);
    m_GravityOn = flag;
    ReleaseSRWLockExclusive(&m_Lock);
}
