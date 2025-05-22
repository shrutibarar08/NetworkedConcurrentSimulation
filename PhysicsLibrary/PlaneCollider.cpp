#include "pch.h"
#include "PlaneCollider.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "Contact.h"

#include <cmath>
#include <algorithm>

#include "CapsuleCollider.h"

PlaneCollider::PlaneCollider(const DirectX::XMFLOAT3& normal,
                             float offset,
                             RigidBody* attachedBody)
    : Normal(normal), Offset(offset), Collider(attachedBody)
{}

Collider::Type PlaneCollider::GetType() const
{
    return Type::PLANE;
}

const DirectX::XMFLOAT3& PlaneCollider::GetNormal() const
{
    return Normal;
}

float PlaneCollider::GetOffset() const
{
    return Offset;
}

bool PlaneCollider::CheckCollisionWithBox(Collider* other, Contact& contact) const
{
    BoxCollider* box = dynamic_cast<BoxCollider*>(other);
    if (!box) return false;

    // Retrieve the box's center and half extents
    DirectX::XMVECTOR boxCenter = box->GetCenter();
    DirectX::XMVECTOR halfExtents = box->GetHalfExtents();

    DirectX::XMVECTOR planeNormal = XMLoadFloat3(&Normal);
    float planeOffset = Offset;

    // Compute the distance from the box center to the plane
    float centerToPlane = DirectX::XMVectorGetX(DirectX::XMVector3Dot(planeNormal, boxCenter)) - planeOffset;

    // Compute the projection radius of the box onto the plane normal
    DirectX::XMVECTOR absPlaneNormal = DirectX::XMVectorAbs(planeNormal);
    float projectedRadius = DirectX::XMVectorGetX(DirectX::XMVector3Dot(absPlaneNormal, halfExtents));

    // Check for intersection
    if (std::abs(centerToPlane) <= projectedRadius)
    {
        // Determine the contact normal direction
        DirectX::XMVECTOR contactNormal = (centerToPlane < 0.0f) ? DirectX::XMVectorNegate(planeNormal) : planeNormal;

        // Calculate penetration depth
        float penetration = projectedRadius - std::abs(centerToPlane);

        DirectX::XMVECTOR contactPoint = DirectX::XMVectorSubtract(boxCenter, DirectX::XMVectorScale(planeNormal, centerToPlane));

        XMStoreFloat3(&contact.ContactNormal, contactNormal);
        XMStoreFloat3(&contact.ContactPoint, contactPoint);
        contact.Penetration = penetration;
        contact.Restitution = 0.5f;
        contact.Friction = 0.3f;

        return true;
    }

    return false;
}

bool PlaneCollider::CheckCollisionWithSphere(Collider* other, Contact& contact) const
{
    SphereCollider* sphere = dynamic_cast<SphereCollider*>(other);
    if (!sphere) return false;

    const DirectX::XMVECTOR& center = sphere->GetCenter();
    DirectX::XMVECTOR normal = XMLoadFloat3(&Normal);

    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Dot(normal, center)) - Offset;

    if (distance < sphere->GetRadius())
    {
        contact.Body[0] = sphere->GetBody();
        contact.Body[1] = nullptr; // Static plane

    	XMStoreFloat3(&contact.ContactNormal, normal);
    	contact.Penetration = sphere->GetRadius() - distance;

    	DirectX::XMVECTOR contactPoint = DirectX::XMVectorSubtract(center, DirectX::XMVectorScale(normal, distance));
        XMStoreFloat3(&contact.ContactPoint, contactPoint);
        contact.Restitution = 0.5f;
        contact.Friction = 0.3f;
        return true;
    }
    return false;
}

