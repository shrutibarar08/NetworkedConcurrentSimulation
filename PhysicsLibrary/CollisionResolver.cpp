#include "pch.h"
#include "CollisionResolver.h"

#include <algorithm>
#include <random>

void CollisionResolver::ResolveContact(Contact& contact, float deltaTime, float totalTime)
{
    ICollider* a = contact.Colliders[0];
    ICollider* b = contact.Colliders[1];
    if (!a || !b) return;

    if (IsStatic(a) && IsStatic(b)) return;

    if (a->GetColliderType() == ColliderType::Cube && b->GetColliderType() == ColliderType::Cube)
    {
        ResolveContactWithCubeVsCube(contact, deltaTime, totalTime);
    }
    else if (a->GetColliderType() == ColliderType::Cube && b->GetColliderType() == ColliderType::Sphere)
    {
        ResolveVelocity(contact, deltaTime);
        ResolvePenetrationWithCubeVsSphere(contact, deltaTime);
    }
    else if (a->GetColliderType() == ColliderType::Sphere && b->GetColliderType() == ColliderType::Cube)
    {
        ICollider* a = contact.Colliders[0];
        ICollider* b = contact.Colliders[1];
        contact.Colliders[1] = a;
        contact.Colliders[0] = b;
        ResolveVelocity(contact, deltaTime);
        ResolvePenetrationWithCubeVsSphere(contact, deltaTime);
    }
	else
    {
        ResolveVelocity(contact, deltaTime);
        ResolvePenetration(contact, deltaTime);
    }
}

void CollisionResolver::ResolveContacts(std::vector<Contact>& contacts, float deltaTime, float totalTime)
{
    for (Contact& contact : contacts)
    {
        ResolveContact(contact, deltaTime, totalTime);
    }
}

void CollisionResolver::SetToleranceCount(int val)
{
    m_ResolveTolerance = val;
}

void CollisionResolver::ResolveContactWithCubeVsCube(Contact& contact, float deltaTime, float totalTime)
{
    ResolveVelocityWithCubeVsCube(contact, deltaTime);
    ResolveFrictionWithCubeVsCube(contact, deltaTime);
    ResolvePenetration(contact, deltaTime);
    ResolveRestingStateWithCubeVsCube(contact, deltaTime);
    ResolveAngularDampingWithCubeVsCube(contact, deltaTime);
}

void CollisionResolver::ResolvePenetration(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    ICollider* colliderA = contact.Colliders[0];
    ICollider* colliderB = contact.Colliders[1];

    RigidBody* bodyA = colliderA->GetRigidBody();
    RigidBody* bodyB = colliderB->GetRigidBody();

    const bool isStaticA = colliderA->GetColliderState() == ColliderState::Static;
    const bool isStaticB = colliderB->GetColliderState() == ColliderState::Static;

    // If both objects are static, do nothing
    if (isStaticA && isStaticB)
        return;

    const float slop = 0.01f;
    const float percent = 0.8f;

    float penetration = contact.PenetrationDepth - slop;
    if (penetration <= 0.0f)
        return;

    XMVECTOR normal = XMLoadFloat3(&contact.ContactNormal);

    if (isStaticA)
    {
        // Move only B along the normal
        XMVECTOR correction = XMVectorScale(normal, penetration * percent);
        XMVECTOR posB = bodyB->GetPosition();
        posB = XMVectorAdd(posB, correction);
        bodyB->SetPosition(posB);
    }
    else if (isStaticB)
    {
        // Move only A against the normal
        XMVECTOR correction = XMVectorScale(normal, penetration * percent);
        XMVECTOR posA = bodyA->GetPosition();
        posA = XMVectorSubtract(posA, correction);
        bodyA->SetPosition(posA);
    }
    else
    {
        // Both dynamic — split correction based on inverse mass
        float invMassA = bodyA->GetInverseMass();
        float invMassB = bodyB->GetInverseMass();
        float totalInvMass = invMassA + invMassB;

        if (totalInvMass <= 0.0f)
            return;

        XMVECTOR correction = XMVectorScale(normal, (penetration * percent) / totalInvMass);

        XMVECTOR posA = bodyA->GetPosition();
        posA = XMVectorSubtract(posA, XMVectorScale(correction, invMassA));
        bodyA->SetPosition(posA);

        XMVECTOR posB = bodyB->GetPosition();
        posB = XMVectorAdd(posB, XMVectorScale(correction, invMassB));
        bodyB->SetPosition(posB);
    }
}

