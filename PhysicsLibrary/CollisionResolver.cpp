#include "pch.h"
#include "CollisionResolver.h"

#include <algorithm>

void CollisionResolver::ResolveContact(Contact& contact, float deltaTime)
{
    ICollider* a = contact.Colliders[0];
    ICollider* b = contact.Colliders[1];
    if (!a || !b) return;

    // Only supporting cube for now
    if (a->GetColliderType() == ColliderType::Cube &&
        b->GetColliderType() == ColliderType::Cube)
    {
        // Always resolve penetration
        ResolveCubeInterPenetration(contact);

        // Only apply velocity impulse if at least one is not static
        if (a->GetColliderState() != ColliderSate::Static ||
            b->GetColliderState() != ColliderSate::Static)
        {
            ResolveCubeVelocity(contact, deltaTime);
        }
    }
}

void CollisionResolver::ResolveContacts(std::vector<Contact>& contacts, float deltaTime)
{
    for (Contact& contact : contacts)
    {
        ResolveContact(contact, deltaTime);
    }
}

void CollisionResolver::ResolveCubeInterPenetration(Contact& contact)
{
    using namespace DirectX;

    ICollider* a = contact.Colliders[0];
    ICollider* b = contact.Colliders[1];

    RigidBody* bodyA = a->GetRigidBody();
    RigidBody* bodyB = b->GetRigidBody();

    if (!bodyA || !bodyB || contact.PenetrationDepth <= 0.0f)
        return;

    XMVECTOR normal = XMLoadFloat3(&contact.ContactNormal);
    float penetration = contact.PenetrationDepth;

    bool isStaticA = a->GetColliderState() == ColliderSate::Static;
    bool isStaticB = b->GetColliderState() == ColliderSate::Static;

    float totalInvMass = bodyA->GetInverseMass() + bodyB->GetInverseMass();
    if (totalInvMass == 0.0f) return;

    // Position correction proportion
    float percent = 0.99f; // 99% of correction applied (prevents overshoot)
    float slop = 0.01f;   // Ignore tiny penetrations (prevents jitter)

    float correctionDepth = (penetration - slop) > 0.0f? penetration - slop: 0.0f;
    XMVECTOR correction = normal * (correctionDepth * percent / totalInvMass);

    if (!isStaticA)
    {
        XMVECTOR posA = bodyA->GetPosition();
        bodyA->SetPosition(posA - correction * bodyA->GetInverseMass());
    }

    if (!isStaticB)
    {
        XMVECTOR posB = bodyB->GetPosition();
        bodyB->SetPosition(posB + correction * bodyB->GetInverseMass());
    }
}

void CollisionResolver::ResolveCubeVelocity(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    RigidBody* bodyA = contact.Colliders[0]->GetRigidBody();
    RigidBody* bodyB = contact.Colliders[1]->GetRigidBody();

    if (!bodyA || !bodyB) return;

    // Load normal and contact point
    XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&contact.ContactNormal));
    XMVECTOR point = XMLoadFloat3(&contact.ContactPoint);

    XMVECTOR velA = bodyA->GetVelocity();
    XMVECTOR velB = bodyB->GetVelocity();

    XMVECTOR angVelA = bodyA->GetAngularVelocity();
    XMVECTOR angVelB = bodyB->GetAngularVelocity();

    XMVECTOR rA = point - bodyA->GetPosition();
    XMVECTOR rB = point - bodyB->GetPosition();

    // Linear + angular velocity at contact
    XMVECTOR vA = velA + XMVector3Cross(angVelA, rA);
    XMVECTOR vB = velB + XMVector3Cross(angVelB, rB);
    XMVECTOR relativeVelocity = vA - vB;

    float sepVel = XMVectorGetX(XMVector3Dot(relativeVelocity, normal));

    // Do not resolve if separating or resting
    if (sepVel > 0.0f) return;

    float invMassA = bodyA->GetInverseMass();
    float invMassB = bodyB->GetInverseMass();

    XMMATRIX invInertiaA = bodyA->GetInverseInertiaTensor();
    XMMATRIX invInertiaB = bodyB->GetInverseInertiaTensor();

    // Compute impulse scalar
    XMVECTOR rA_cross_n = XMVector3Cross(rA, normal);
    XMVECTOR rB_cross_n = XMVector3Cross(rB, normal);

    XMVECTOR angTermA = XMVector3Cross(XMVector3Transform(rA_cross_n, invInertiaA), rA);
    XMVECTOR angTermB = XMVector3Cross(XMVector3Transform(rB_cross_n, invInertiaB), rB);

    float angularEffect = XMVectorGetX(XMVector3Dot(angTermA + angTermB, normal));
    float denom = invMassA + invMassB + angularEffect;

    if (denom <= 0.0f) return;

    float restitution = contact.Restitution;
    float impulseScalar = -(1.0f + restitution) * sepVel / denom;

    XMVECTOR impulse = normal * impulseScalar;

    // Apply linear impulse
    if (invMassA > 0.0f)
        bodyA->SetVelocity(velA + impulse * invMassA);

    if (invMassB > 0.0f)
        bodyB->SetVelocity(velB - impulse * invMassB);

    // Apply angular impulse
    if (invMassA > 0.0f)
    {
        XMVECTOR angularImpulseA = XMVector3Transform(XMVector3Cross(rA, impulse), invInertiaA);
        bodyA->SetAngularVelocity(angVelA + angularImpulseA);
    }

    if (invMassB > 0.0f)
    {
        XMVECTOR angularImpulseB = XMVector3Transform(XMVector3Cross(rB, -impulse), invInertiaB);
        bodyB->SetAngularVelocity(angVelB + angularImpulseB);
    }
	ApplyCubeFriction(contact, relativeVelocity, impulse);
}

