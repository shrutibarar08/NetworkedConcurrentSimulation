#include "pch.h"
#include "ICollider.h"


ICollider::ICollider(RigidBody* attachBody)
	: m_RigidBody(attachBody)
{}

ColliderSate ICollider::GetColliderState()
{
	AcquireSRWLockShared(&m_Lock);
	ColliderSate state = m_ColliderState;
	ReleaseSRWLockShared(&m_Lock);
	return state;
}

void ICollider::SetColliderState(ColliderSate state)
{
	AcquireSRWLockExclusive(&m_Lock);
	m_ColliderState = state;
	ReleaseSRWLockExclusive(&m_Lock);
}

const char* ICollider::GetColliderTypeName() const
{
    switch (GetColliderType())
	{
	    case ColliderType::Cube: return "Cube";
	    case ColliderType::Sphere: return "Sphere";
	    case ColliderType::Capsule: return "Capsule";
	    default: return "Unknown";
    }
}
