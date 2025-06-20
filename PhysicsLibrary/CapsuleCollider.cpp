#include "pch.h"
#include "CapsuleCollider.h"
#include "Contact.h"

#include <cmath>
#include <algorithm>

#include "CubeCollider.h"
#include "SphereCollider.h"

CapsuleCollider::CapsuleCollider(RigidBody* body)
    : ICollider(body)
{
    m_RigidBody->ComputeInverseInertiaTensorCapsule(m_Radius, m_Height);
}

bool CapsuleCollider::CheckCollision(ICollider* other, Contact& outContact)
{
    if (other->GetColliderType() == ColliderType::Capsule)
    {
        return CheckCollisionWithCapsule(other, outContact);
    }
    if (other->GetColliderType() == ColliderType::Cube)
    {
        return CheckCollisionWithCube(other, outContact);
    }
    if (other->GetColliderType() == ColliderType::Sphere)
    {
        return CheckCollisionWithSphere(other, outContact);
    }

    return false;
}

ColliderType CapsuleCollider::GetColliderType() const
{
    return ColliderType::Capsule;
}

void CapsuleCollider::SetRadius(float radius)
{
    m_Radius = radius;

    // Update X and Z components of scale (diameter)
    float diameter = radius * 1.3f;

    DirectX::XMFLOAT3 scale;
    DirectX::XMStoreFloat3(&scale, m_Scale);
    scale.x = diameter;
    scale.z = diameter;
    m_Scale = DirectX::XMLoadFloat3(&scale);
    m_RigidBody->ComputeInverseInertiaTensorCapsule(m_Radius, m_Height);
}

void CapsuleCollider::SetHeight(float height)
{
    m_Height = height;

    // Update Y component of scale
    DirectX::XMFLOAT3 scale;
    DirectX::XMStoreFloat3(&scale, m_Scale);
    scale.y = height;
    m_Scale = DirectX::XMLoadFloat3(&scale);
    m_RigidBody->ComputeInverseInertiaTensorCapsule(m_Radius, m_Height);
}

float CapsuleCollider::GetRadius() const
{
    float result = m_Radius;
    return result;
}

float CapsuleCollider::GetHeight() const
{
    float result = m_Height;
    return result;
}

void CapsuleCollider::SetScale(const DirectX::XMVECTOR& vector)
{
    using namespace DirectX;

    XMFLOAT3 scaleVec;
    XMStoreFloat3(&scaleVec, vector);

    float avgRadius = (scaleVec.x + scaleVec.z) * 0.5f * 0.5f;
    float height = scaleVec.y;

    m_Scale = vector;
    m_Radius = avgRadius;
    m_Height = height;
    m_RigidBody->ComputeInverseInertiaTensorCapsule(m_Radius, m_Height);
}

DirectX::XMVECTOR CapsuleCollider::GetScale() const
{
    DirectX::XMVECTOR scale = m_Scale;
    return scale;
}