void CollisionResolver::ResolveVelocityWithCubeVsCube(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    ICollider* colliderA = contact.Colliders[0];
    ICollider* colliderB = contact.Colliders[1];

    RigidBody* bodyA = colliderA->GetRigidBody();
    RigidBody* bodyB = colliderB->GetRigidBody();

    const bool isStaticA = colliderA->GetColliderState() == ColliderState::Static;
    const bool isStaticB = colliderB->GetColliderState() == ColliderState::Static;

    if (isStaticA && isStaticB)
        return;

    const float invMassA = bodyA->GetInverseMass();
    const float invMassB = bodyB->GetInverseMass();
    const float totalInvMass = invMassA + invMassB;

    if (totalInvMass <= 0.0f)
        return;

    XMVECTOR contactPoint = XMLoadFloat3(&contact.ContactPoint);
    XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&contact.ContactNormal));

    // rA = contact - centerA
    XMVECTOR rA = XMVectorSubtract(contactPoint, bodyA->GetPosition());
    XMVECTOR rB = XMVectorSubtract(contactPoint, bodyB->GetPosition());

    // Velocity at contact points
    XMVECTOR vA = XMVectorAdd(bodyA->GetVelocity(), XMVector3Cross(bodyA->GetAngularVelocity(), rA));
    XMVECTOR vB = XMVectorAdd(bodyB->GetVelocity(), XMVector3Cross(bodyB->GetAngularVelocity(), rB));

    // Relative velocity
    XMVECTOR vRel = XMVectorSubtract(vA, vB);
    float vRelAlongNormal = XMVectorGetX(XMVector3Dot(vRel, normal));

    XMFLOAT3 relVelF;
    XMStoreFloat3(&relVelF, vRel);

    if (contact.PenetrationDepth <= 0.0f && vRelAlongNormal > 0.0f)
        return;

    // Compute combined restitution with elasticity
    float restitution = contact.Restitution * contact.Elasticity;

    // Compute angular terms
    XMVECTOR raCrossN = XMVector3Cross(rA, normal);
    XMVECTOR rbCrossN = XMVector3Cross(rB, normal);

    XMVECTOR raInertia = XMVector3Transform(raCrossN, bodyA->GetInverseInertiaTensorWorld());
    XMVECTOR rbInertia = XMVector3Transform(rbCrossN, bodyB->GetInverseInertiaTensorWorld());

    float angularTermA = XMVectorGetX(XMVector3Dot(raInertia, raCrossN));
    float angularTermB = XMVectorGetX(XMVector3Dot(rbInertia, rbCrossN));

    float denom = totalInvMass + angularTermA + angularTermB;

    if (denom <= 0.0f)
        return;

    float j = -(1.0f + restitution) * vRelAlongNormal / denom;
    contact.NormalImpulseMagnitude = j;

    XMVECTOR impulse = XMVectorScale(normal, j);

    // Apply impulses
    if (!isStaticA)
    {
        bodyA->ApplyLinearImpulse(impulse);
        bodyA->ApplyAngularImpulse(impulse, rA);
    }

    if (!isStaticB)
    {
        XMVECTOR negImpulse = XMVectorNegate(impulse);
        bodyB->ApplyLinearImpulse(negImpulse);
        bodyB->ApplyAngularImpulse(negImpulse, rB);
    }
}

