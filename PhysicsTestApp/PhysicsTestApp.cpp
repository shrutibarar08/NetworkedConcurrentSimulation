#include "pch.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "RigidBody.h"
#include "Contact.h"
#include "ContactResolver.h"
#include "ForceRegistry.h"
#include "Particle.h"
#include "Gravity.h"
#include "Drag.h"
#include "PhysicsManager.h"
#include "SphereCollider.h"
#include "PlaneCollider.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include <iostream>

void simulateManualCollisionTest() {
    std::cout << "=== Manual Contact Resolution ===\n";

    RigidBody bodyA;
    bodyA.setPosition(Vector3(-1.0f, 0, 0));
    bodyA.setVelocity(Vector3(5.0f, 0, 0));
    bodyA.setMass(1.0f);

    RigidBody bodyB;
    bodyB.setPosition(Vector3(1.0f, 0, 0));
    bodyB.setVelocity(Vector3(-5.0f, 0, 0));
    bodyB.setMass(1.0f);

    Contact contact;
    contact.body[0] = &bodyA;
    contact.body[1] = &bodyB;
    contact.contactNormal = Vector3(-1.0f, 0.0f, 0.0f);
    contact.penetration = 0.2f;
    contact.restitution = 0.9f;
    contact.friction = 0.0f;
    contact.contactPoint = Vector3(0.0f, 0.0f, 0.0f);

    std::cout << "Before: A: " << bodyA.getVelocity().x << ", B: " << bodyB.getVelocity().x << "\n";
    ContactResolver resolver(1);
    resolver.resolveContacts(&contact, 1, 1.0f / 60.0f);
    std::cout << "After: A: " << bodyA.getVelocity().x << ", B: " << bodyB.getVelocity().x << "\n\n";
}

void simulateDragAndGravity() {
    std::cout << "=== Particle Gravity + Drag ===\n";
    Particle particle;
    particle.setMass(1.0f);
    particle.setPosition(Vector3(0, 10, 0));
    particle.setVelocity(Vector3(0, 0, 0));
    particle.setAcceleration(Vector3(0, 0, 0));
    particle.setDamping(0.99f);

    Gravity gravity(Vector3(0, -9.81f, 0));
    Drag drag(1.5f, 0.8f);
    ForceRegistry registry;
    registry.add(&particle, &gravity);
    registry.add(&particle, &drag);

    float duration = 0.016f;
    for (int i = 0; i < 60; ++i) {
        registry.updateForces(duration);
        particle.integrate(duration);
        Vector3 pos = particle.getPosition();
        std::cout << "[t=" << i * duration << "] Pos: (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
    }
    std::cout << "\n";
}

void simulatePhysicsManagerScene() {
    std::cout << "=== Physics Manager Test ===\n";
    RigidBody* a = new RigidBody();
    a->setPosition(Vector3(-1, 0, 0));
    a->setVelocity(Vector3(1, 0, 0));
    a->setMass(1.0f);

    RigidBody* b = new RigidBody();
    b->setPosition(Vector3(1, 0, 0));
    b->setVelocity(Vector3(-1, 0, 0));
    b->setMass(1.0f);

    SphereCollider* sphereA = new SphereCollider(a, 1.0f);
    SphereCollider* sphereB = new SphereCollider(b, 1.0f);

    PhysicsManager& physicsManager = PhysicsManager::get();
    physicsManager.addRigidBody(a, sphereA);
    physicsManager.addRigidBody(b, sphereB);

    for (int i = 0; i < 60; ++i) {
        physicsManager.update(1.0f / 60.0f);
        std::cout << "[t=" << i * 1.0f / 60 << "] A: " << a->getPosition().x << ", B: " << b->getPosition().x << "\n";
    }
    std::cout << "\n";

    physicsManager.clear();
    delete a; delete b;
    delete sphereA; delete sphereB;
}

void simulateBoxAndPlaneCollision() {
    std::cout << "=== Box vs Plane ===\n";
    RigidBody* boxBody = new RigidBody();
    boxBody->setPosition(Vector3(0, 1, 0));
    boxBody->setVelocity(Vector3(0, -1, 0));
    boxBody->setMass(1.0f);

    RigidBody* planeBody = new RigidBody();
    planeBody->setMass(0.0f);

    BoxCollider* box = new BoxCollider(boxBody, Vector3(0.5f, 0.5f, 0.5f));
    PlaneCollider* plane = new PlaneCollider(Vector3(0, 1, 0), 0);

    PhysicsManager& manager = PhysicsManager::get();
    manager.addRigidBody(boxBody, box);
    manager.addRigidBody(planeBody, plane);

    for (int i = 0; i < 60; ++i) {
        manager.update(1.0f / 60.0f);
        std::cout << "[t=" << i * 1.0f / 60 << "] Box Y: " << boxBody->getPosition().y << "\n";
    }

    manager.clear();
    delete boxBody; delete planeBody;
    delete box; delete plane;
}

int main() {
    simulateManualCollisionTest();
    simulateDragAndGravity();
    simulatePhysicsManagerScene();
    simulateBoxAndPlaneCollision();
    return 0;
}