bool CapsuleCollider::CheckCollisionWithCapsule(ICollider* other, Contact& outContact)
{
    using namespace DirectX;

    if (!other || other->GetColliderType() != ColliderType::Capsule)
        return false;

    CapsuleCollider* capsuleB = other->As<CapsuleCollider>();
    if (!capsuleB) return false;

    RigidBody* bodyA = this->GetRigidBody();
    RigidBody* bodyB = capsuleB->GetRigidBody();
    if (!bodyA || !bodyB) return false;

    const float radiusA = this->GetRadius();
    const float heightA = this->GetHeight();
    const float radiusB = capsuleB->GetRadius();
    const float heightB = capsuleB->GetHeight();

    const XMVECTOR posA = bodyA->GetPosition();
    const XMVECTOR posB = bodyB->GetPosition();

    const Quaternion qA = bodyA->GetOrientation();
    const Quaternion qB = bodyB->GetOrientation();

    const XMVECTOR upA = qA.RotateVector(XMVectorSet(0, 1, 0, 0));
    const XMVECTOR upB = qB.RotateVector(XMVectorSet(0, 1, 0, 0));

    // === Define capsule center segments (between sphere cap centers) ===
    const float halfA = 0.5f * (heightA - 2.0f * radiusA);
    const float halfB = 0.5f * (heightB - 2.0f * radiusB);

    XMVECTOR a1 = posA - upA * halfA;
    XMVECTOR a2 = posA + upA * halfA;

    XMVECTOR b1 = posB - upB * halfB;
    XMVECTOR b2 = posB + upB * halfB;

    // === Find closest points between segments ===
    XMVECTOR p1, p2;
    float s = 0.0f, t = 0.0f;
    ClosestPtSegmentSegment(a1, a2, b1, b2, p1, p2, s, t);

    XMVECTOR delta = p1 - p2;
    float distSq = XMVectorGetX(XMVector3LengthSq(delta));
    float combinedRadius = radiusA + radiusB;

    constexpr float epsilon = 1e-6f;
    constexpr float allowedSlack = 0.001f;

    if (distSq > (combinedRadius + allowedSlack) * (combinedRadius + allowedSlack))
        return false;

    float d = distSq > epsilon ? distSq : epsilon;
    float dist = std::sqrt(d);
    XMVECTOR normal = (dist > epsilon) ? XMVector3Normalize(delta) : XMVectorSet(1, 0, 0, 0);
    XMVECTOR contactPoint = 0.5f * (p1 + p2);

    outContact.Colliders[0] = this;
    outContact.Colliders[1] = other;
    XMStoreFloat3(&outContact.ContactPoint, contactPoint);
    XMStoreFloat3(&outContact.ContactNormal, normal);
    outContact.PenetrationDepth = combinedRadius - dist;

    outContact.Restitution = 0.5f * (bodyA->GetRestitution() + bodyB->GetRestitution());
    outContact.Friction = 0.5f * (bodyA->GetFriction() + bodyB->GetFriction());
    outContact.Elasticity = 0.5f * (bodyA->GetElasticity() + bodyB->GetElasticity());

    return true;
}

bool CapsuleCollider::CheckCollisionWithSphere(ICollider* other, Contact& outContact)
{
    using namespace DirectX;

    if (!other || other->GetColliderType() != ColliderType::Sphere)
        return false;

    SphereCollider* sphere = other->As<SphereCollider>();
    if (!sphere) return false;

    RigidBody* bodyA = GetRigidBody();    // Capsule
    RigidBody* bodyB = sphere->GetRigidBody(); // Sphere

    if (!bodyA || !bodyB) return false;

    XMVECTOR capsuleCenter = bodyA->GetPosition();
    XMVECTOR sphereCenter = bodyB->GetPosition();

    const Quaternion q = bodyA->GetOrientation();
    XMVECTOR up = q.RotateVector(XMVectorSet(0, 1, 0, 0));

    float capsuleHeight = GetHeight();
    float capsuleRadius = GetRadius();

    float sphereRadius = sphere->GetRadius();

    float halfSegmentLen = 0.5f * (capsuleHeight - 2.0f * capsuleRadius);

    // Capsule segment endpoints
    XMVECTOR a = capsuleCenter + up * halfSegmentLen;
    XMVECTOR b = capsuleCenter - up * halfSegmentLen;

    // Find closest point on capsule segment to sphere center
    XMVECTOR ab = b - a;
    XMVECTOR ap = sphereCenter - a;

    float denom = XMVectorGetX(XMVector3Dot(ab, ab));
    float t = 0.0f;

    if (denom > 1e-6f) // small epsilon to avoid divide-by-zero
    {
        t = std::clamp(
            XMVectorGetX(XMVector3Dot(ap, ab)) / denom,
            0.0f, 1.0f
        );
    }
    else
    {
        t = 0.0f; // default to start of segment when degenerate
    }

    XMVECTOR closest = a + ab * t;

    XMVECTOR diff = sphereCenter - closest;
    float distSq = XMVectorGetX(XMVector3LengthSq(diff));
    float totalRadius = capsuleRadius + sphereRadius;

    if (distSq > totalRadius * totalRadius)
        return false; // No collision

    float distance = std::sqrt(distSq);
    XMVECTOR normal = (distance > 1e-6f) ? XMVector3Normalize(diff) : XMVectorSet(1, 0, 0, 0);

    outContact.Colliders[0] = this;
    outContact.Colliders[1] = other;
    XMStoreFloat3(&outContact.ContactPoint, closest);
    XMStoreFloat3(&outContact.ContactNormal, normal);
    outContact.PenetrationDepth = totalRadius - distance;

    outContact.Restitution = 0.5f * (
        bodyA->GetRestitution() + bodyB->GetRestitution());

    outContact.Friction = 0.5f * (
        bodyA->GetFriction() + bodyB->GetFriction());

    outContact.Elasticity = 0.5f * (
        bodyA->GetElasticity() + bodyB->GetElasticity());

    return true;
}