void CollisionResolver::ResolveFrictionWithCubeVsCube(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    ICollider* colliderA = contact.Colliders[0];
    ICollider* colliderB = contact.Colliders[1];

    RigidBody* bodyA = colliderA->GetRigidBody();
    RigidBody* bodyB = colliderB->GetRigidBody();

    const bool isStaticA = colliderA->GetColliderState() == ColliderState::Static;
    const bool isStaticB = colliderB->GetColliderState() == ColliderState::Static;

    if (isStaticA && isStaticB)
        return;

    XMVECTOR contactPoint = XMLoadFloat3(&contact.ContactPoint);
    XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&contact.ContactNormal));

    // rA and rB
    XMVECTOR rA = XMVectorSubtract(contactPoint, bodyA->GetPosition());
    XMVECTOR rB = XMVectorSubtract(contactPoint, bodyB->GetPosition());

    // Velocity at contact point
    XMVECTOR vA = XMVectorAdd(bodyA->GetVelocity(), XMVector3Cross(bodyA->GetAngularVelocity(), rA));
    XMVECTOR vB = XMVectorAdd(bodyB->GetVelocity(), XMVector3Cross(bodyB->GetAngularVelocity(), rB));

    XMVECTOR vRel = XMVectorSubtract(vA, vB);

    // Remove normal component
    float vRelAlongNormal = XMVectorGetX(XMVector3Dot(vRel, normal));
    XMVECTOR vRelNormal = XMVectorScale(normal, vRelAlongNormal);
    XMVECTOR vTangent = XMVectorSubtract(vRel, vRelNormal);

    // Skip if no tangent movement
    if (XMVector3LengthSq(vTangent).m128_f32[0] < 1e-6f)
        return;

    XMVECTOR tangent = XMVector3Normalize(vTangent);

    // jt denominator
    XMVECTOR raCrossT = XMVector3Cross(rA, tangent);
    XMVECTOR rbCrossT = XMVector3Cross(rB, tangent);

    XMVECTOR raInertia = XMVector3Transform(raCrossT, bodyA->GetInverseInertiaTensorWorld());
    XMVECTOR rbInertia = XMVector3Transform(rbCrossT, bodyB->GetInverseInertiaTensorWorld());

    float angularTermA = XMVectorGetX(XMVector3Dot(raInertia, raCrossT));
    float angularTermB = XMVectorGetX(XMVector3Dot(rbInertia, rbCrossT));

    float denom = bodyA->GetInverseMass() + bodyB->GetInverseMass() + angularTermA + angularTermB;
    if (denom <= 0.0f)
        return;

    float jt = -XMVectorGetX(XMVector3Dot(vRel, tangent)) / denom;

    // Clamp friction impulse
    float maxFriction = contact.Friction * std::abs(contact.NormalImpulseMagnitude);
    jt = std::clamp(jt, -maxFriction, maxFriction);

    XMVECTOR frictionImpulse = XMVectorScale(tangent, jt);

    if (!isStaticA)
    {
        bodyA->ApplyLinearImpulse(frictionImpulse);
        bodyA->ApplyAngularImpulse(frictionImpulse, rA);
    }

    if (!isStaticB)
    {
        XMVECTOR negFrictionImpulse = XMVectorNegate(frictionImpulse);
        bodyB->ApplyLinearImpulse(negFrictionImpulse);
        bodyB->ApplyAngularImpulse(negFrictionImpulse, rB);
    }
}

