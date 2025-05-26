#include "pch.h"
#include "CollisionResolver.h"

#include <algorithm>
#include <random>

void CollisionResolver::ResolveContact(Contact& contact, float deltaTime)
{
    ICollider* a = contact.Colliders[0];
    ICollider* b = contact.Colliders[1];
    if (!a || !b) return;

    ResolvePositionInterpenetration(contact);
    if (IsStatic(a) && IsStatic(b)) return;
    ResolveVelocity(contact, deltaTime);
}

void CollisionResolver::ResolveContacts(std::vector<Contact>& contacts, float deltaTime)
{
    for (Contact& contact : contacts)
    {
        ResolveContact(contact, deltaTime);
    }
}

void CollisionResolver::ResolvePositionInterpenetration(const Contact& contact)
{
    using namespace DirectX;

    ICollider* a = contact.Colliders[0];
    ICollider* b = contact.Colliders[1];
    if (!a || !b || contact.PenetrationDepth <= 0.0f) return;

    RigidBody* bodyA = a->GetRigidBody();
    RigidBody* bodyB = b->GetRigidBody();
    if (!bodyA || !bodyB) return;

    bool isStaticA = IsStatic(a);
    bool isStaticB = IsStatic(b);

    float invMassA = bodyA->GetInverseMass();
    float invMassB = bodyB->GetInverseMass();
    float totalInvMass = invMassA + invMassB;
    if (totalInvMass <= 0.0f) return;

    XMVECTOR normal = XMLoadFloat3(&contact.ContactNormal);
    float penetration = contact.PenetrationDepth;

    // === Correct normal direction to match bodyA->bodyB ===
    XMVECTOR posA = bodyA->GetPosition();
    XMVECTOR posB = bodyB->GetPosition();
    XMVECTOR dirAB = posB - posA;
    if (XMVectorGetX(XMVector3Dot(normal, dirAB)) < 0.0f)
        normal = -normal;

    // === Shape-specific handling ===
    ColliderType typeA = a->GetColliderType();
    ColliderType typeB = b->GetColliderType();

    bool isCapsuleInvolved = (typeA == ColliderType::Capsule || typeB == ColliderType::Capsule);
    bool isCapsuleCapsule = (typeA == ColliderType::Capsule && typeB == ColliderType::Capsule);

    // === Penetration correction params ===
    constexpr float slop = 0.01f;      // Overlap allowed before correction
    constexpr float percent = 1.0f;    // Resolve full overlap to prevent sticking

    float cd = penetration - slop;
    float correctionDepth = cd > 0.0f ? cd : 0.0f;
    XMVECTOR correction = normal * (correctionDepth * percent / totalInvMass);

    // Optional: abort if correction is effectively nothing (numerical jitter)
    if (XMVectorGetX(XMVector3LengthSq(correction)) < 1e-8f)
        return;

    // Mass-weighted biasing
    float weightA = invMassA / totalInvMass;
    float weightB = invMassB / totalInvMass;

    // Slightly reduce push if both are capsules (avoids infinite shoving)
    float capsuleBias = isCapsuleCapsule ? 0.85f : 1.0f;

    if (!isStaticA && invMassA > 0.0f)
    {
        XMVECTOR pos = bodyA->GetPosition();
        bodyA->SetPosition(pos - correction * weightA * capsuleBias);
    }

    if (!isStaticB && invMassB > 0.0f)
    {
        XMVECTOR pos = bodyB->GetPosition();
        bodyB->SetPosition(pos + correction * weightB * capsuleBias);
    }

    // === kill small linear velocity after deep overlap correction ===
    auto DampSmallVelocity = [](RigidBody* body) {
        XMVECTOR v = body->GetVelocity();
        if (XMVectorGetX(XMVector3LengthSq(v)) < 1e-4f)
            body->SetVelocity(XMVectorZero());
        };

    if (!isStaticA) DampSmallVelocity(bodyA);
    if (!isStaticB) DampSmallVelocity(bodyB);
}

