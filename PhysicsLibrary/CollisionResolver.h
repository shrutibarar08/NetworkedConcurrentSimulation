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
};
