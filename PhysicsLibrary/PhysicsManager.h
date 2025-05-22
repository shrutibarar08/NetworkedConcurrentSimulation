#pragma once
#include <vector>
#include "RigidBody.h"
#include "Collider.h"
#include "Contact.h"
#include "ContactResolver.h"

class PhysicsManager
{
public:
    static PhysicsManager& Get();

    void AddRigidBody(RigidBody* body, Collider* collider);
    void Update(float dt, IntegrationType type = IntegrationType::SemiImplicitEuler);
    void Clear();

private:
    PhysicsManager(unsigned contactIterations);

    std::vector<RigidBody*> RigidBodies;
    std::vector<Collider*> Colliders;
    std::vector<Contact> Contacts;
    ContactResolver Resolver;
};