void CollisionResolver::ResolveRestingStateWithCubeVsCube(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    ICollider* colliderA = contact.Colliders[0];
    ICollider* colliderB = contact.Colliders[1];

    RigidBody* bodyA = colliderA->GetRigidBody();
    RigidBody* bodyB = colliderB->GetRigidBody();

    const bool isStaticA = colliderA->GetColliderState() == ColliderState::Static;
    const bool isStaticB = colliderB->GetColliderState() == ColliderState::Static;

    if (!(isStaticA ^ isStaticB)) // One must be static
        return;

    RigidBody* dynamicBody = isStaticA ? bodyB : bodyA;
    RigidBody* platformBody = isStaticA ? bodyA : bodyB;

    // Optional: if you use IsPlatform() flag on spheres too
    if (!platformBody->IsPlatform())
        return;

    // 1. Check upward contact normal
    static const XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
    XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&contact.ContactNormal));
    float upwardness = XMVectorGetX(XMVector3Dot(normal, worldUp));
    if (upwardness < 0.7f)
        return;

    // 2. Shallow penetration
    if (contact.PenetrationDepth > 0.01f)
        return;

    // 3. Check low contact point velocity
    XMVECTOR contactPoint = XMLoadFloat3(&contact.ContactPoint);
    XMVECTOR r = XMVectorSubtract(contactPoint, dynamicBody->GetPosition());

    XMVECTOR velocity = XMVectorAdd(dynamicBody->GetVelocity(), XMVector3Cross(dynamicBody->GetAngularVelocity(), r));
    float speed = XMVectorGetX(XMVector3Length(velocity));
    if (speed > 0.3f)
        return;

    // 4. Vertical velocity must be low
    XMFLOAT3 velFloat;
    XMStoreFloat3(&velFloat, dynamicBody->GetVelocity());
    if (std::abs(velFloat.y) > 0.4f)
        return;

    // Passed all checks
    dynamicBody->SetRestingState(true);
}

void CollisionResolver::ResolveAngularDampingWithCubeVsCube(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    for (int i = 0; i < 2; ++i)
    {
        ICollider* collider = contact.Colliders[i];
        if (!collider || collider->GetColliderState() == ColliderState::Static)
            continue;

        RigidBody* body = collider->GetRigidBody();
        if (!body)
            continue;

        XMVECTOR angVel = body->GetAngularVelocity();

        const float threshold = 0.01f;
        if (XMVectorGetX(XMVector3LengthSq(angVel)) < threshold)
        {
            body->SetAngularVelocity(XMVectorZero());
            continue;
        }

        float damping = body->GetAngularDamping();
        XMVECTOR damped = XMVectorScale(angVel, std::pow(damping, deltaTime));
        body->SetAngularVelocity(damped);
    }
}

