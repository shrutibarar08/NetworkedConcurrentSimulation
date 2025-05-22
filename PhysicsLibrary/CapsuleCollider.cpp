#include "pch.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include "PlaneCollider.h"
#include <cmath>
#include <algorithm>

#include "BoxCollider.h"

CapsuleCollider::CapsuleCollider(RigidBody* body,
                                 float radius,
                                 float height)
    : Collider(body), Radius(radius), Height(height)
{}

Collider::Type CapsuleCollider::GetType() const {
    return Collider::Type::CAPSULE;
}

float CapsuleCollider::GetRadius() const
{
    return Radius;
}

float CapsuleCollider::GetHeight() const
{
    return Height;
}

DirectX::XMVECTOR CapsuleCollider::GetStart() const
{
	DirectX::XMVECTOR center = GetBody()->GetPosition();
	DirectX::XMVECTOR offset = DirectX::XMVectorSet(0.0f, Height * 0.5f, 0.0f, 0.0f);
	return DirectX::XMVectorAdd(center, offset);
}

DirectX::XMVECTOR CapsuleCollider::GetEnd() const
{
	DirectX::XMVECTOR center = GetBody()->GetPosition();
	DirectX::XMVECTOR offset = DirectX::XMVectorSet(0.0f, Height * 0.5f, 0.0f, 0.0f);
	return DirectX::XMVectorSubtract(center, offset);
}

bool CapsuleCollider::CheckCollisionWithBox(Collider* other, Contact& contact) const
{
    using namespace DirectX;

    // Cast to BoxCollider
    BoxCollider* box = dynamic_cast<BoxCollider*>(other);

    XMVECTOR boxCenter = box->GetCenter();
    XMVECTOR boxHalfExtents = box->GetHalfExtents();

    // Capsule segment points
    XMVECTOR capsuleStart = GetStart();
    XMVECTOR capsuleEnd = GetEnd();
    float capsuleRadius = GetRadius();

    // Clamp function for component-wise clamp between min and max
    auto Clamp = [](const XMVECTOR& value, const XMVECTOR& min, const XMVECTOR& max) -> XMVECTOR
	{
        return XMVectorClamp(value, min, max);
    };

    // Compute box min and max points
    XMVECTOR boxMin = XMVectorSubtract(boxCenter, boxHalfExtents);
    XMVECTOR boxMax = XMVectorAdd(boxCenter, boxHalfExtents);

    // Compute the closest point on capsule segment to box
    XMVECTOR segDir = XMVectorSubtract(capsuleEnd, capsuleStart);
    float segLengthSq = XMVectorGetX(XMVector3LengthSq(segDir));
	XMVECTOR startToBox = XMVectorSubtract(boxCenter, capsuleStart);

    // Project startToBox onto segment direction, clamped to [0,1]
    float t = 0.f;
    if (segLengthSq > 0.f)
    {
        t = XMVectorGetX(XMVector3Dot(startToBox, segDir)) / segLengthSq;
        t = std::clamp(t, 0.0f, 1.0f);
    }

    // Closest point on capsule segment to box center
    XMVECTOR closestPointOnSegment = XMVectorAdd(capsuleStart, XMVectorScale(segDir, t));
	XMVECTOR closestPointOnBox = Clamp(closestPointOnSegment, boxMin, boxMax);
	XMVECTOR diff = XMVectorSubtract(closestPointOnSegment, closestPointOnBox);
    float distSq = XMVectorGetX(XMVector3LengthSq(diff));

    // If squared distance less than squared radius, collision occurs
    if (distSq <= capsuleRadius * capsuleRadius)
    {
        float dist = sqrtf(distSq);
        XMVECTOR contactNormal;

    	if (dist > 0.0001f) contactNormal = XMVectorScale(diff, 1.0f / dist);
        else contactNormal = XMVectorSet(0, 1, 0, 0);

        contact.Body[0] = GetBody();
        contact.Body[1] = box->GetBody();
        DirectX::XMStoreFloat3(&contact.ContactNormal, contactNormal);
        DirectX::XMStoreFloat3(&contact.ContactPoint, closestPointOnBox);
        contact.Penetration = capsuleRadius - dist;
        contact.Restitution = 0.5f;
        contact.Friction = 0.3f;

        return true;
    }
    return false;
}

bool CapsuleCollider::CheckCollisionWithSphere(Collider* other, Contact& contact) const
{
    using namespace DirectX;

    SphereCollider* sphere = dynamic_cast<SphereCollider*>(other);

    XMVECTOR capsuleStart = GetStart();
    XMVECTOR capsuleEnd = GetEnd();
    XMVECTOR sphereCenter = sphere->GetCenter();

    XMVECTOR ab = XMVectorSubtract(capsuleEnd, capsuleStart);
    XMVECTOR ac = XMVectorSubtract(sphereCenter, capsuleStart);

    // Dot products
    float abDotAb = XMVectorGetX(XMVector3Dot(ab, ab));
    float acDotAb = XMVectorGetX(XMVector3Dot(ac, ab));
	float t = std::max(0.0f, std::min(1.0f, acDotAb / abDotAb));

    // closest point on capsule segment to sphere center
    XMVECTOR closestPoint = XMVectorAdd(capsuleStart, XMVectorScale(ab, t));
	XMVECTOR diff = XMVectorSubtract(sphereCenter, closestPoint);

    // Squared distance between closest point and sphere center
    float distanceSq = XMVectorGetX(XMVector3LengthSq(diff));

    float radiusSum = Radius + sphere->GetRadius();

    if (distanceSq <= radiusSum * radiusSum)
    {
        float distance = std::sqrt(distanceSq);

        // Normalize diff for contact Normal (handle zero distance)
        XMVECTOR contactNormal = distance > 0.0f ? XMVectorScale(diff, 1.0f / distance) : XMVectorSet(0, 1, 0, 0);
    	XMVECTOR contactPoint = XMVectorAdd(closestPoint, XMVectorScale(contactNormal, Radius));

        contact.Body[0] = GetBody();
        contact.Body[1] = sphere->GetBody();

        XMStoreFloat3(&contact.ContactNormal, contactNormal);
        contact.Penetration = radiusSum - distance;
        XMStoreFloat3(&contact.ContactPoint, contactPoint);
        contact.Restitution = 0.5f;
        contact.Friction = 0.3f;

        return true;
    }

    return false;
}