bool CapsuleCollider::CheckCollisionWithCube(ICollider* other, Contact& outContact)
{
    using namespace DirectX;

    if (!other || other->GetColliderType() != ColliderType::Cube)
        return false;

    CubeCollider* cube = other->As<CubeCollider>();
    if (!cube) return false;

    RigidBody* capsuleBody = GetRigidBody();
    RigidBody* cubeBody = cube->GetRigidBody();
    if (!capsuleBody || !cubeBody) return false;

    const XMVECTOR capsulePos = capsuleBody->GetPosition();
    const Quaternion capsuleRot = capsuleBody->GetOrientation();
    const XMVECTOR up = capsuleRot.RotateVector(XMVectorSet(0, 1, 0, 0));

    const float capsuleHeight = GetHeight();
    const float capsuleRadius = GetRadius();
    const float halfSegment = 0.5f * (capsuleHeight - 2.0f * capsuleRadius);

    // Define capsule segment (between sphere centers)
    XMVECTOR a = capsulePos + up * halfSegment;
    XMVECTOR b = capsulePos - up * halfSegment;

    // OBB box data
    const XMVECTOR boxPos = cubeBody->GetPosition();
    const Quaternion boxRot = cubeBody->GetOrientation();
    const XMVECTOR halfExtents = cube->GetHalfExtents();

    XMVECTOR axes[3];
    cube->GetOBBAxes(boxRot, axes);

    XMVECTOR closestCapsulePt, closestBoxPt;
    ClosestPtSegmentOBB(a, b, boxPos, axes, halfExtents, closestCapsulePt, closestBoxPt);

    XMVECTOR diff = closestCapsulePt - closestBoxPt;
    float distSq = XMVectorGetX(XMVector3LengthSq(diff));

    // === Collision Slop Threshold ===
    constexpr float collisionSlop = 0.01f;
    float er = capsuleRadius - collisionSlop;
    float effectiveRadius = er > 0.001f ? er : 0.001f;

    if (distSq > effectiveRadius * effectiveRadius)
        return false;

    float distance = std::sqrt(distSq);
    XMVECTOR normal = (distance > 1e-6f) ? XMVector3Normalize(diff) : XMVectorSet(1, 0, 0, 0);

    outContact.Colliders[0] = this;
    outContact.Colliders[1] = other;
    XMStoreFloat3(&outContact.ContactPoint, closestBoxPt);
    XMStoreFloat3(&outContact.ContactNormal, normal);
    outContact.PenetrationDepth = capsuleRadius - distance;

    outContact.Restitution = 0.5f * (capsuleBody->GetRestitution() + cubeBody->GetRestitution());
    outContact.Friction = 0.5f * (capsuleBody->GetFriction() + cubeBody->GetFriction());
    outContact.Elasticity = 0.5f * (capsuleBody->GetElasticity() + cubeBody->GetElasticity());

    return true;
}