void CollisionResolver::ResolveVelocityWithCubeVsSphere(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    ICollider* colliderA = contact.Colliders[0]; // Cube
    ICollider* colliderB = contact.Colliders[1]; // Sphere

    RigidBody* bodyA = colliderA->GetRigidBody();
    RigidBody* bodyB = colliderB->GetRigidBody();

    const bool isStaticA = colliderA->GetColliderState() == ColliderState::Static;
    const bool isStaticB = colliderB->GetColliderState() == ColliderState::Static;

    if (isStaticA && isStaticB)
    {
        return;
    }

    const float invMassA = bodyA->GetInverseMass();
    const float invMassB = bodyB->GetInverseMass();
    const float totalInvMass = invMassA + invMassB;

    if (totalInvMass <= 0.0f)
    {
        return;
    }

    XMVECTOR contactPoint = XMLoadFloat3(&contact.ContactPoint);
    XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&contact.ContactNormal));

    XMVECTOR rA = contactPoint - bodyA->GetPosition();
    XMVECTOR rB = contactPoint - bodyB->GetPosition();

    XMVECTOR vA = bodyA->GetVelocity() + XMVector3Cross(bodyA->GetAngularVelocity(), rA);
    XMVECTOR vB = bodyB->GetVelocity();

    XMVECTOR vRel = vA - vB;
    float vRelAlongNormal = XMVectorGetX(XMVector3Dot(vRel, normal));

    if (contact.PenetrationDepth <= 0.0f && vRelAlongNormal > 1e-4f)
    {
        return;
    }

    float restitution = contact.Restitution * contact.Elasticity;

    XMVECTOR raCrossN = XMVector3Cross(rA, normal);
    XMVECTOR raInertia = XMVector3Transform(raCrossN, bodyA->GetInverseInertiaTensorWorld());
    float angularTermA = XMVectorGetX(XMVector3Dot(raInertia, raCrossN));

    float angularTermB = 0.0f;
    if (!isStaticB)
    {
        XMVECTOR rbCrossN = XMVector3Cross(rB, normal);
        XMVECTOR rbInertia = XMVector3Transform(rbCrossN, bodyB->GetInverseInertiaTensorWorld());
        angularTermB = XMVectorGetX(XMVector3Dot(rbInertia, rbCrossN));
    }

    float denom = totalInvMass + angularTermA + angularTermB;
    if (denom <= 0.0f)
    {
        return;
    }

    if (vRelAlongNormal > 0.0f)
    {
        if (contact.PenetrationDepth > 0.001f)
        {
            float biasRestitution = 0.01f; // small bias to push them apart
            float jBias = -(1.0f + biasRestitution) * vRelAlongNormal / denom;
            XMVECTOR biasImpulse = XMVectorScale(normal, jBias);

            if (!isStaticA)
            {
                bodyA->ApplyLinearImpulse(biasImpulse);
                bodyA->ApplyAngularImpulse(biasImpulse, rA);
            }

            if (!isStaticB)
            {
                XMVECTOR negBiasImpulse = XMVectorNegate(biasImpulse);
                bodyB->ApplyLinearImpulse(negBiasImpulse);
                bodyB->ApplyAngularImpulse(negBiasImpulse, rB);
            }
        }
        return;
    }

    float j = -(1.0f + restitution) * vRelAlongNormal / denom;
    contact.NormalImpulseMagnitude = j;

    XMVECTOR impulse = XMVectorScale(normal, j);

    XMVECTOR velBefore = bodyB->GetVelocity(); // velocity before impulse

    if (!isStaticA)
    {

        bodyA->ApplyLinearImpulse(impulse);
        bodyA->ApplyAngularImpulse(impulse, rA);
    }

    if (!isStaticB)
    {
        XMVECTOR negImpulse = XMVectorNegate(impulse);

        bodyB->ApplyLinearImpulse(negImpulse);
        bodyB->ApplyAngularImpulse(negImpulse, rB); // optional

        XMVECTOR velAfter = bodyB->GetVelocity();
    }
}