bool CapsuleCollider::CheckCollisionWithPlane(Collider* other, Contact& contact) const
{
    using namespace DirectX;

    PlaneCollider* plane = dynamic_cast<PlaneCollider*>(other);

	XMVECTOR capsuleStart = GetStart();
    XMVECTOR capsuleEnd = GetEnd();
	XMVECTOR planeNormal = XMLoadFloat3(&plane->GetNormal());
    float planeOffset = plane->GetOffset();

    // Calculate signed distances from capsule ends to plane
    float dStart = XMVectorGetX(XMVector3Dot(planeNormal, capsuleStart)) - planeOffset;
    float dEnd = XMVectorGetX(XMVector3Dot(planeNormal, capsuleEnd)) - planeOffset;

    // Check if either end is within capsule radius of plane
    if (dStart <= Radius || dEnd <= Radius)
    {
        contact.Body[0] = GetBody();
        contact.Body[1] = plane->GetBody();
        contact.ContactNormal = plane->GetNormal();
    	contact.Penetration = Radius - std::min(dStart, dEnd);
        DirectX::XMVECTOR contactPoint = (dStart < dEnd) ? capsuleStart : capsuleEnd;
        DirectX::XMStoreFloat3(&contact.ContactPoint, contactPoint);
        contact.Restitution = 0.5f;
        contact.Friction = 0.3f;

        return true;
    }
    return false;
}

bool CapsuleCollider::CheckCollisionWithCapsule(Collider* other, Contact& contact) const
{
    using namespace DirectX;

    CapsuleCollider* capsule = dynamic_cast<CapsuleCollider*>(other);

    XMVECTOR p1 = GetStart();
    XMVECTOR q1 = GetEnd();
    float r1 = GetRadius();

    XMVECTOR p2 = capsule->GetStart();
    XMVECTOR q2 = capsule->GetEnd();
    float r2 = capsule->GetRadius();

    // Vector from p1 to q1
    XMVECTOR d1 = XMVectorSubtract(q1, p1);
    XMVECTOR d2 = XMVectorSubtract(q2, p2);
	XMVECTOR r = XMVectorSubtract(p1, p2);

    float a = XMVectorGetX(XMVector3Dot(d1, d1)); // Squared length of segment 1
    float e = XMVectorGetX(XMVector3Dot(d2, d2)); // Squared length of segment 2
    float f = XMVectorGetX(XMVector3Dot(d2, r));

    float s, t;

    // If either segment is degenerate (length ~ 0)
    if (a <= EPSILON && e <= EPSILON)
    {
        // Both segments degenerate to points
        s = t = 0.0f;
    }
    else if (a <= EPSILON)
    {
        // First segment degenerate to point
        s = 0.0f;
        t = std::clamp(f / e, 0.0f, 1.0f);
    }
    else if (e <= EPSILON)
    {
        // Second segment degenerate to point
        t = 0.0f;
        s = std::clamp(-XMVectorGetX(XMVector3Dot(d1, r)) / a, 0.0f, 1.0f);
    }
    else
    {
        float c = XMVectorGetX(XMVector3Dot(d1, r));
        float b = XMVectorGetX(XMVector3Dot(d1, d2));
        float denom = a * e - b * b;

        if (denom != 0.0f)
            s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
        else
            s = 0.0f;

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
    // Closest points
    XMVECTOR c1 = XMVectorAdd(p1, XMVectorScale(d1, s));
    XMVECTOR c2 = XMVectorAdd(p2, XMVectorScale(d2, t));
	XMVECTOR diff = XMVectorSubtract(c1, c2);

	float distSq = XMVectorGetX(XMVector3LengthSq(diff));
    float radiusSum = r1 + r2;

    if (distSq <= radiusSum * radiusSum)
    {
        float dist = sqrtf(distSq);
        XMVECTOR normal;

        if (dist > 1e-6f) normal = XMVectorScale(diff, 1.0f / dist);
        else normal = XMVectorSet(1, 0, 0, 0);

        contact.Body[0] = GetBody();
        contact.Body[1] = capsule->GetBody();
        contact.Penetration = radiusSum - dist;
        DirectX::XMStoreFloat3(&contact.ContactNormal, normal);
        DirectX::XMVECTOR contactPoint = DirectX::XMVectorAdd(c2, DirectX::XMVectorScale(normal, r2));
        DirectX::XMStoreFloat3(&contact.ContactPoint, contactPoint);
        contact.Restitution = 0.5f;
        contact.Friction = 0.3f;

        return true;
    }
    return false;
}
