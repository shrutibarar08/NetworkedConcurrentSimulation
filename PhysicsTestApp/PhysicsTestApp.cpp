#include "pch.h"
#include "Quaternion.h"
#include <Windows.h>
#include "RigidBody.h"
#include "Contact.h"
#include "ContactResolver.h"
#include "ForceRegistry.h"
#include "Gravity.h"
#include "Drag.h"
#include "PhysicsManager.h"
#include "SphereCollider.h"
#include "PlaneCollider.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include <iostream>

void simulateManualCollisionTest()
{
    std::cout << "=== Manual Contact Resolution ===\n";

    RigidBody bodyA;
    bodyA.SetPosition(DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
    bodyA.SetVelocity(DirectX::XMVectorSet(5.0f, 0.0f, 0.0f, 0.0f));
    bodyA.SetMass(1.0f);

    RigidBody bodyB;
    bodyB.SetPosition(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
    bodyB.SetVelocity(DirectX::XMVectorSet(-5.0f, 0.0f, 0.0f, 0.0f));
    bodyB.SetMass(1.0f);

    Contact contact;
    contact.Body[0] = &bodyA;
    contact.Body[1] = &bodyB;
    contact.ContactNormal = DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f);
    contact.ContactPoint = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    contact.Penetration = 0.2f;
    contact.Restitution = 0.9f;
    contact.Friction = 0.0f;

    ContactResolver resolver(1);
    float dt = 1.0f / 60.0f;

    for (int i = 0; i < 10; ++i) // Simulate 10 frames
    {
        std::cout << "[Frame " << i << "]\n";

        // Integrate motion (optional, depending on your physics setup)
        bodyA.Integrate(dt, IntegrationType::Euler);
        bodyB.Integrate(dt, IntegrationType::Euler);

        // Resolve collision
        resolver.ResolveContacts(&contact, 1, dt);

        DirectX::XMVECTOR posA = bodyA.GetPosition();
        DirectX::XMVECTOR velA = bodyA.GetVelocity();
        DirectX::XMVECTOR posB = bodyB.GetPosition();
        DirectX::XMVECTOR velB = bodyB.GetVelocity();

        std::cout << "  A Pos: " << DirectX::XMVectorGetX(posA)
            << " Vel: " << DirectX::XMVectorGetX(velA) << "\n";
        std::cout << "  B Pos: " << DirectX::XMVectorGetX(posB)
            << " Vel: " << DirectX::XMVectorGetX(velB) << "\n\n";

        Sleep(500); // Sleep 0.5 seconds
    }
}

void simulateDragAndGravity()
{
    std::cout << "=== Particle Gravity + Drag ===\n";

    RigidBody body;
    body.SetMass(1.0f);
    body.Position = DirectX::XMVectorSet(0.0f, 10.0f, 0.0f, 0.0f);
    body.Velocity = DirectX::XMVectorSet(10.0f, 0.0f, 0.0f, 0.0f);
    body.Acceleration = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    body.SetDamping(0.99f);

    Gravity gravity(DirectX::XMVectorSet(0.0f, -9.81f, 0.0f, 0.0f));
    Drag drag(0.1f, 0.01f);

    ForceRegistry registry;
    registry.Add(&body, &gravity);
    registry.Add(&body, &drag);

    float duration = 0.001f;

    for (int i = 0; i < 60; ++i) 
    {
        if (i > 0)
        {
            registry.UpdateForces(duration);
            body.Integrate(duration, IntegrationType::Euler);
        }

        DirectX::XMVECTOR pos = body.Position;
        DirectX::XMVECTOR vel = body.Velocity;

        std::cout << "[t=" << duration * static_cast<float>(i) << " seconds] "
            << "Pos: (" << DirectX::XMVectorGetX(pos) << ", "
            << DirectX::XMVectorGetY(pos) << ", "
            << DirectX::XMVectorGetZ(pos) << ") "
            << "Vel: (" << DirectX::XMVectorGetX(vel) << ", "
            << DirectX::XMVectorGetY(vel) << ", "
            << DirectX::XMVectorGetZ(vel) << ")\n";

        Sleep(1);
    }

    std::cout << "\n";
}

void simulatePhysicsManagerScene() {
    std::cout << "=== Physics Manager Test ===\n";

    RigidBody* a = new RigidBody();
    a->SetPosition(DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
    a->SetVelocity(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
    a->SetMass(1.0f);

    RigidBody* b = new RigidBody();
    b->SetPosition(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
    b->SetVelocity(DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
    b->SetMass(1.0f);

    SphereCollider* sphereA = new SphereCollider(a, 1.0f);
    SphereCollider* sphereB = new SphereCollider(b, 1.0f);

    PhysicsManager& physicsManager = PhysicsManager::Get();
    physicsManager.AddRigidBody(a, sphereA);
    physicsManager.AddRigidBody(b, sphereB);

    for (int i = 0; i < 60; ++i) {
        physicsManager.Update(1.0f / 60.0f);
        DirectX::XMVECTOR posA = a->GetPosition();
        DirectX::XMVECTOR posB = b->GetPosition();
        std::cout << "[t=" << i * 1.0f / 60 << "] A: "
            << DirectX::XMVectorGetX(posA) << ", B: "
            << DirectX::XMVectorGetX(posB) << "\n";
    }

    std::cout << "\n";

    physicsManager.Clear();
    delete a; delete b;
    delete sphereA; delete sphereB;
}

void simulateBoxAndPlaneCollision()
{
    std::cout << "=== Box vs Plane ===\n";

    RigidBody* boxBody = new RigidBody();
    boxBody->SetPosition(DirectX::XMVectorSet(10.f, 0.0f, 0.0f, 0.0f));
    boxBody->SetVelocity(DirectX::XMVectorSet(-4.0f, 0.0f, 0.0f, 0.0f));
    boxBody->SetMass(1.0f);

    RigidBody* boxBody_2 = new RigidBody();
    boxBody_2->SetMass(1.0f);
    boxBody_2->SetPosition({ -10.0f, 0.0f, 0.0f });
    boxBody_2->SetVelocity({ 4.0f, 0.0f, 0.0f });

    BoxCollider* box = new BoxCollider(boxBody, { 0.5f, 0.5f, 0.5f });
    BoxCollider* box_2 = new BoxCollider(boxBody, { 0.5f, 0.5f, 0.5f });

    PhysicsManager& manager = PhysicsManager::Get();
    manager.AddRigidBody(boxBody, box);
    manager.AddRigidBody(boxBody_2, box_2);

    float duration = 0.001f;
    int i = 0;
    while (true)
    {
        if (i > 0)
        {
            manager.Update(duration);
        }

        DirectX::XMVECTOR pos = boxBody->GetPosition();
        std::cout << "[t=" << i * duration << "] Box Y: "
            << DirectX::XMVectorGetX(pos) << "\n";

        Sleep(1);
        i++;
    }
}

int main()
{
    simulateBoxAndPlaneCollision();
    return 0;
}