void CollisionResolver::ResolvePenetrationWithCubeVsSphere(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    ICollider* colliderA = contact.Colliders[0]; // Cube
    ICollider* colliderB = contact.Colliders[1]; // Sphere

    RigidBody* bodyA = colliderA->GetRigidBody();
    RigidBody* bodyB = colliderB->GetRigidBody();

    const bool isStaticA = colliderA->GetColliderState() == ColliderState::Static;
    const bool isStaticB = colliderB->GetColliderState() == ColliderState::Static;

    // Exit early if both are static
    if (isStaticA && isStaticB)
        return;

    const float slop = 0.0001f;    // tolerance before pushing
    const float percent = 1.0f;    // aggressive correction

    float penetration = contact.PenetrationDepth - slop;
    if (penetration <= 0.0f)
        return;

    XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&contact.ContactNormal));

    // Calculate the correction vector
    XMVECTOR correction = XMVectorScale(normal, penetration * percent);

    if (isStaticA)
    {
        // Push only sphere
        XMVECTOR posB = bodyB->GetPosition();
        posB = XMVectorAdd(posB, correction);
        bodyB->SetPosition(posB);

    }
    else if (isStaticB)
    {
        // Push only cube
        XMVECTOR posA = bodyA->GetPosition();
        posA = XMVectorSubtract(posA, correction);
        bodyA->SetPosition(posA);

    }
    else
    {
        // Both dynamic  split by inverse mass
        float invMassA = bodyA->GetInverseMass();
        float invMassB = bodyB->GetInverseMass();
        float totalInvMass = invMassA + invMassB;

        if (totalInvMass <= 0.0f)
            return;

        XMVECTOR pushA = XMVectorScale(correction, invMassA / totalInvMass);
        XMVECTOR pushB = XMVectorScale(correction, invMassB / totalInvMass);

        XMVECTOR posA = bodyA->GetPosition();
        XMVECTOR posB = bodyB->GetPosition();

        bodyA->SetPosition(XMVectorSubtract(posA, pushA));
        bodyB->SetPosition(XMVectorAdd(posB, pushB));
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
    constexpr float percent = 1.1f;    // Resolve full overlap to prevent sticking

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
    // if (sepVel > 0.0f) return; // Already separating

    // std::cout << "Now Working!\n";
    // Use restitution and elasticity together
    float restitution = contact.Restitution;
    float elasticity = contact.Elasticity;
    float combinedRestitution = std::clamp(restitution * (1.0f + elasticity), 0.0f, 1.0f);

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
        //ApplyFriction(contact, relativeVel, normal * sepVel);
        ApplyFrictionImproved(contact, relativeVel, normal * sepVel);
    }
    else
    {
        ResolveDynamicVsDynamic(bodyA, bodyB, normal, sepVel, combinedRestitution, rA, rB, invInertiaA, invInertiaB);
        //ApplyFriction(contact, relativeVel, normal * sepVel);
        ApplyFrictionImproved(contact, relativeVel, normal * sepVel);
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

void CollisionResolver::ApplyFrictionImproved(Contact& contact, DirectX::XMVECTOR relativeVelocity,
	DirectX::XMVECTOR impulse)
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

    // --- Tangent Vector ---
    XMVECTOR tangent = relativeVelocity - normal * XMVector3Dot(relativeVelocity, normal);
    float tangentLenSq = XMVectorGetX(XMVector3LengthSq(tangent));
    if (tangentLenSq < 1e-6f) return;

    tangent = XMVectorScale(tangent, 1.0f / std::sqrt(tangentLenSq));

    // --- Friction Impulse ---
    float friction = contact.Friction;
    float normalImpulseMag = XMVectorGetX(XMVector3Length(impulse));
    float maxFrictionImpulseMag = friction * normalImpulseMag;

    float tangentialVelMag = XMVectorGetX(XMVector3Dot(relativeVelocity, tangent));
    float denom = invMassA + invMassB;
    float frictionImpulseMag = -tangentialVelMag / denom;
    frictionImpulseMag = std::clamp(frictionImpulseMag, -maxFrictionImpulseMag, maxFrictionImpulseMag);

    XMVECTOR frictionImpulse = tangent * frictionImpulseMag;

    // --- Apply Linear Impulse ---
    if (!isStaticA && invMassA > 0.0f)
        bodyA->ApplyLinearImpulse(-frictionImpulse);

    if (!isStaticB && invMassB > 0.0f)
        bodyB->ApplyLinearImpulse(frictionImpulse);

    // --- Apply Angular Friction ---
    XMVECTOR rA = contactPoint - bodyA->GetPosition();
    XMVECTOR rB = contactPoint - bodyB->GetPosition();

    XMMATRIX invInertiaA = bodyA->GetInverseInertiaTensor();
    XMMATRIX invInertiaB = bodyB->GetInverseInertiaTensor();

    if (!isStaticA && invMassA > 0.0f)
    {
        XMVECTOR angularImpulseA = XMVector3Transform(XMVector3Cross(rA, -frictionImpulse), invInertiaA);
        float damping = std::clamp(5.0f * invMassA, 0.1f, 1.0f);
        angularImpulseA *= damping;

        XMVECTOR angVelA = bodyA->GetAngularVelocity();
        bodyA->SetAngularVelocity(angVelA + angularImpulseA);

        if (XMVectorGetX(XMVector3LengthSq(angVelA)) < 0.01f)
        {
            float noiseStrength = normalImpulseMag * 0.05f;
            XMVECTOR randomTorque = GenerateRandomAngularNoise(noiseStrength);
            bodyA->SetAngularVelocity(bodyA->GetAngularVelocity() + randomTorque);
        }
    }

    if (!isStaticB && invMassB > 0.0f)
    {
        XMVECTOR angularImpulseB = XMVector3Transform(XMVector3Cross(rB, frictionImpulse), invInertiaB);
        float damping = std::clamp(5.0f * invMassB, 0.1f, 1.0f);
        angularImpulseB *= damping;

        XMVECTOR angVelB = bodyB->GetAngularVelocity();
        bodyB->SetAngularVelocity(angVelB + angularImpulseB);

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
    return collider->GetColliderState() == ColliderState::Static;
}

DirectX::XMVECTOR CollisionResolver::GetVelocityAtPoint(RigidBody* body, const DirectX::XMVECTOR& r)
{
    using namespace DirectX;

	XMVECTOR linearVel = body->GetVelocity();
	XMVECTOR angVel = body->GetAngularVelocity();

    return linearVel + XMVector3Cross(angVel, r);
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
    if (invMassA + invMassB <= 0.0f) return;

    // === Combine Restitution and Elasticity ===
    float elasticity = 0.5f * (bodyA->GetElasticity() + bodyB->GetElasticity());
    float combinedRestitution = restitution * (1.0f + elasticity);

    // === Angular Effects ===
    XMVECTOR rA_cross_n = XMVector3Cross(rA, normal);
    XMVECTOR rB_cross_n = XMVector3Cross(rB, normal);

    XMVECTOR angA = XMVector3Cross(XMVector3Transform(rA_cross_n, invInertiaA), rA);
    XMVECTOR angB = XMVector3Cross(XMVector3Transform(rB_cross_n, invInertiaB), rB);

    float angularEffect = XMVectorGetX(XMVector3Dot(angA + angB, normal));
    float denom = invMassA + invMassB + angularEffect;
    if (denom <= 1e-6f) return;

    // === Impulse Magnitude ===
    float impulseMag = -combinedRestitution * sepVel / denom;
    XMVECTOR impulse = normal * impulseMag;

    // === LINEAR + ANGULAR ===
    ApplyImpulse(bodyA, impulse, rA, invInertiaA);
    ApplyImpulse(bodyB, -impulse, rB, invInertiaB);

    // === Bounce-back Velocity ===
    XMVECTOR newVelA = bodyA->GetVelocity() + impulse * invMassA;
    XMVECTOR newVelB = bodyB->GetVelocity() - impulse * invMassB;
    bodyA->SetVelocity(newVelA);
    bodyB->SetVelocity(newVelB);

    // === Friction ===
    XMVECTOR velA = GetVelocityAtPoint(bodyA, rA);
    XMVECTOR velB = GetVelocityAtPoint(bodyB, rB);
    XMVECTOR relativeVel = velA - velB;

    XMVECTOR tangent = relativeVel - normal * XMVector3Dot(relativeVel, normal);
    if (XMVector3LengthSq(tangent).m128_f32[0] > 1e-6f)
    {
        tangent = XMVector3Normalize(tangent);

        float friction = 0.5f * (bodyA->GetFriction() + bodyB->GetFriction());
        float tangentialVelMag = XMVectorGetX(XMVector3Dot(relativeVel, tangent));
        float frictionImpulseMag = -tangentialVelMag / denom;

        float normalImpulseMag = XMVectorGetX(XMVector3Length(impulse));
        float maxFrictionImpulse = friction * normalImpulseMag;
        frictionImpulseMag = std::clamp(frictionImpulseMag, -maxFrictionImpulse, maxFrictionImpulse);

        XMVECTOR frictionImpulse = tangent * frictionImpulseMag;

        ApplyImpulse(bodyA, frictionImpulse, rA, invInertiaA);
        ApplyImpulse(bodyB, -frictionImpulse, rB, invInertiaB);
    }
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
