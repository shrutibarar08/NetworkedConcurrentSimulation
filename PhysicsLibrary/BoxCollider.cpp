#include "pch.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "PlaneCollider.h"
#include "CapsuleCollider.h"

#include <algorithm>
#include <cmath>

BoxCollider::BoxCollider
(
    RigidBody* attachedBody,
    const DirectX::XMVECTOR& halfExtents
)
    : Collider(attachedBody), HalfExtents(halfExtents)
{}

Collider::Type BoxCollider::GetType() const
{
    return Type::BOX;
}

const DirectX::XMVECTOR& BoxCollider::GetHalfExtents() const
{
    return HalfExtents;
}

const DirectX::XMVECTOR& BoxCollider::GetCenter() const
{
    return m_RigidBody->GetPosition();
}

bool BoxCollider::CheckCollisionWithBox(Collider* other, Contact& contact) const
{
    BoxCollider* box = dynamic_cast<BoxCollider*>(other);

    // Calculate min and max points for both boxes
    DirectX::XMVECTOR aCenter = GetCenter();
    DirectX::XMVECTOR bCenter = box->GetCenter();

    DirectX::XMVECTOR aHalfExtents = HalfExtents;
    DirectX::XMVECTOR bHalfExtents = box->HalfExtents;

    DirectX::XMVECTOR aMin = DirectX::XMVectorSubtract(aCenter, aHalfExtents);
    DirectX::XMVECTOR aMax = DirectX::XMVectorAdd(aCenter, aHalfExtents);

    DirectX::XMVECTOR bMin = DirectX::XMVectorSubtract(bCenter, bHalfExtents);
    DirectX::XMVECTOR bMax = DirectX::XMVectorAdd(bCenter, bHalfExtents);

    // Check for overlap on all three axes
    bool xOverlap = DirectX::XMVectorGetX(aMax) >= DirectX::XMVectorGetX(bMin) && DirectX::XMVectorGetX(aMin) <= DirectX::XMVectorGetX(bMax);
    bool yOverlap = DirectX::XMVectorGetY(aMax) >= DirectX::XMVectorGetY(bMin) && DirectX::XMVectorGetY(aMin) <= DirectX::XMVectorGetY(bMax);
    bool zOverlap = DirectX::XMVectorGetZ(aMax) >= DirectX::XMVectorGetZ(bMin) && DirectX::XMVectorGetZ(aMin) <= DirectX::XMVectorGetZ(bMax);

    if (xOverlap && yOverlap && zOverlap)
    {
        // Compute overlap on each axis
        float xPenetration = std::min(DirectX::XMVectorGetX(aMax), DirectX::XMVectorGetX(bMax)) - std::max(DirectX::XMVectorGetX(aMin), DirectX::XMVectorGetX(bMin));
        float yPenetration = std::min(DirectX::XMVectorGetY(aMax), DirectX::XMVectorGetY(bMax)) - std::max(DirectX::XMVectorGetY(aMin), DirectX::XMVectorGetY(bMin));
        float zPenetration = std::min(DirectX::XMVectorGetZ(aMax), DirectX::XMVectorGetZ(bMax)) - std::max(DirectX::XMVectorGetZ(aMin), DirectX::XMVectorGetZ(bMin));

        // Find the axis of least penetration
        float minPenetration = xPenetration;
        DirectX::XMVECTOR normal = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

        if (yPenetration < minPenetration)
        {
            minPenetration = yPenetration;
            normal = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        }

        if (zPenetration < minPenetration)
        {
            minPenetration = zPenetration;
            normal = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        }

        // Determine the direction of the Normal
        DirectX::XMVECTOR direction = DirectX::XMVectorSubtract(bCenter, aCenter);
        if (DirectX::XMVector3Dot(direction, normal).m128_f32[0] < 0.0f)
        {
            normal = DirectX::XMVectorNegate(normal);
        }

        // Compute contact point as the midpoint between the centers
        DirectX::XMVECTOR contactPoint =
            DirectX::XMVectorScale(DirectX::XMVectorAdd(
                aCenter, bCenter), 0.5f);

        // Populate contact information
        contact.Body[0] = GetBody();
        contact.Body[1] = box->GetBody();
        contact.Penetration = minPenetration;
        DirectX::XMStoreFloat3(&contact.ContactPoint, contactPoint);
        DirectX::XMStoreFloat3(&contact.ContactNormal, normal);
        contact.Restitution = 0.5f;
        contact.Friction = 0.3f;

        return true;
    }

    return false;
}