void CollisionResolver::ApplyCubeFriction(Contact& contact, DirectX::XMVECTOR relativeVelocity, DirectX::XMVECTOR impulse)
{
    using namespace DirectX;

    RigidBody* bodyA = contact.Colliders[0]->GetRigidBody();
    RigidBody* bodyB = contact.Colliders[1]->GetRigidBody();
    if (!bodyA || !bodyB) return;

    float invMassA = bodyA->GetInverseMass();
    float invMassB = bodyB->GetInverseMass();
    if (invMassA + invMassB <= 0.0f) return;

    XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&contact.ContactNormal));
    XMVECTOR contactPoint = XMLoadFloat3(&contact.ContactPoint);

    // Remove normal component to get tangential velocity
    XMVECTOR velTangent = relativeVelocity - normal * XMVector3Dot(relativeVelocity, normal);
    if (XMVector3LengthSq(velTangent).m128_f32[0] < 1e-6f) return; // No tangential motion

    XMVECTOR tangent = XMVector3Normalize(velTangent);

    // Calculate tangential impulse magnitude
    float friction = contact.Friction;
    float normalImpulseMag = XMVectorGetX(XMVector3Length(impulse));
    float maxFrictionImpulseMag = friction * normalImpulseMag;

    // Estimate desired friction impulse
    float tangentialVelMag = XMVectorGetX(XMVector3Dot(relativeVelocity, tangent));
    float denom = invMassA + invMassB;

    float frictionImpulseMag = -tangentialVelMag / denom;
    frictionImpulseMag = std::clamp(frictionImpulseMag, -maxFrictionImpulseMag, maxFrictionImpulseMag);

    XMVECTOR frictionImpulse = tangent * frictionImpulseMag;

    // Apply linear friction
    if (invMassA > 0.0f)
    {
        XMVECTOR vel = bodyA->GetVelocity();
        bodyA->SetVelocity(vel + frictionImpulse * invMassA);
    }

    if (invMassB > 0.0f)
    {
        XMVECTOR vel = bodyB->GetVelocity();
        bodyB->SetVelocity(vel - frictionImpulse * invMassB);
    }

    // Apply angular friction
    XMVECTOR rA = contactPoint - bodyA->GetPosition();
    XMVECTOR rB = contactPoint - bodyB->GetPosition();

    XMMATRIX invInertiaA = bodyA->GetInverseInertiaTensor();
    XMMATRIX invInertiaB = bodyB->GetInverseInertiaTensor();

    if (invMassA > 0.0f)
    {
        XMVECTOR angImpulseA = XMVector3Transform(XMVector3Cross(rA, frictionImpulse), invInertiaA);
        XMVECTOR angVel = bodyA->GetAngularVelocity();
        bodyA->SetAngularVelocity(angVel + angImpulseA);
    }

    if (invMassB > 0.0f)
    {
        XMVECTOR angImpulseB = XMVector3Transform(XMVector3Cross(rB, -frictionImpulse), invInertiaB);
        XMVECTOR angVel = bodyB->GetAngularVelocity();
        bodyB->SetAngularVelocity(angVel + angImpulseB);
    }
}
