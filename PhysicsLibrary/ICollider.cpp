#include "pch.h"
#include "ICollider.h"


ICollider::ICollider(RigidBody* attachBody)
	: m_RigidBody(attachBody)
{}

ColliderSate ICollider::GetColliderState()
{
	ColliderSate state = m_ColliderState;
	return state;
}

void ICollider::SetColliderState(ColliderSate state)
{
	m_ColliderState = state;
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