void CollisionResolver::ResolveVelocity(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    RigidBody* bodyA = contact.Colliders[0]->GetRigidBody();
    RigidBody* bodyB = contact.Colliders[1]->GetRigidBody();
    if (!bodyA || !bodyB) return;

    bool aStatic = IsStatic(contact.Colliders[0]);
    bool bStatic = IsStatic(contact.Colliders[1]);

    float invMassA = bodyA->GetInverseMass();
    float invMassB = bodyB->GetInverseMass();
    if (invMassA + invMassB <= 0.0f) return;

    XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&contact.ContactNormal));
    XMVECTOR point = XMLoadFloat3(&contact.ContactPoint);

    XMVECTOR rA = point - bodyA->GetPosition();
    XMVECTOR rB = point - bodyB->GetPosition();

    XMVECTOR vA = GetVelocityAtPoint(bodyA, rA);
    XMVECTOR vB = GetVelocityAtPoint(bodyB, rB);
    XMVECTOR relativeVel = vA - vB;

    float sepVel = XMVectorGetX(XMVector3Dot(relativeVel, normal));
    //if (sepVel > 0.0f) return; // Already separating

    // Use restitution and elasticity together
    float restitution = contact.Restitution;
    float elasticity = contact.Elasticity;
    float combinedRestitution = restitution * (1.0f + elasticity);

    XMMATRIX invInertiaA = bodyA->GetInverseInertiaTensor();
    XMMATRIX invInertiaB = bodyB->GetInverseInertiaTensor();

    if (bStatic)
    {
        ReflectFromStatic(bodyA, normal, sepVel, combinedRestitution, rA, invInertiaA);
        ApplyFriction(contact, relativeVel, -normal * sepVel);
    }
    else if (aStatic)
    {
        ReflectFromStatic(bodyB, -normal, sepVel, combinedRestitution, rB, invInertiaB);
        ApplyFriction(contact, relativeVel, normal * sepVel);
    }
    else
    {
        ResolveDynamicVsDynamic(bodyA, bodyB, normal, sepVel, combinedRestitution, rA, rB, invInertiaA, invInertiaB);
        ApplyFriction(contact, relativeVel, normal * sepVel);
    }

    // === Damping ===
    float linearDampingA = bodyA->GetDamping();
    float linearDampingB = bodyB->GetDamping();
    float angularDampingA = bodyA->GetAngularDamping();
    float angularDampingB = bodyB->GetAngularDamping();

    // Linear damping reduces velocity over time
    if (!aStatic)
    {
        bodyA->SetVelocity(bodyA->GetVelocity() * std::pow(1.0f - linearDampingA, deltaTime));
        bodyA->SetAngularVelocity(bodyA->GetAngularVelocity() * std::pow(1.0f - angularDampingA, deltaTime));
    }

    if (!bStatic)
    {
        bodyB->SetVelocity(bodyB->GetVelocity() * std::pow(1.0f - linearDampingB, deltaTime));
        bodyB->SetAngularVelocity(bodyB->GetAngularVelocity() * std::pow(1.0f - angularDampingB, deltaTime));
    }
}

