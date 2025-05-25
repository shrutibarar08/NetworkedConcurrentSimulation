#pragma once

#include <vector>
#include "Contact.h"

class CollisionResolver
{
public:
    // Resolve a single contact (positional + velocity)
    static void ResolveContact(Contact& contact, float deltaTime);

    // Resolve a batch of contacts (e.g. from PhysicsManager)
    static void ResolveContacts(std::vector<Contact>& contacts, float deltaTime);

private:
    // Resolve Cube Related collision
    static void ResolveCubeInterPenetration(Contact& contact);
    static void ResolveCubeVelocity(Contact& contact, float deltaTime);
    static void ApplyCubeFriction(Contact& contact, DirectX::XMVECTOR relativeVelocity, DirectX::XMVECTOR impulse);


    //~ Helper Functions
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
};
