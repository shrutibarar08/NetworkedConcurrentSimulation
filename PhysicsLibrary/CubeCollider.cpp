#include "pch.h"

#include "CubeCollider.h"
#include "Contact.h"

#include <algorithm>
#include <cmath>

#include "CapsuleCollider.h"
#include "SphereCollider.h"


CubeCollider::CubeCollider(RigidBody* body)
    : ICollider(body)
{
    DirectX::XMFLOAT3 scale; DirectX::XMStoreFloat3(&scale, m_Scale);
    m_RigidBody->ComputeInverseInertiaTensorBox(scale.x, scale.y, scale.z);
}

bool CubeCollider::CheckCollision(ICollider* other, Contact& outContact)
{
    if (!other) return false;

    if (other->GetColliderType() == ColliderType::Cube)
    {
        return CheckCollisionWithCube(other, outContact);
    }

    if (other->GetColliderType() == ColliderType::Sphere)
    {
        return CheckCollisionWithSphere(other, outContact);
    }
    if (other->GetColliderType() == ColliderType::Capsule)
    {
        return CheckCollisionWithCapsule(other, outContact);
    }

    return false;
}

ColliderType CubeCollider::GetColliderType() const
{
    return ColliderType::Cube;
}

RigidBody* CubeCollider::GetRigidBody() const
{
    return ICollider::GetRigidBody();
}

DirectX::XMVECTOR CubeCollider::GetHalfExtents() const
{
    DirectX::XMVECTOR scale = GetScale();
    return DirectX::XMVectorScale(scale, 0.5f);
}

bool CubeCollider::CheckCollisionWithCube(ICollider* other, Contact& outContact)
{
    using namespace DirectX;

    // === STEP 1: Type check & cast ===
    if (!other || other->GetColliderType() != ColliderType::Cube) return false;
    const CubeCollider* B = other->As<CubeCollider>();
    if (!B) return false;

    // === STEP 2: Get transforms and OBB axes ===
    const XMVECTOR posA = m_RigidBody->GetPosition();
    const XMVECTOR posB = B->GetRigidBody()->GetPosition();

    const XMVECTOR halfA = GetHalfExtents();
    const XMVECTOR halfB = B->GetHalfExtents();

    const Quaternion qA = m_RigidBody->GetOrientation();
    const Quaternion qB = B->GetRigidBody()->GetOrientation();

    XMVECTOR axesA[3];
    XMVECTOR axesB[3];

    GetOBBAxes(qA, axesA);
    GetOBBAxes(qB, axesB);

    const XMVECTOR toCenter = posB - posA;

    float minOverlap = FLT_MAX;
    XMVECTOR bestAxis = XMVectorZero();

    // === STEP 3: Test all 15 axes ===
    for (int i = 0; i < 3; ++i)
    {
        XMVECTOR axis = axesA[i];
        if (!TryNormalize(axis)) continue;

        float projA = ProjectOBB(axis, axesA, halfA);
        float projB = ProjectOBB(axis, axesB, halfB);
        float dist = fabsf(XMVectorGetX(XMVector3Dot(toCenter, axis)));
        float overlap = projA + projB - dist;

        if (overlap < 0) return false; // Separating axis found

        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            bestAxis = axis;
            if (XMVectorGetX(XMVector3Dot(toCenter, axis)) < 0)
                bestAxis = -axis;
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        XMVECTOR axis = axesB[i];
        if (!TryNormalize(axis)) continue;

        float projA = ProjectOBB(axis, axesA, halfA);
        float projB = ProjectOBB(axis, axesB, halfB);
        float dist = fabsf(XMVectorGetX(XMVector3Dot(toCenter, axis)));
        float overlap = projA + projB - dist;

        if (overlap < 0) return false;

        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            bestAxis = axis;
            if (XMVectorGetX(XMVector3Dot(toCenter, axis)) < 0)
                bestAxis = -axis;
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            XMVECTOR axis = XMVector3Cross(axesA[i], axesB[j]);
            if (!TryNormalize(axis)) continue;

            float projA = ProjectOBB(axis, axesA, halfA);
            float projB = ProjectOBB(axis, axesB, halfB);
            float dist = fabsf(XMVectorGetX(XMVector3Dot(toCenter, axis)));
            float overlap = projA + projB - dist;

            if (overlap < 0) return false;

            if (overlap < minOverlap)
            {
                minOverlap = overlap;
                bestAxis = axis;
                if (XMVectorGetX(XMVector3Dot(toCenter, axis)) < 0)
                    bestAxis = -axis;
            }
        }
    }

    // === STEP 4: Fill out contact data ===
    outContact.Colliders[0] = this;
    outContact.Colliders[1] = other;

    XMStoreFloat3(&outContact.ContactNormal, bestAxis);
    outContact.PenetrationDepth = minOverlap;

    // Approximate center point between boxes (improve later via clipping or contact manifold)
    XMVECTOR midpoint = 0.5f * (posA + posB);
    XMStoreFloat3(&outContact.ContactPoint, midpoint);

    outContact.Restitution = 0.5f * (
        m_RigidBody->GetRestitution() + B->GetRigidBody()->GetRestitution());
    outContact.Friction = 0.5f * (
        m_RigidBody->GetFriction() + B->GetRigidBody()->GetFriction());
    outContact.Elasticity = 0.5f * (
        m_RigidBody->GetElasticity() + B->GetRigidBody()->GetElasticity());

    return true;
}