bool BoxCollider::CheckCollisionWithSphere(Collider* other, Contact& contact) const
{
    SphereCollider* sphere = dynamic_cast<SphereCollider*>(other);
    if (!sphere) return false;

    DirectX::XMVECTOR boxCenter = GetCenter();
    DirectX::XMVECTOR boxHalfExtents = HalfExtents;
    DirectX::XMVECTOR sphereCenter = sphere->GetCenter();

    DirectX::XMVECTOR boxMin = DirectX::XMVectorSubtract(boxCenter, boxHalfExtents);
    DirectX::XMVECTOR boxMax = DirectX::XMVectorAdd(boxCenter, boxHalfExtents);

    // Clamp sphere center to the box
    DirectX::XMVECTOR closestPoint = DirectX::XMVectorClamp(sphereCenter, boxMin, boxMax);

    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(sphereCenter, closestPoint);
    DirectX::XMVECTOR distSqVec = DirectX::XMVector3LengthSq(diff);
    float distanceSq = DirectX::XMVectorGetX(distSqVec);
    float radius = sphere->GetRadius();

    if (distanceSq <= radius * radius)
    {
        float distance = std::sqrt(distanceSq);
        DirectX::XMVECTOR normal = DirectX::XMVector3Normalize(diff);

        contact.Body[0] = GetBody();
        contact.Body[1] = sphere->GetBody();
        contact.Penetration = radius - distance;
        DirectX::XMStoreFloat3(&contact.ContactPoint, closestPoint);
        DirectX::XMStoreFloat3(&contact.ContactNormal, normal);
        contact.Restitution = 0.5f;
        contact.Friction = 0.3f;

        return true;
    }

    return false;
}

bool BoxCollider::CheckCollisionWithPlane(Collider* other, Contact& contact) const
{
    PlaneCollider* plane = dynamic_cast<PlaneCollider*>(other);
    DirectX::XMVECTOR center = GetCenter();
    DirectX::XMFLOAT3 normal = plane->GetNormal();

    // Compute projection of half extents onto plane Normal
    float projection =
	    DirectX::XMVectorGetX(HalfExtents) * std::abs(normal.x) +
	    DirectX::XMVectorGetY(HalfExtents) * std::abs(normal.y) +
	    DirectX::XMVectorGetZ(HalfExtents) * std::abs(normal.z);

    DirectX::XMVECTOR normalVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&normal));
    float boxDistance = DirectX::XMVectorGetX(DirectX::XMVector3Dot(center, normalVec)) - plane->GetOffset();

    if (boxDistance <= projection)
    {
        contact.Body[0] = GetBody();
        contact.Body[1] = plane->GetBody();
        contact.ContactNormal = normal;
        contact.Penetration = projection - boxDistance;

    	DirectX::XMVECTOR normalVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&normal));
        DirectX::XMVECTOR contactPoint = DirectX::XMVectorSubtract(center, DirectX::XMVectorScale(normalVec, boxDistance));
    	DirectX::XMStoreFloat3(&contact.ContactPoint, contactPoint);        DirectX::XMStoreFloat3(&contact.ContactPoint, contactPoint);

    	contact.Restitution = 0.5f;
        contact.Friction = 0.3f;

        return true;
    }

    return false;
}

