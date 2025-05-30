#include "pch.h"
#include "ICollider.h"


ICollider::ICollider(RigidBody* attachBody)
	: m_RigidBody(attachBody)
{}

void ICollider::RegisterCollision(const ICollider* collider)
{
	RigidBody* rigidBody = collider->GetRigidBody();

	if (m_RigidBody->IsPlatform()) return;

	if (rigidBody->IsPlatform())
	{
		m_PlatformCollisionInfo.hitCount++;
		m_PlatformCollisionInfo.lastHitTime = mTotalElapsedTime;

		if (m_PlatformCollisionInfo.hitCount > mRestThreshold)
		{
			m_RigidBody->SetRestingState(true);
		}
	}else
	{
		m_PlatformCollisionInfo.hitCount = 0;
		m_PlatformCollisionInfo.lastHitTime = 0.0f;
		m_RigidBody->SetRestingState(false);
	}
}

ColliderState ICollider::GetColliderState()
{
	ColliderState state = m_ColliderState;
	return state;
}

void ICollider::SetColliderState(ColliderState state)
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

void ICollider::Update(float deltaTime)
{
	mTotalElapsedTime += deltaTime;

	if (m_PlatformCollisionInfo.hitCount > 0 && 
		mTotalElapsedTime - m_PlatformCollisionInfo.lastHitTime > mRestTimeThreshold)
	{
		m_RigidBody->SetRestingState(false);
		m_PlatformCollisionInfo.hitCount = 0;
		m_PlatformCollisionInfo.lastHitTime = 0.0f;
	}

	m_TransformationMatrix =
		DirectX::XMMatrixScalingFromVector(GetScale()) *
		DirectX::XMMatrixRotationQuaternion(m_RigidBody->GetOrientation().ToXmVector()) *
		DirectX::XMMatrixTranslationFromVector(m_RigidBody->GetPosition());

}

DirectX::XMMATRIX ICollider::GetWorldMatrix() const
{
	using namespace DirectX;
	XMVECTOR scale = GetScale();
	XMVECTOR rotationQuat = m_RigidBody->GetOrientation().ToXmVector();
	XMVECTOR position = m_RigidBody->GetPosition();

	return XMMatrixAffineTransformation(
		scale,                     // Scaling
		XMVectorZero(),            // Rotation origin
		rotationQuat,              // Rotation quaternion
		position                   // Translation
	);
}
