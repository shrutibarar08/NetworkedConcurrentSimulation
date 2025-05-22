#include "pch.h"
#include "SphereCollider.h"

#include <cmath>
#include <algorithm>

#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "PlaneCollider.h"

SphereCollider::SphereCollider(RigidBody* body, float radius)
    : Collider(body), Radius(radius)
{}

float SphereCollider::GetRadius() const
{
    return Radius;
}

DirectX::XMVECTOR SphereCollider::GetCenter() const
{
    return m_RigidBody->GetPosition(); // Assumes RigidBody has GetPosition()
}

Collider::Type SphereCollider::GetType() const
{
    return Type::SPHERE;
}

bool SphereCollider::CheckCollisionWithBox(Collider* other, Contact& contact) const
{
    BoxCollider* box = dynamic_cast<BoxCollider*>(other);
    if (!box) return false;

    DirectX::XMVECTOR boxCenter = box->GetCenter();
    DirectX::XMVECTOR boxHalfExtents = box->GetHalfExtents();
    DirectX::XMVECTOR sphereCenter = GetCenter();

    // Compute the closest point on the box to the sphere center
    DirectX::XMVECTOR minCorner = DirectX::XMVectorSubtract(boxCenter, boxHalfExtents);
    DirectX::XMVECTOR maxCorner = DirectX::XMVectorAdd(boxCenter, boxHalfExtents);

    DirectX::XMVECTOR closestPoint = DirectX::XMVectorClamp(sphereCenter, minCorner, maxCorner);

    // Vector from closest point to sphere center
    DirectX::XMVECTOR delta = DirectX::XMVectorSubtract(sphereCenter, closestPoint);
    float distanceSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(delta));

    if (distanceSq > Radius * Radius)
        return false;

    float distance = std::sqrt(distanceSq);
    float penetration = Radius - distance;

    // Normalize delta to get contact normal
    DirectX::XMVECTOR normal = (distance > 1e-6f)
        ? DirectX::XMVectorScale(delta, 1.0f / distance)
        : DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // Arbitrary normal if sphere center is inside box

    // Fill in the contact
    DirectX::XMStoreFloat3(&contact.ContactPoint, closestPoint);
    DirectX::XMStoreFloat3(&contact.ContactNormal, normal);
    contact.Penetration = penetration;

    return true;
}

bool SphereCollider::CheckCollisionWithSphere(Collider* other, Contact& contact) const
{
    SphereCollider* otherSphere = dynamic_cast<SphereCollider*>(other);
    if (!otherSphere) return false;

    DirectX::XMVECTOR centerA = GetCenter();
    DirectX::XMVECTOR centerB = otherSphere->GetCenter();

    float radiusA = GetRadius();
    float radiusB = otherSphere->GetRadius();

    DirectX::XMVECTOR delta = DirectX::XMVectorSubtract(centerB, centerA);
    float distanceSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(delta));
    float radiusSum = radiusA + radiusB;

    if (distanceSq > radiusSum * radiusSum)
        return false;

    float distance = std::sqrt(distanceSq);
    float penetration = radiusSum - distance;

    DirectX::XMVECTOR normal = (distance > 1e-6f)
        ? DirectX::XMVectorScale(delta, 1.0f / distance)
        : DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // arbitrary direction if perfectly overlapping

    DirectX::XMVECTOR contactPoint = DirectX::XMVectorAdd(
        centerA,
        DirectX::XMVectorScale(normal, radiusA - penetration * 0.5f)
    );

    DirectX::XMStoreFloat3(&contact.ContactPoint, contactPoint);
    DirectX::XMStoreFloat3(&contact.ContactNormal, normal);
    contact.Penetration = penetration;

    return true;
}

bool SphereCollider::CheckCollisionWithPlane(Collider* other, Contact& contact) const
{
    PlaneCollider* plane = dynamic_cast<PlaneCollider*>(other);
    if (!plane) return false;

    DirectX::XMVECTOR sphereCenter = GetCenter();
    DirectX::XMVECTOR planeNormal = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&plane->GetNormal()));
    float planeOffset = plane->GetOffset();
    float radius = GetRadius();

    // Compute signed distance from sphere center to plane
    float distanceToPlane = DirectX::XMVectorGetX(DirectX::XMVector3Dot(sphereCenter, planeNormal)) - planeOffset;

    // If outside the sphere radius, no collision
    if (fabs(distanceToPlane) > radius)
        return false;

    // Compute penetration depth
    float penetration = radius - fabs(distanceToPlane);

    // Contact normal points away from the plane
    DirectX::XMVECTOR contactNormal = (distanceToPlane < 0.0f)
        ? DirectX::XMVectorNegate(planeNormal)
        : planeNormal;

    // Contact point is sphere surface point touching the plane
    DirectX::XMVECTOR contactPoint = DirectX::XMVectorSubtract(
        sphereCenter,
        DirectX::XMVectorScale(planeNormal, distanceToPlane)
    );

    DirectX::XMStoreFloat3(&contact.ContactPoint, contactPoint);
    DirectX::XMStoreFloat3(&contact.ContactNormal, contactNormal);
    contact.Penetration = penetration;

    return true;
}

bool SphereCollider::CheckCollisionWithCapsule(Collider* other, Contact& contact) const
{
    CapsuleCollider* capsule = dynamic_cast<CapsuleCollider*>(other);
    if (!capsule) return false;

    DirectX::XMVECTOR sphereCenter = GetCenter();
    float sphereRadius = GetRadius();

    DirectX::XMVECTOR capsuleStart = capsule->GetStart();
    DirectX::XMVECTOR capsuleEnd = capsule->GetEnd();
    float capsuleRadius = capsule->GetRadius();

    // === Closest point on capsule segment to sphere center ===
    DirectX::XMVECTOR capsuleSegment = DirectX::XMVectorSubtract(capsuleEnd, capsuleStart);
    DirectX::XMVECTOR toCenter = DirectX::XMVectorSubtract(sphereCenter, capsuleStart);

    float segLenSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(capsuleSegment));
    float t = (segLenSq > 1e-6f) ?
        DirectX::XMVectorGetX(DirectX::XMVector3Dot(toCenter, capsuleSegment)) / segLenSq :
        0.0f;

    t = std::clamp(t, 0.0f, 1.0f);
    DirectX::XMVECTOR closestPoint = DirectX::XMVectorAdd(capsuleStart, DirectX::XMVectorScale(capsuleSegment, t));

    // === Sphere-sphere collision test ===
    DirectX::XMVECTOR delta = DirectX::XMVectorSubtract(sphereCenter, closestPoint);
    float distSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(delta));
    float combinedRadius = sphereRadius + capsuleRadius;

    if (distSq > combinedRadius * combinedRadius)
        return false;

    float distance = std::sqrt(distSq);
    float penetration = combinedRadius - distance;

    // Compute contact normal
    DirectX::XMVECTOR normal = (distance > 1e-6f)
        ? DirectX::XMVectorScale(delta, 1.0f / distance)
        : DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // arbitrary up if centers are same

    // Contact point on sphere surface
    DirectX::XMVECTOR contactPoint = DirectX::XMVectorSubtract(
        sphereCenter,
        DirectX::XMVectorScale(normal, sphereRadius - penetration * 0.5f)
    );

    DirectX::XMStoreFloat3(&contact.ContactPoint, contactPoint);
    DirectX::XMStoreFloat3(&contact.ContactNormal, normal);
    contact.Penetration = penetration;

    return true;
}
