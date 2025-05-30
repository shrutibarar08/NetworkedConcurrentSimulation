#pragma once

#include <vector>
#include "Contact.h"

class CollisionResolver
{
public:
    // Resolve a single contact (positional + velocity)
    static void ResolveContact(Contact& contact, float deltaTime, float totalTime);

    // Resolve a batch of contacts (e.g. from PhysicsManager)
    static void ResolveContacts(std::vector<Contact>& contacts, float deltaTime, float totalTime);
    static void SetToleranceCount(int val);

private:

    //~ Cube vs Cube
    static void ResolveContactWithCubeVsCube(Contact& contact, float deltaTime, float totalTime);
    static void ResolvePenetrationWithCubeVsCube(Contact& contact, float deltaTime);
    static void ResolveVelocityWithCubeVsCube(Contact& contact, float deltaTime);
    static void ResolveFrictionWithCubeVsCube(Contact& contact, float deltaTime);
    static void ResolveRestingStateWithCubeVsCube(Contact& contact, float deltaTime);
    static void ResolveAngularDampingWithCubeVsCube(Contact& contact, float deltaTime);

    //~ Specific to spheres
    static void ResolveVelocityWithCubeVsSphere(Contact& contact, float deltaTime);

    //~ Resolve Inter Penetration
    static void ResolvePositionInterpenetration(const Contact& contact);

    // Resolve collisions
    static void ResolveVelocity(Contact& contact, float deltaTime);

    //~ Helper Functions
    static void ApplyFriction(Contact& contact, DirectX::XMVECTOR relativeVelocity, DirectX::XMVECTOR impulse);
    static bool IsStatic(ICollider* collider);
    static DirectX::XMVECTOR GetVelocityAtPoint(RigidBody* body, const DirectX::XMVECTOR& r);
    static float ComputeDenominator(
        float invMassA, float invMassB,
        const DirectX::XMVECTOR& rA, const DirectX::XMVECTOR& rB,
        const DirectX::XMVECTOR& normal,
        const DirectX::XMMATRIX& invInertiaA,
        const DirectX::XMMATRIX& invInertiaB);
    static void ApplyImpulse(
        RigidBody* body, const DirectX::XMVECTOR& impulse,
        const DirectX::XMVECTOR& r,
        const DirectX::XMMATRIX& invInertia);
    static void ReflectFromStatic(
        RigidBody* body, const DirectX::XMVECTOR& normal,
        float sepVel, float restitution,
        const DirectX::XMVECTOR& r,
        const DirectX::XMMATRIX& invInertia);
    static void ResolveDynamicVsDynamic(
        RigidBody* bodyA, RigidBody* bodyB,
        const DirectX::XMVECTOR& normal,
        float sepVel, float restitution,
        const DirectX::XMVECTOR& rA, const DirectX::XMVECTOR& rB,
        const DirectX::XMMATRIX& invInertiaA, const DirectX::XMMATRIX& invInertiaB);

    static DirectX::XMVECTOR GenerateRandomAngularNoise(float strength);

private:
    inline static int m_ResolveTolerance{ 6 }; // always choose even.
};
