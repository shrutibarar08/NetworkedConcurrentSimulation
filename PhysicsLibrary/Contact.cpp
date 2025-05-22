#include "pch.h"
#include "Contact.h"

#include <algorithm>

void Contact::Resolve(float duration)
{
    ResolveVelocity(duration);
    ResolveInterpenetration(duration);
}

float Contact::CalculateSeparatingVelocity() const
{
    using namespace DirectX;

    XMVECTOR velocityA = Body[0]->GetVelocity();
    XMVECTOR velocityB = Body[1] ? Body[1]->GetVelocity() : XMVectorZero();

	XMVECTOR relativeVelocity = XMVectorSubtract(velocityA, velocityB);

	XMVECTOR contactNormal = XMLoadFloat3(&ContactNormal);
	XMVECTOR separatingVelocityVec = XMVector3Dot(relativeVelocity, contactNormal);
    return XMVectorGetX(separatingVelocityVec);
}

void Contact::ResolveVelocity(float duration)
{
    using namespace DirectX;

    XMVECTOR contactNormal = XMLoadFloat3(&ContactNormal);

    // Calculate separating velocity
    float separatingVel = CalculateSeparatingVelocity();
    if (separatingVel > 0.0f)
        return; // No action required if separating

    // Calculate new separating velocity after restitution
    float newSepVel = -separatingVel * Restitution;
    float deltaVel = newSepVel - separatingVel;

    // Calculate total inverse mass
    float totalInverseMass = Body[0]->GetInverseMass();
    if (Body[1])
        totalInverseMass += Body[1]->GetInverseMass();

    // If total inverse mass is zero, bodies are immovable
    if (totalInverseMass <= 0.0f)
        return;

    // Calculate impulse scalar
    float impulse = deltaVel / totalInverseMass;

    // Calculate impulse per unit inverse mass
    XMVECTOR impulsePerIMass = XMVectorScale(contactNormal, impulse);

    // Apply impulse to the first body
    XMVECTOR velocityA = Body[0]->GetVelocity();
    velocityA = XMVectorAdd(velocityA, XMVectorScale(impulsePerIMass, Body[0]->GetInverseMass()));
    Body[0]->SetVelocity(velocityA);

    // Apply impulse to the second body, if it exists
    if (Body[1])
    {
        XMVECTOR velocityB = Body[1]->GetVelocity();
        velocityB = XMVectorSubtract(velocityB, XMVectorScale(impulsePerIMass, Body[1]->GetInverseMass()));
        Body[1]->SetVelocity(velocityB);
    }
}

void Contact::ResolveInterpenetration(float duration) const
{
    using namespace DirectX;

    // Load contact normal into XMVECTOR
    XMVECTOR contactNormal = XMLoadFloat3(&ContactNormal);

    // Calculate separating velocity
    float separatingVel = CalculateSeparatingVelocity();
    if (separatingVel > 0.0f)
        return; // No action required if separating

    // Calculate new separating velocity after restitution
    float newSepVel = -separatingVel * Restitution;
    float deltaVel = newSepVel - separatingVel;

    // Calculate total inverse mass
    float totalInverseMass = Body[0]->GetInverseMass();
    if (Body[1])
        totalInverseMass += Body[1]->GetInverseMass();

    // If total inverse mass is zero, bodies are immovable
    if (totalInverseMass <= 0.0f)
        return;

    // Calculate impulse scalar
    float impulseScalar = deltaVel / totalInverseMass;

    // Calculate impulse per unit inverse mass
    XMVECTOR impulsePerIMass = XMVectorScale(contactNormal, impulseScalar);

    // Apply impulse to the first body
    XMVECTOR velocityA = Body[0]->GetVelocity();
    velocityA = XMVectorAdd(velocityA, XMVectorScale(impulsePerIMass, Body[0]->GetInverseMass()));
    Body[0]->SetVelocity(velocityA);

    // Apply impulse to the second body, if it exists
    XMVECTOR velocityB;
    XMFLOAT3 newVelocityB;
    if (Body[1])
    {
    	velocityB = Body[1]->GetVelocity();
        velocityB = XMVectorSubtract(velocityB, XMVectorScale(impulsePerIMass, Body[1]->GetInverseMass()));
        Body[1]->SetVelocity(velocityB);
    }

    // Compute relative velocity
    XMVECTOR relativeVelocity = Body[0]->GetVelocity();

	if (Body[1])
    {
        velocityB = Body[1]->GetVelocity();
        relativeVelocity = XMVectorSubtract(relativeVelocity, velocityB);
    }

    // Compute lateral velocity component
    XMVECTOR normalComponent = XMVectorScale(contactNormal, XMVectorGetX(XMVector3Dot(relativeVelocity, contactNormal)));
    XMVECTOR lateralVelocity = XMVectorSubtract(relativeVelocity, normalComponent);

    // Check if lateral velocity is near zero
    if (XMVector3LengthSq(lateralVelocity).m128_f32[0] < 1e-6f)
        return;

    // Normalize lateral direction
    XMVECTOR lateralDir = XMVector3Normalize(lateralVelocity);

    // Calculate friction impulse magnitude
    float tangentialImpulseMag = -XMVectorGetX(XMVector3Dot(relativeVelocity, lateralDir)) / totalInverseMass;

    // Coulomb's law
    float maxFrictionImpulse = Friction * impulseScalar;
    tangentialImpulseMag = std::clamp(tangentialImpulseMag, -maxFrictionImpulse, maxFrictionImpulse);

    // Apply friction impulse
    XMVECTOR frictionImpulse = XMVectorScale(lateralDir, tangentialImpulseMag);

    // Apply to the first body
    velocityA = Body[0]->GetVelocity();
    velocityA = XMVectorAdd(velocityA, XMVectorScale(frictionImpulse, Body[0]->GetInverseMass()));
    Body[0]->SetVelocity(velocityA);

    // Apply to the second body, if it exists
    if (Body[1])
    {
        velocityB = Body[1]->GetVelocity();
        velocityB = XMVectorSubtract(velocityB, XMVectorScale(frictionImpulse, Body[1]->GetInverseMass()));
        Body[1]->SetVelocity(velocityB);
    }
}