bool BoxCollider::CheckCollisionWithCapsule(Collider* other, Contact& contact) const
{
    CapsuleCollider* capsule = dynamic_cast<CapsuleCollider*>(other);

    // Load box data
    DirectX::XMVECTOR boxCenter = GetCenter();
    DirectX::XMVECTOR boxHalfExtents = HalfExtents;

    DirectX::XMVECTOR capsuleStart = capsule->GetStart();
    DirectX::XMVECTOR capsuleEnd = capsule->GetEnd();
    float capsuleRadius = capsule->GetRadius();

    auto Clamp = [](const DirectX::XMVECTOR& value, const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max) -> DirectX::XMVECTOR
	{
        return DirectX::XMVectorClamp(value, min, max);
	};

    // Find closest point on the capsule segment to the box (and vice versa)
    DirectX::XMVECTOR segDir = DirectX::XMVectorSubtract(capsuleEnd, capsuleStart);
    DirectX::XMVECTOR segLengthSqVec = DirectX::XMVector3LengthSq(segDir);
    float segLengthSq = DirectX::XMVectorGetX(segLengthSqVec);

    if (segLengthSq < 1e-6f)
    {
	    DirectX::XMVECTOR closestPointOnBox = Clamp(capsuleStart, DirectX::XMVectorSubtract(boxCenter, boxHalfExtents), DirectX::XMVectorAdd(boxCenter, boxHalfExtents));

	    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(capsuleStart, closestPointOnBox);
	    DirectX::XMVECTOR distSqVec = DirectX::XMVector3LengthSq(diff);
        float distSq = DirectX::XMVectorGetX(distSqVec);

        if (distSq <= capsuleRadius * capsuleRadius)
        {
            contact.Body[0] = GetBody();
            contact.Body[1] = capsule->GetBody();
            contact.Penetration = capsuleRadius - std::sqrt(distSq);
            DirectX::XMVECTOR normalVec = DirectX::XMVector3Normalize(diff);
            DirectX::XMStoreFloat3(&contact.ContactNormal, normalVec);
            DirectX::XMStoreFloat3(&contact.ContactPoint, closestPointOnBox);
            contact.Restitution = 0.5f;
            contact.Friction = 0.3f;
            return true;
        }
        return false;
    }

    // Normalize segment direction
    DirectX::XMVECTOR segDirNorm = DirectX::XMVectorScale(segDir, 1.0f / std::sqrt(segLengthSq));
    DirectX::XMVECTOR startToBox = DirectX::XMVectorSubtract(boxCenter, capsuleStart);

    float t = DirectX::XMVectorGetX(DirectX::XMVector3Dot(startToBox, segDirNorm));

    float segLength = std::sqrt(segLengthSq);
    t = std::clamp(t, 0.0f, segLength);

    DirectX::XMVECTOR closestPointOnSegment = DirectX::XMVectorAdd(capsuleStart, DirectX::XMVectorScale(segDirNorm, t));

    DirectX::XMVECTOR boxMin = DirectX::XMVectorSubtract(boxCenter, boxHalfExtents);
    DirectX::XMVECTOR boxMax = DirectX::XMVectorAdd(boxCenter, boxHalfExtents);
    DirectX::XMVECTOR closestPointOnBox = Clamp(closestPointOnSegment, boxMin, boxMax);

    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(closestPointOnSegment, closestPointOnBox);
    DirectX::XMVECTOR distSqVec = DirectX::XMVector3LengthSq(diff);
    float distSq = DirectX::XMVectorGetX(distSqVec);

    if (distSq <= capsuleRadius * capsuleRadius)
    {
        float dist = std::sqrt(distSq);
    	contact.Body[0] = GetBody();
        contact.Body[1] = capsule->GetBody();
        contact.Penetration = capsuleRadius - dist;

        if (dist > 1e-6f)
        {
            DirectX::XMVECTOR normalVec = DirectX::XMVector3Normalize(diff);
            DirectX::XMStoreFloat3(&contact.ContactNormal, normalVec);
        }
        else
        {
            DirectX::XMStoreFloat3(&contact.ContactNormal, DirectX::XMVectorSet(0, 1, 0, 0));
        }

        DirectX::XMStoreFloat3(&contact.ContactPoint, closestPointOnBox);

        contact.Restitution = 0.5f;
        contact.Friction = 0.3f;

        return true;
    }
	return false;
}
