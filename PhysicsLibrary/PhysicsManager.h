#pragma once
#include <vector>
#include "RigidBody.h"
#include "Collider.h"
#include "Contact.h"
#include "ContactResolver.h"

class PhysicsManager {
public:
    static PhysicsManager& get();

    void addRigidBody(RigidBody* body, Collider* collider);
    void update(float dt, IntegrationType type = IntegrationType::SemiImplicitEuler);

    void clear();

private:
    PhysicsManager(unsigned contactIterations);

    std::vector<RigidBody*> rigidBodies;
    std::vector<Collider*> colliders;
    std::vector<Contact> contacts;
    ContactResolver resolver;
};
