#include "pch.h"
#include "PhysicsManager.h"

#include <iostream>

PhysicsManager& PhysicsManager::Get()
{
    static PhysicsManager instance(10);
    return instance;
}

PhysicsManager::PhysicsManager(unsigned contactIterations)
    : Resolver(contactIterations)
{}

void PhysicsManager::AddRigidBody(RigidBody* body, Collider* collider)
{
    RigidBodies.push_back(body);
    Colliders.push_back(collider);
}

void PhysicsManager::Update(float dt, IntegrationType type)
{
    Contacts.clear();
    for (size_t i = 0; i < Colliders.size(); ++i)
    {
        for (size_t j = i + 1; j < Colliders.size(); ++j) 
        {
            Contact contact;
            if (Colliders[i]->CheckCollision(Colliders[j], contact))
            {
                Contacts.push_back(contact);
            }
        }
    }

    if (!Contacts.empty())
    {
        Resolver.resolveContacts(Contacts.data(), static_cast<unsigned int>(Contacts.size()), dt);
    }

    for (RigidBody* body : RigidBodies)
    {
        body->Integrate(dt, type);
    }
}

void PhysicsManager::Clear()
{
    RigidBodies.clear();
    Colliders.clear();
    Contacts.clear();
}
