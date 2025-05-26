#include "pch.h"
#include "SphereCollider.h"
#include "Contact.h"
#include <cmath>
#include <algorithm>

#include "CubeCollider.h"

SphereCollider::SphereCollider(RigidBody* body)
	: ICollider(body)
{
}

bool SphereCollider::CheckCollision(ICollider* other, Contact& outContact)
{
	if (other->GetColliderType() == ColliderType::Sphere)
	{
		return CheckCollisionWithSphere(other, outContact);
	}
	if (other->GetColliderType() == ColliderType::Cube)
	{
		return CheckCollisionWithCube(other, outContact);
	}
	return false;
}

ColliderType SphereCollider::GetColliderType() const
{
	return ColliderType::Sphere;
}

RigidBody* SphereCollider::GetRigidBody() const
{
	return ICollider::GetRigidBody();
}

void SphereCollider::SetRadius(float radius)
{
    AcquireSRWLockExclusive(&m_Lock);
    m_Radius = (radius > 0.01) ? radius : 0.01;
    ReleaseSRWLockExclusive(&m_Lock);
}

float SphereCollider::GetRadius() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    auto result = m_Radius;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

bool SphereCollider::CheckCollisionWithSphere(ICollider* other, Contact& outContact)
{
    using namespace DirectX;
    if (!other || other->GetColliderType() != ColliderType::Sphere) return false;

    SphereCollider* otherSphere = other->As<SphereCollider>();
    if (!otherSphere) return false;

    XMVECTOR centerA = m_RigidBody->GetPosition();
    XMVECTOR centerB = otherSphere->GetRigidBody()->GetPosition();

    float radiusA = GetRadius();
    float radiusB = otherSphere->GetRadius();

    XMVECTOR diff = centerB - centerA;
    float distanceSq = XMVectorGetX(XMVector3LengthSq(diff));
    float combinedRadius = radiusA + radiusB;

    if (distanceSq >= combinedRadius * combinedRadius)
        return false; // No collision

    float distance = std::sqrt(distanceSq);
    XMVECTOR normal = (distance > 1e-6f) ? XMVector3Normalize(diff) : XMVectorSet(1, 0, 0, 0); // Fallback normal

    // Fill contact
    outContact.Colliders[0] = this;
    outContact.Colliders[1] = other;

    XMStoreFloat3(&outContact.ContactNormal, normal);
    XMStoreFloat3(&outContact.ContactPoint, centerA + normal * radiusA); // Approximate contact point
    outContact.PenetrationDepth = combinedRadius - distance;

    outContact.Restitution = 0.5f * (
        m_RigidBody->GetRestitution() + otherSphere->GetRigidBody()->GetRestitution());

    outContact.Friction = 0.5f * (
        m_RigidBody->GetFriction() + otherSphere->GetRigidBody()->GetFriction());

    outContact.Elasticity = 0.5f * (
        m_RigidBody->GetElasticity() + otherSphere->GetRigidBody()->GetElasticity());

    return true;
}

bool SphereCollider::CheckCollisionWithCube(ICollider* other, Contact& outContact)
{
    using namespace DirectX;
    if (!other || other->GetColliderType() != ColliderType::Cube) return false;

    CubeCollider* cube = other->As<CubeCollider>();
    if (!cube) return false;

    // === STEP 1: Get transforms ===
    XMVECTOR sphereCenter = m_RigidBody->GetPosition();
    float radius = GetRadius();

    XMVECTOR cubeCenter = cube->GetRigidBody()->GetPosition();
    XMVECTOR cubeHalfExtents = cube->GetHalfExtents();
    Quaternion cubeOrientation = cube->GetRigidBody()->GetOrientation();

    // === STEP 2: Get cube axes ===
    XMVECTOR axes[3];
    cube->GetOBBAxes(cubeOrientation, axes);

    // === STEP 3: Closest point on cube to sphere center ===
    XMVECTOR relative = sphereCenter - cubeCenter;
    XMVECTOR closest = cubeCenter;

    for (int i = 0; i < 3; ++i)
    {
        float distance = XMVectorGetX(XMVector3Dot(relative, axes[i]));
        float clamped = std::clamp(distance, -XMVectorGetX(cubeHalfExtents), XMVectorGetX(cubeHalfExtents));
        closest += axes[i] * clamped;
    }

    // === STEP 4: Test distance to closest point ===
    XMVECTOR diff = sphereCenter - closest;
    float distSq = XMVectorGetX(XMVector3LengthSq(diff));

    if (distSq > radius * radius)
        return false; // No collision

    float distance = std::sqrt(distSq);
    XMVECTOR normal = (distance > 1e-6f) ? XMVector3Normalize(diff) : XMVectorSet(1, 0, 0, 0);

    // === STEP 5: Fill Contact ===
    outContact.Colliders[0] = this;
    outContact.Colliders[1] = other;

    XMStoreFloat3(&outContact.ContactPoint, closest);
    XMStoreFloat3(&outContact.ContactNormal, normal);
    outContact.PenetrationDepth = radius - distance;

    outContact.Restitution = 0.5f * (
        m_RigidBody->GetRestitution() + cube->GetRigidBody()->GetRestitution());

    outContact.Friction = 0.5f * (
        m_RigidBody->GetFriction() + cube->GetRigidBody()->GetFriction());

    outContact.Elasticity = 0.5f * (
        m_RigidBody->GetElasticity() + cube->GetRigidBody()->GetElasticity());

    return true;
}