float CapsuleCollider::ClosestPtSegmentSegment(const DirectX::XMVECTOR& p1, const DirectX::XMVECTOR& p2,
    const DirectX::XMVECTOR& q1, const DirectX::XMVECTOR& q2, DirectX::XMVECTOR& outPt1, DirectX::XMVECTOR& outPt2,
    float& outS, float& outT)
{
    const DirectX::XMVECTOR d1 = DirectX::XMVectorSubtract(p2, p1); // Direction vector of segment S1
    const DirectX::XMVECTOR d2 = DirectX::XMVectorSubtract(q2, q1); // Direction vector of segment S2
    const DirectX::XMVECTOR r = DirectX::XMVectorSubtract(p1, q1); // Vector between segment starts


    const float a = DirectX::XMVectorGetX(DirectX::XMVector3Dot(d1, d1));  // Squared length of segment S1
    const float e = DirectX::XMVectorGetX(DirectX::XMVector3Dot(d2, d2));  // Squared length of segment S2
    const float f = DirectX::XMVectorGetX(DirectX::XMVector3Dot(d2, r));

    float s, t;

    if (a <= 1e-6f && e <= 1e-6f)
    {
        // Both segments degenerate into points
        outPt1 = p1;
        outPt2 = q1;
        outS = outT = 0.0f;
        return DirectX::XMVectorGetX(
            DirectX::XMVector3LengthSq(
                DirectX::XMVectorSubtract(p1, q1)
            )
        );
    }

    if (a <= 1e-6f)
    {
        // First segment is a point
        s = 0.0f;
        t = std::clamp(f / e, 0.0f, 1.0f);
    }
    else
    {
        const float c = DirectX::XMVectorGetX(DirectX::XMVector3Dot(d1, r));
        if (e <= 1e-6f)
        {
            // Second segment is a point
            t = 0.0f;
            s = std::clamp(-c / a, 0.0f, 1.0f);
        }
        else
        {
            const float b = DirectX::XMVectorGetX(DirectX::XMVector3Dot(d1, d2));
            const float denom = a * e - b * b;

            if (denom != 0.0f)
            {
                s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
            }
            else
            {
                s = 0.0f;
            }

            t = (b * s + f) / e;

            if (t < 0.0f)
            {
                t = 0.0f;
                s = std::clamp(-c / a, 0.0f, 1.0f);
            }
            else if (t > 1.0f)
            {
                t = 1.0f;
                s = std::clamp((b - c) / a, 0.0f, 1.0f);
            }
        }
    }

    outPt1 = DirectX::XMVectorAdd(p1, DirectX::XMVectorScale(d1, s));
    outPt2 = DirectX::XMVectorAdd(q1, DirectX::XMVectorScale(d2, t));
    outS = s;
    outT = t;

    return DirectX::XMVectorGetX(
        DirectX::XMVector3LengthSq(
            DirectX::XMVectorSubtract(outPt1, outPt2)
        )
    );
}

void CapsuleCollider::ClosestPtSegmentOBB(const DirectX::XMVECTOR& segStart, const DirectX::XMVECTOR& segEnd, const DirectX::XMVECTOR& boxCenter, const DirectX::XMVECTOR axes[3], const DirectX::XMVECTOR& halfExtents, DirectX::XMVECTOR& outCapsulePt, DirectX::XMVECTOR& outBoxPt)
{
    using namespace DirectX;

    const int steps = 10;
    float closestDistSq = FLT_MAX;

    for (int i = 0; i <= steps; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        XMVECTOR p = XMVectorLerp(segStart, segEnd, t);

        // Project capsule point `p` into OBB space
        XMVECTOR d = p - boxCenter;
        XMVECTOR q = boxCenter;

        for (int j = 0; j < 3; ++j)
        {
            float dist = XMVectorGetX(XMVector3Dot(d, axes[j]));

            XMFLOAT3 half;
            XMStoreFloat3(&half, halfExtents);
            float extents[3] = { half.x, half.y, half.z };
            float clamped = std::clamp(dist, -extents[j], extents[j]);
            q += axes[j] * clamped;
        }

        XMVECTOR diff = p - q;
        float distSq = XMVectorGetX(XMVector3LengthSq(diff));

        if (distSq < closestDistSq)
        {
            closestDistSq = distSq;
            outCapsulePt = p;
            outBoxPt = q;
        }
    }
}