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

void ICollider::SetLastHitCollider(ICollider* collider)
{
	if (m_LastHitCollider && m_LastHitCollider == collider)
	{
		m_LastHitColliderCounts++;
	}else
	{
		m_LastHitCollider = collider;
		m_LastHitColliderCounts = 0;
		m_LastHitResolved = false;
	}
}

int ICollider::GetHitCount(ICollider* collider, float totalTime)
{
	if (m_LastHitCollider && m_LastHitCollider == collider)
	{
		if (totalTime - m_LastHitTime <= 0.3)
		{
			m_LastHitColliderCounts++;
			return m_LastHitColliderCounts;
		}
		m_LastHitColliderCounts = 1;
		return m_LastHitColliderCounts;
	}
	SetLastHitCollider(collider);
	return m_LastHitColliderCounts;
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
