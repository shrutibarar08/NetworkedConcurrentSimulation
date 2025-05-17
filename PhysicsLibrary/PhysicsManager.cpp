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
    void update(float dt);
    void clear();

private:
    PhysicsManager(unsigned contactIterations);

    std::vector<RigidBody*> rigidBodies;
    std::vector<Collider*> colliders;
    std::vector<Contact> contacts;
    ContactResolver resolver;
};

// PhysicsManager.cpp
#include "PhysicsManager.h"

PhysicsManager& PhysicsManager::get() {
    static PhysicsManager instance(10);
    return instance;
}

PhysicsManager::PhysicsManager(unsigned contactIterations)
    : resolver(contactIterations) {
}

void PhysicsManager::addRigidBody(RigidBody* body, Collider* collider) {
    rigidBodies.push_back(body);
    colliders.push_back(collider);
}

void PhysicsManager::update(float dt, IntegrationType type) {
    for (RigidBody* body : rigidBodies) {
        body->integrate(dt, type);
    }

    contacts.clear();
    for (size_t i = 0; i < colliders.size(); ++i) {
        for (size_t j = i + 1; j < colliders.size(); ++j) {
            Contact contact;
            if (colliders[i]->checkCollision(colliders[j], contact)) {
                contacts.push_back(contact);
            }
        }
    }

    if (!contacts.empty()) {
        resolver.resolveContacts(contacts.data(), (unsigned int)contacts.size(), dt);
    }
}


void PhysicsManager::clear() {
    rigidBodies.clear();
    colliders.clear();
    contacts.clear();
}
