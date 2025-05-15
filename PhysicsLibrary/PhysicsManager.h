#pragma once
#include <vector>
#include "RigidBody.h"
#include "ContactResolver.h"
#include "Contact.h"
#include "Collider.h"

class PhysicsManager {
private:
    std::vector<RigidBody*> rigidBodies;
    std::vector<Collider*> colliders;
    std::vector<Contact> contacts;
    ContactResolver resolver;

public:
    static PhysicsManager& get();

    void addRigidBody(RigidBody* body, Collider* collider);
    void update(float dt);
    void clear();

private:
    PhysicsManager(unsigned contactIterations = 10); 
};