void CollisionResolver::ApplyFriction(Contact& contact, DirectX::XMVECTOR relativeVelocity, DirectX::XMVECTOR impulse)
{
    using namespace DirectX;

    RigidBody* bodyA = contact.Colliders[0]->GetRigidBody();
    RigidBody* bodyB = contact.Colliders[1]->GetRigidBody();
    if (!bodyA || !bodyB) return;

    float invMassA = bodyA->GetInverseMass();
    float invMassB = bodyB->GetInverseMass();
    if (invMassA + invMassB <= 0.0f) return;

    bool isStaticA = IsStatic(contact.Colliders[0]);
    bool isStaticB = IsStatic(contact.Colliders[1]);

    XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&contact.ContactNormal));
    XMVECTOR contactPoint = XMLoadFloat3(&contact.ContactPoint);

    // Get the tangential velocity
    XMVECTOR velTangent = relativeVelocity - normal * XMVector3Dot(relativeVelocity, normal);
    if (XMVector3LengthSq(velTangent).m128_f32[0] < 1e-6f) return;

    XMVECTOR tangent = XMVector3Normalize(velTangent);

    float friction = contact.Friction;
    float normalImpulseMag = XMVectorGetX(XMVector3Length(impulse));
    float maxFrictionImpulseMag = friction * normalImpulseMag;

    float tangentialVelMag = XMVectorGetX(XMVector3Dot(relativeVelocity, tangent));
    float denom = invMassA + invMassB;
    float frictionImpulseMag = -tangentialVelMag / denom;
    frictionImpulseMag = std::clamp(frictionImpulseMag, -maxFrictionImpulseMag, maxFrictionImpulseMag);

    XMVECTOR frictionImpulse = tangent * frictionImpulseMag;

    // === Linear friction ===
    if (!isStaticA && invMassA > 0.0f)
    {
        XMVECTOR velA = bodyA->GetVelocity();
        XMVECTOR tangentVel = tangent * XMVectorGetX(XMVector3Dot(velA, tangent));
        XMVECTOR damped = tangentVel * -friction;
        bodyA->SetVelocity(velA + damped);
    }

    if (!isStaticB && invMassB > 0.0f)
    {
        XMVECTOR velB = bodyB->GetVelocity();
        XMVECTOR tangentVel = tangent * XMVectorGetX(XMVector3Dot(velB, tangent));
        XMVECTOR damped = tangentVel * -friction;
        bodyB->SetVelocity(velB + damped);
    }

    // === Angular friction ===
    XMVECTOR rA = contactPoint - bodyA->GetPosition();
    XMVECTOR rB = contactPoint - bodyB->GetPosition();

    XMMATRIX invInertiaA = bodyA->GetInverseInertiaTensor();
    XMMATRIX invInertiaB = bodyB->GetInverseInertiaTensor();

    if (!isStaticA && invMassA > 0.0f)
    {
        XMVECTOR angImpulseA = XMVector3Transform(XMVector3Cross(rA, frictionImpulse), invInertiaA);
        XMVECTOR angVelA = bodyA->GetAngularVelocity();

        // Optional damping factor
        float damping = std::clamp(5.0f * invMassA, 0.1f, 1.0f);
        angImpulseA *= damping;

        bodyA->SetAngularVelocity(angVelA + angImpulseA);

        if (XMVectorGetX(XMVector3LengthSq(angVelA)) < 0.01f)
        {
            float noiseStrength = normalImpulseMag * 0.05f;
            XMVECTOR randomTorque = GenerateRandomAngularNoise(noiseStrength);
            bodyA->SetAngularVelocity(bodyA->GetAngularVelocity() + randomTorque);
        }
    }

    if (!isStaticB && invMassB > 0.0f)
    {
        XMVECTOR angImpulseB = XMVector3Transform(XMVector3Cross(rB, -frictionImpulse), invInertiaB);
        XMVECTOR angVelB = bodyB->GetAngularVelocity();

        float damping = std::clamp(5.0f * invMassB, 0.1f, 1.0f);
        angImpulseB *= damping;

        bodyB->SetAngularVelocity(angVelB + angImpulseB);

        if (XMVectorGetX(XMVector3LengthSq(angVelB)) < 0.01f)
        {
            float noiseStrength = normalImpulseMag * 0.05f;
            XMVECTOR randomTorque = GenerateRandomAngularNoise(noiseStrength);
            bodyB->SetAngularVelocity(bodyB->GetAngularVelocity() + randomTorque);
        }
    }
}

bool CollisionResolver::IsStatic(ICollider* collider)
{
    return collider->GetColliderState() == ColliderSate::Static;
}

DirectX::XMVECTOR CollisionResolver::GetVelocityAtPoint(RigidBody* body, const DirectX::XMVECTOR& r)
{
    using namespace DirectX;

	XMVECTOR linearVel = body->GetVelocity();
	XMVECTOR angVel = body->GetAngularVelocity();

	return linearVel + angVel;
}

float CollisionResolver::ComputeDenominator(float invMassA, float invMassB, const DirectX::XMVECTOR& rA,
	const DirectX::XMVECTOR& rB, const DirectX::XMVECTOR& normal, const DirectX::XMMATRIX& invInertiaA,
	const DirectX::XMMATRIX& invInertiaB)
{
    using namespace DirectX;

    XMVECTOR rA_cross_n = XMVector3Cross(rA, normal);
    XMVECTOR rB_cross_n = XMVector3Cross(rB, normal);

    XMVECTOR angularA = XMVector3Cross(XMVector3Transform(rA_cross_n, invInertiaA), rA);
    XMVECTOR angularB = XMVector3Cross(XMVector3Transform(rB_cross_n, invInertiaB), rB);

    float angularEffect = XMVectorGetX(XMVector3Dot(angularA + angularB, normal));
    return invMassA + invMassB + angularEffect;
}

