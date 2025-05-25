#include "pch.h"
#include "CollisionResolver.h"

#include <algorithm>
#include <random>

void CollisionResolver::ResolveContact(Contact& contact, float deltaTime)
{
    ICollider* a = contact.Colliders[0];
    ICollider* b = contact.Colliders[1];
    if (!a || !b) return;

    // === Currently supporting only CubeCollider ===
    if (a->GetColliderType() != ColliderType::Cube ||
        b->GetColliderType() != ColliderType::Cube)
        return;

    // === Always resolve interpenetration first ===
    ResolveCubeInterPenetration(contact);

    // Skip only if both are static (immovable)
    if (IsStatic(a) && IsStatic(b))
        return;

    ResolveCubeVelocity(contact, deltaTime);
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

    const float bounceThreshold = 0.1f;
    float restitution = contact.Restitution;

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
    // if (sepVel > -bounceThreshold) return;

    XMMATRIX invInertiaA = bodyA->GetInverseInertiaTensor();
    XMMATRIX invInertiaB = bodyB->GetInverseInertiaTensor();


    if (bStatic)
    {
        ReflectFromStatic(bodyA, normal, sepVel, restitution, rA, invInertiaA);
        ApplyCubeFriction(contact, relativeVel, -normal * sepVel);
    }
    else if (aStatic)
    {
        ReflectFromStatic(bodyB, -normal, sepVel, restitution, rB, invInertiaB);
       ApplyCubeFriction(contact, relativeVel, normal * sepVel);
    }
    else
    {
        ResolveDynamicVsDynamic(bodyA, bodyB, normal, sepVel, restitution, rA, rB, invInertiaA, invInertiaB);
        ApplyCubeFriction(contact, relativeVel, normal * sepVel);
    }
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

    bool isStaticA = IsStatic(contact.Colliders[0]);
    bool isStaticB = IsStatic(contact.Colliders[1]);

    XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&contact.ContactNormal));
    XMVECTOR contactPoint = XMLoadFloat3(&contact.ContactPoint);

    // Tangential (sliding) component
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

    // --- Linear Friction ---
    if (!isStaticA && invMassA > 0.0f)
    {
        XMVECTOR vel = bodyA->GetVelocity();
        XMVECTOR tangentComponent = tangent * XMVector3Dot(vel, tangent);
        XMVECTOR damped = tangentComponent * -friction;
        if (XMVectorGetX(XMVector3LengthSq(tangentComponent)) > 1e-6f)
        {
            bodyA->SetVelocity(vel + damped);
        }
    }

    if (!isStaticB && invMassB > 0.0f)
    {
        XMVECTOR vel = bodyB->GetVelocity();
        XMVECTOR tangentComponent = tangent * XMVector3Dot(vel, tangent);
        XMVECTOR damped = tangentComponent * -friction;
        if (XMVectorGetX(XMVector3LengthSq(tangentComponent)) > 1e-6f)
        {
            bodyB->SetVelocity(vel + damped);
        }
    }

    // --- Angular Friction + Random Torque ---
    XMVECTOR rA = contactPoint - bodyA->GetPosition();
    XMVECTOR rB = contactPoint - bodyB->GetPosition();

    XMMATRIX invInertiaA = bodyA->GetInverseInertiaTensor();
    XMMATRIX invInertiaB = bodyB->GetInverseInertiaTensor();

    if (!isStaticA && invMassA > 0.0f)
    {
        XMVECTOR angImpulseA = XMVector3Transform(XMVector3Cross(rA, frictionImpulse), invInertiaA);
        float dampingA = std::clamp(5.0f * invMassA, 0.1f, 1.0f);
        angImpulseA *= dampingA;

        XMVECTOR angVelA = bodyA->GetAngularVelocity();
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
        float dampingB = std::clamp(5.0f * invMassB, 0.1f, 1.0f);
        angImpulseB *= dampingB;

        XMVECTOR angVelB = bodyB->GetAngularVelocity();
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
    return DirectX::XMVectorAdd(body->GetVelocity(), r);
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

    // Apply linear impulse
    body->SetVelocity(body->GetVelocity() + impulse * invMass);

    // Compute and apply angular impulse
    XMVECTOR angularImpulse = XMVector3Transform(XMVector3Cross(r, impulse), invInertia);

    // Dampen angular impulse based on inverse mass (heavy objects rotate less)
    float damping = std::clamp(5.0f * invMass, 0.1f, 1.0f); // Heuristic
    angularImpulse *= damping;

    body->SetAngularVelocity(body->GetAngularVelocity() + angularImpulse);
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

    float bounceVel = -(1.0f + restitution) * sepVel;
    XMVECTOR reflected = normal * bounceVel;

    XMVECTOR oldVel = body->GetVelocity();
    XMVECTOR newVel = oldVel + reflected;
    body->SetVelocity(newVel);

    float invMass = body->GetInverseMass();
    float massFactor = invMass > 0.0f ? invMass : 0.0f;

    // Apply scaled angular impulse
    XMVECTOR angularImpulse = XMVector3Transform(XMVector3Cross(r, reflected), invInertia);

    // Heuristic scaling based on inverse mass (heavier bodies spin less)
    float damping = std::clamp(5.0f * massFactor, 0.1f, 1.0f);
    angularImpulse = angularImpulse * damping;

    body->SetAngularVelocity(body->GetAngularVelocity() + angularImpulse);
}

void CollisionResolver::ResolveDynamicVsDynamic(RigidBody* bodyA, RigidBody* bodyB, const DirectX::XMVECTOR& normal,
	float sepVel, float restitution, const DirectX::XMVECTOR& rA, const DirectX::XMVECTOR& rB,
	const DirectX::XMMATRIX& invInertiaA, const DirectX::XMMATRIX& invInertiaB)
{
    using namespace DirectX;

    float invMassA = bodyA->GetInverseMass();
    float invMassB = bodyB->GetInverseMass();

    float denom = ComputeDenominator(invMassA, invMassB, rA, rB, normal, invInertiaA, invInertiaB);
    if (denom <= 0.0f) return;

    float impulseMag = -(1.0f + restitution) * sepVel / denom;
    XMVECTOR impulse = normal * impulseMag;

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
