#include "pch.h"
#include "PhysicsManager.h"

PhysicsManager& PhysicsManager::get() {
    static PhysicsManager instance(10); // 10 contact iterations
    return instance;
}

PhysicsManager::PhysicsManager(unsigned contactIterations)
    : resolver(contactIterations) {
}

void PhysicsManager::addRigidBody(RigidBody* body, Collider* collider) {
    rigidBodies.push_back(body);
    colliders.push_back(collider);
}

void PhysicsManager::update(float dt) {
    for (RigidBody* body : rigidBodies) {
        body->integrate(dt);
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
        unsigned int count = static_cast<unsigned int>(contacts.size());
        resolver.resolveContacts(contacts.data(), count, dt);
    }
}

void PhysicsManager::clear() {
    rigidBodies.clear();
    colliders.clear();
    contacts.clear();
}
