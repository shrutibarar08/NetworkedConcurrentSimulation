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

const char* ICollider::ToString() const
{
	switch (GetColliderType())
	{
	case ColliderType::Cube:    return "Cube";
	case ColliderType::Sphere:  return "Sphere";
	case ColliderType::Capsule: return "Capsule";
	default:                    return "Unknown";
	}
	return "null";
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

DirectX::XMMATRIX ICollider::GetTransformationMatrix() const
{
	return m_TransformationMatrix;
}

void ICollider::Update()
{
	m_TransformationMatrix =
		DirectX::XMMatrixScalingFromVector(GetScale()) *
		DirectX::XMMatrixRotationQuaternion(m_RigidBody->GetOrientation().ToXmVector()) *
		DirectX::XMMatrixTranslationFromVector(m_RigidBody->GetPosition());

}