bool CubeCollider::CheckCollisionWithSphere(ICollider* other, Contact& outContact)
{
    if (!other || other->GetColliderType() != ColliderType::Sphere) return false;

    SphereCollider* sphere = other->As<SphereCollider>();
    if (!sphere) return false;

    using namespace DirectX;

    // === STEP 1: Get transforms ===
    XMVECTOR sphereCenter = sphere->GetRigidBody()->GetPosition();
    float radius = sphere->GetRadius();

    XMVECTOR cubeCenter = m_RigidBody->GetPosition();
    XMVECTOR cubeHalfExtents = GetHalfExtents();
    Quaternion cubeOrientation = m_RigidBody->GetOrientation();

    // === STEP 2: Get cube's local axes ===
    XMVECTOR axes[3];
    GetOBBAxes(cubeOrientation, axes);

    // === STEP 3: Find closest point on cube to sphere center ===
    XMVECTOR relative = sphereCenter - cubeCenter;
    XMVECTOR closest = cubeCenter;

    for (int i = 0; i < 3; ++i)
    {
        float distance = XMVectorGetX(XMVector3Dot(relative, axes[i]));
        float extent = XMVectorGetByIndex(cubeHalfExtents, i); // .x .y .z depending on axis
        float clamped = std::clamp(distance, -extent, extent);
        closest += axes[i] * clamped;
    }

    // === STEP 4: Check for collision ===
    XMVECTOR diff = sphereCenter - closest;
    float distSq = XMVectorGetX(XMVector3LengthSq(diff));

    if (distSq > radius * radius)
        return false;

    float distance = std::sqrt(distSq);
    XMVECTOR normal = (distance > 1e-6f) ? XMVector3Normalize(diff) : XMVectorSet(1, 0, 0, 0);

    // === STEP 5: Fill contact ===
    outContact.Colliders[0] = this;
    outContact.Colliders[1] = other;

    XMStoreFloat3(&outContact.ContactPoint, closest);
    XMStoreFloat3(&outContact.ContactNormal, normal);
    outContact.PenetrationDepth = radius - distance;

    outContact.Restitution = 0.5f * (
        m_RigidBody->GetRestitution() + sphere->GetRigidBody()->GetRestitution());

    outContact.Friction = 0.5f * (
        m_RigidBody->GetFriction() + sphere->GetRigidBody()->GetFriction());

    outContact.Elasticity = 0.5f * (
        m_RigidBody->GetElasticity() + sphere->GetRigidBody()->GetElasticity());

    return true;
}