bool PlaneCollider::CheckCollisionWithPlane(Collider* other, Contact& contact) const
{
    PlaneCollider* otherPlane = dynamic_cast<PlaneCollider*>(other);
    if (!otherPlane)
        return false;

    // Get both plane normals and offsets
    DirectX::XMVECTOR n1 = DirectX::XMLoadFloat3(&Normal);
    DirectX::XMVECTOR n2 = DirectX::XMLoadFloat3(&otherPlane->Normal);
    float d1 = Offset;
    float d2 = otherPlane->Offset;

    // Normalize both normals (in case they aren't already)
    n1 = DirectX::XMVector3Normalize(n1);
    n2 = DirectX::XMVector3Normalize(n2);

    // Compute dot product to check if normals are parallel
    float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(n1, n2));

    if (fabs(dot - 1.0f) < 1e-5f || fabs(dot + 1.0f) < 1e-5f)
    {
        // Planes are parallel
        // Check if they are coincident (i.e., the same plane)
        // Pick any point on first plane: point = n1 * d1
        DirectX::XMVECTOR pointOnPlane1 = DirectX::XMVectorScale(n1, d1);
        float dist = DirectX::XMVectorGetX(DirectX::XMVector3Dot(n2, pointOnPlane1)) - d2;

        if (fabs(dist) < 1e-4f)
        {
            // Planes are coincident
            DirectX::XMStoreFloat3(&contact.ContactPoint, pointOnPlane1);
        	contact.ContactNormal = Normal;
            contact.Penetration = 0.0f;
            return true;
        }
        else
        {
            // Planes are parallel but not coincident - no collision
            return false;
        }
    }

    // If not parallel, planes intersect in a line (infinite line of intersection)
    // Compute line of intersection (direction and point)
    DirectX::XMVECTOR lineDir = DirectX::XMVector3Cross(n1, n2); // direction of the line
    lineDir = DirectX::XMVector3Normalize(lineDir);

    // Construct system of equations to solve for intersection point
    // Solve using formula from plane-plane intersection theory
    DirectX::XMVECTOR temp = DirectX::XMVector3Cross(n2, lineDir);
    float denom = DirectX::XMVectorGetX(DirectX::XMVector3Dot(n1, temp));
    if (fabs(denom) < 1e-5f)
        return false; // Numerically unstable

    float t = (d2 - d1 * dot) / denom;
    DirectX::XMVECTOR pointOnLine = DirectX::XMVectorAdd(DirectX::XMVectorScale(lineDir, t), DirectX::XMVectorScale(n1, d1));

    // For simplicity, set contact point and normal arbitrarily
    DirectX::XMStoreFloat3(&contact.ContactPoint, pointOnLine);
    contact.ContactNormal = Normal;
    contact.Penetration = 0.0f; // There's no penetration as this is an infinite intersection

    return true;
}


bool PlaneCollider::CheckCollisionWithCapsule(Collider* other, Contact& contact) const
{
    CapsuleCollider* capsule = dynamic_cast<CapsuleCollider*>(other);
    if (!capsule) return false;

    // Load plane normal and offset
    DirectX::XMVECTOR planeNormal = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&Normal));
    float planeOffset = Offset;

    // Get capsule segment ends
    DirectX::XMVECTOR capsuleStart = capsule->GetStart();
    DirectX::XMVECTOR capsuleEnd = capsule->GetEnd();

    // Compute signed distances of each endpoint to the plane
    float dStart = DirectX::XMVectorGetX(DirectX::XMVector3Dot(capsuleStart, planeNormal)) - planeOffset;
    float dEnd = DirectX::XMVectorGetX(DirectX::XMVector3Dot(capsuleEnd, planeNormal)) - planeOffset;

    float radius = capsule->GetRadius();

    // No intersection if both points are on the same side and farther than radius
    if (dStart > radius && dEnd > radius) return false;
    if (dStart < -radius && dEnd < -radius) return false;

    // Find the point on the capsule segment closest to the plane
    // Linearly interpolate between start and end
    float t = dStart / (dStart - dEnd); // safe if dStart != dEnd
    t = std::clamp(t, 0.0f, 1.0f);
    DirectX::XMVECTOR closestPoint = DirectX::XMVectorLerp(capsuleStart, capsuleEnd, t);

    // Project the closest point onto the plane to get contact point
    float distanceToPlane = DirectX::XMVectorGetX(DirectX::XMVector3Dot(closestPoint, planeNormal)) - planeOffset;
    float penetration = radius - fabsf(distanceToPlane);

    if (penetration < 0.0f)
        return false;

    DirectX::XMVECTOR contactPoint = DirectX::XMVectorSubtract(closestPoint, DirectX::XMVectorScale(planeNormal, distanceToPlane));

    // Fill contact data
    DirectX::XMStoreFloat3(&contact.ContactPoint, contactPoint);

    DirectX::XMVECTOR finalNormal = (distanceToPlane > 0.0f)
        ? DirectX::XMVectorNegate(planeNormal)
        : planeNormal;

    DirectX::XMStoreFloat3(&contact.ContactNormal, finalNormal);
	contact.Penetration = penetration;

    return true;
}
