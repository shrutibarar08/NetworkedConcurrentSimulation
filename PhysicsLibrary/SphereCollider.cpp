#include "pch.h"
#include "SphereCollider.h"
#include "Contact.h"
#include <cmath>
#include <algorithm>

#include "CapsuleCollider.h"
#include "CubeCollider.h"


SphereCollider::SphereCollider(RigidBody* body)
    : ICollider(body)
{
    m_RigidBody->ComputeInverseInertiaTensorSphere(m_Radius);
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
    if (other->GetColliderType() == ColliderType::Capsule)
    {
        return CheckCollisionWithCapsule(other, outContact);
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
    m_Radius = (radius > 0.01f) ? radius : 0.01f;
    m_RigidBody->ComputeInverseInertiaTensorSphere(m_Radius);
}

float SphereCollider::GetRadius() const
{
    auto result = m_Radius;
    return result;
}

void SphereCollider::SetScale(const DirectX::XMVECTOR& vector)
{
    using namespace DirectX;

    // Extract and average the scale vector
    XMFLOAT3 scale;
    XMStoreFloat3(&scale, vector);
    float avg = (scale.x + scale.y + scale.z) / 3.0f;

    m_Radius = avg * 0.5f;
    m_Scale = { scale.x, scale.y, scale.z };
    m_RigidBody->ComputeInverseInertiaTensorSphere(m_Radius);
}

DirectX::XMVECTOR SphereCollider::GetScale() const
{
    float diameter = m_Radius;
    return DirectX::XMVectorSet(diameter, diameter, diameter, 0.0f);
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

bool SphereCollider::CheckCollisionWithCapsule(ICollider* other, Contact& outContact)
{
    using namespace DirectX;

    if (!other || other->GetColliderType() != ColliderType::Capsule) return false;
    CapsuleCollider* capsule = other->As<CapsuleCollider>();
    if (!capsule) return false;

    RigidBody* sphereBody = m_RigidBody;
    RigidBody* capsuleBody = capsule->GetRigidBody();
    if (!sphereBody || !capsuleBody) return false;

    float sphereRadius = GetRadius();
    float capsuleRadius = capsule->GetRadius();
    float capsuleHeight = capsule->GetHeight();

    // Get capsule orientation and up vector
    Quaternion q = capsuleBody->GetOrientation();
    XMVECTOR up = q.RotateVector(XMVectorSet(0, 1, 0, 0));

    XMVECTOR capCenter = capsuleBody->GetPosition();
    XMVECTOR sphereCenter = sphereBody->GetPosition();

    XMVECTOR halfHeightVec = up * (capsuleHeight * 0.5f);
    XMVECTOR capA = capCenter - halfHeightVec;
    XMVECTOR capB = capCenter + halfHeightVec;

    // Find closest point on capsule line segment to sphere center
    float t = ClosestPtPointSegment(sphereCenter, capA, capB);
    XMVECTOR closestPoint = capA + (capB - capA) * t;

    // Vector from closest point to sphere center
    XMVECTOR delta = sphereCenter - closestPoint;
    float distSq = XMVectorGetX(XMVector3LengthSq(delta));
    float totalRadius = sphereRadius + capsuleRadius;

    if (distSq > totalRadius * totalRadius)
        return false; // No collision

    float distance = std::sqrt(distSq);
    XMVECTOR normal = (distance > 1e-6f) ? XMVector3Normalize(delta) : XMVectorSet(1, 0, 0, 0);
    XMVECTOR contactPoint = closestPoint + normal * capsuleRadius;

    // === Fill Contact ===
    outContact.Colliders[0] = this;
    outContact.Colliders[1] = capsule;

    XMStoreFloat3(&outContact.ContactPoint, contactPoint);
    XMStoreFloat3(&outContact.ContactNormal, normal);
    outContact.PenetrationDepth = totalRadius - distance;

    outContact.Restitution = 0.5f * (sphereBody->GetRestitution() + capsuleBody->GetRestitution());
    outContact.Friction = 0.5f * (sphereBody->GetFriction() + capsuleBody->GetFriction());
    outContact.Elasticity = 0.5f * (sphereBody->GetElasticity() + capsuleBody->GetElasticity());

    return true;
}

float SphereCollider::ClosestPtPointSegment(DirectX::XMVECTOR p, DirectX::XMVECTOR a, DirectX::XMVECTOR b)
{
    using namespace DirectX;
    XMVECTOR ab = b - a;
    XMVECTOR ap = p - a;
    float t = XMVectorGetX(XMVector3Dot(ap, ab)) / XMVectorGetX(XMVector3Dot(ab, ab));
    return std::clamp(t, 0.0f, 1.0f);
}