bool CubeCollider::CheckCollisionWithCapsule(ICollider* other, Contact& outContact)
{
    using namespace DirectX;

    if (!other || other->GetColliderType() != ColliderType::Capsule) return false;
    CapsuleCollider* capsule = other->As<CapsuleCollider>();
    if (!capsule) return false;

    RigidBody* cubeBody = m_RigidBody;
    RigidBody* capsuleBody = capsule->GetRigidBody();
    if (!cubeBody || !capsuleBody) return false;

    float capsuleRadius = capsule->GetRadius();
    float capsuleHeight = capsule->GetHeight();

    // === STEP 1: Get capsule segment endpoints ===
    Quaternion q = capsuleBody->GetOrientation();
    XMVECTOR up = q.RotateVector(XMVectorSet(0, 1, 0, 0));

    XMVECTOR capCenter = capsuleBody->GetPosition();
    XMVECTOR halfHeightVec = up * (capsuleHeight * 0.5f);
    XMVECTOR capA = capCenter - halfHeightVec;
    XMVECTOR capB = capCenter + halfHeightVec;

    // === STEP 2: Get cube data ===
    XMVECTOR cubeCenter = cubeBody->GetPosition();
    XMVECTOR cubeHalfExtents = GetHalfExtents();

    Quaternion cubeRot = cubeBody->GetOrientation();
    XMVECTOR axes[3];
    GetOBBAxes(cubeRot, axes);

    // === STEP 3: Find closest points ===
    XMVECTOR capsulePt, cubePt;
    CapsuleCollider::ClosestPtSegmentOBB(capA, capB, cubeCenter, axes, cubeHalfExtents, capsulePt, cubePt);

    XMVECTOR delta = capsulePt - cubePt;
    float distSq = XMVectorGetX(XMVector3LengthSq(delta));

    if (distSq > capsuleRadius * capsuleRadius)
        return false;

    float distance = std::sqrt(distSq);
    XMVECTOR normal = (distance > 1e-6f) ? XMVector3Normalize(delta) : XMVectorSet(1, 0, 0, 0);
    XMVECTOR contactPoint = cubePt;

    // === STEP 4: Fill Contact ===
    outContact.Colliders[0] = this;
    outContact.Colliders[1] = capsule;

    XMStoreFloat3(&outContact.ContactPoint, contactPoint);
    XMStoreFloat3(&outContact.ContactNormal, normal);
    outContact.PenetrationDepth = capsuleRadius - distance;

    outContact.Restitution = 0.5f * (cubeBody->GetRestitution() + capsuleBody->GetRestitution());
    outContact.Friction = 0.5f * (cubeBody->GetFriction() + capsuleBody->GetFriction());
    outContact.Elasticity = 0.5f * (cubeBody->GetElasticity() + capsuleBody->GetElasticity());

    return true;
}

void CubeCollider::SetScale(const DirectX::XMVECTOR& vector)
{
    m_Scale = vector;
    DirectX::XMFLOAT3 scale; DirectX::XMStoreFloat3(&scale, m_Scale);
    m_RigidBody->ComputeInverseInertiaTensorBox(scale.x, scale.y, scale.z);
}

DirectX::XMVECTOR CubeCollider::GetScale() const
{
    DirectX::XMVECTOR scale = m_Scale;
    return scale;
}

void CubeCollider::GetOBBAxes(const Quaternion& q, DirectX::XMVECTOR axes[3])
{
    using namespace DirectX;
    XMMATRIX rotMat = q.ToRotationMatrix();
    axes[0] = rotMat.r[0]; // Right (local X)
    axes[1] = rotMat.r[1]; // Up    (local Y)
    axes[2] = rotMat.r[2]; // Forward (local Z)
}

float CubeCollider::ProjectOBB(const DirectX::XMVECTOR& axis, const DirectX::XMVECTOR axes[3], const DirectX::XMVECTOR& halfExtents)
{
    using namespace DirectX;
    return fabsf(XMVectorGetX(XMVector3Dot(axes[0], axis))) * XMVectorGetX(halfExtents) +
        fabsf(XMVectorGetX(XMVector3Dot(axes[1], axis))) * XMVectorGetY(halfExtents) +
        fabsf(XMVectorGetX(XMVector3Dot(axes[2], axis))) * XMVectorGetZ(halfExtents);
}

bool CubeCollider::TryNormalize(DirectX::XMVECTOR& axis)
{
    using namespace DirectX;
    if (XMVector3LengthSq(axis).m128_f32[0] < 1e-6f) return false;
    axis = XMVector3Normalize(axis);
    return true;
}