void CollisionResolver::ApplyImpulse(
    RigidBody* body,
    const DirectX::XMVECTOR& impulse,
    const DirectX::XMVECTOR& r,
    const DirectX::XMMATRIX& invInertia)
{
    using namespace DirectX;

    float invMass = body->GetInverseMass();
    if (invMass <= 0.0f) return;

    // === LINEAR IMPULSE ===
    body->SetVelocity(body->GetVelocity() + impulse * invMass);

    // === ANGULAR IMPULSE ===
    XMVECTOR angularImpulse = XMVector3Transform(XMVector3Cross(r, impulse), invInertia);
    XMVECTOR newAngVel = body->GetAngularVelocity() + angularImpulse;

    // === WEIGHT-BASED DAMPING LOGIC ===
    float mass = invMass > 0.0f ? (1.0f / invMass) : FLT_MAX;

    // More mass => lower damping effect. Less mass => more damping.
    float baseDamping = body->GetAngularDamping(); // [0..1], set per body
    float dampingFactor = std::clamp(baseDamping * (1.0f / mass), 0.0f, 1.0f);

    // Apply angular damping by scaling angular velocity (simulate resistance)
    newAngVel *= (1.0f - dampingFactor);

    body->SetAngularVelocity(newAngVel);
}

void CollisionResolver::ReflectFromStatic(
    RigidBody* body,
    const DirectX::XMVECTOR& normal,
    float sepVel,
    float restitution,
    const DirectX::XMVECTOR& r,
    const DirectX::XMMATRIX& invInertia)
{
    using namespace DirectX;

    // === LINEAR REFLECTION ===
    float bounceVel = -(1.0f + restitution) * sepVel;
    XMVECTOR reflected = normal * bounceVel;

    XMVECTOR oldVel = body->GetVelocity();
    XMVECTOR newVel = oldVel + reflected;
    body->SetVelocity(newVel);

    // === ANGULAR RESPONSE ===
    float invMass = body->GetInverseMass();
    if (invMass <= 0.0f) return;

    float mass = 1.0f / invMass;
    float baseDamping = body->GetAngularDamping(); // user-defined [0..1]
    float dampingFactor = std::clamp(baseDamping * (1.0f / mass), 0.0f, 1.0f);

    // Calculate angular impulse
    XMVECTOR angularImpulse = XMVector3Transform(XMVector3Cross(r, reflected), invInertia);
    XMVECTOR newAngVel = body->GetAngularVelocity() + angularImpulse;

    // Apply angular damping
    newAngVel *= (1.0f - dampingFactor);
    body->SetAngularVelocity(newAngVel);
}

void CollisionResolver::ResolveDynamicVsDynamic(
    RigidBody* bodyA, RigidBody* bodyB,
    const DirectX::XMVECTOR& normal,
    float sepVel, float restitution,
    const DirectX::XMVECTOR& rA, const DirectX::XMVECTOR& rB,
    const DirectX::XMMATRIX& invInertiaA,
    const DirectX::XMMATRIX& invInertiaB)
{
    using namespace DirectX;

    float invMassA = bodyA->GetInverseMass();
    float invMassB = bodyB->GetInverseMass();

    // === Include elasticity factor (optional) ===
    float elasticity = 0.5f * (bodyA->GetElasticity() + bodyB->GetElasticity());
    float effectiveRestitution = restitution * (1.0f + elasticity); // Boost bounciness slightly

    // === Compute impulse magnitude ===
    float denom = ComputeDenominator(invMassA, invMassB, rA, rB, normal, invInertiaA, invInertiaB);
    if (denom <= 0.0f) return;

    float impulseMag = -effectiveRestitution * sepVel / denom;
    XMVECTOR impulse = normal * impulseMag;

    // === Apply to both bodies (linear + angular) ===
    ApplyImpulse(bodyA, impulse, rA, invInertiaA);
    ApplyImpulse(bodyB, -impulse, rB, invInertiaB);
}

DirectX::XMVECTOR CollisionResolver::GenerateRandomAngularNoise(float strength)
{
    static std::default_random_engine rng(std::random_device{}());
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float x = dist(rng);
    float y = dist(rng);
    float z = dist(rng);

    using namespace DirectX;
    XMVECTOR dir = XMVectorSet(x, y, z, 0.0f);
    dir = XMVector3Normalize(dir);

    return dir * strength;
}
