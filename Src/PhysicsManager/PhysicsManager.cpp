#include "PhysicsManager.h"

#include "CollisionResolver.h" 
#include "RenderManager/Model/IModel.h"
#include "Utils/Logger.h"

#include <ranges>
#include "Contact.h"
#include "RigidBody.h"

bool PhysicsManager::Shutdown()
{
	return ISystem::Shutdown();
}

bool PhysicsManager::Run()
{
    ISystem::Run();

    while (true)
    {
        if (mGlobalEvent.GlobalEndEvent)
        {
            DWORD result = WaitForSingleObject(mGlobalEvent.GlobalEndEvent, 0);
            if (result == WAIT_OBJECT_0)
            {
                LOG_INFO("[PhysicsManager] GlobalEndEvent signaled. Exiting loop.\n");
                break;
            }
        }
        Update(SystemClock::GetDeltaTime(), IntegrationType::Euler);
    }
    return true;
}

bool PhysicsManager::Build(SweetLoader& sweetLoader)
{
	return true;
}

bool PhysicsManager::AddRigidBody(const IModel* model)
{
    AcquireSRWLockExclusive(&m_Lock);
    LOG_INFO("Adding A model in Physics Manager!");
    ID id = model->GetModelId();
    if (m_PhysicsEntity.contains(id))
    {
		LOG_WARNING("Failed To add model into physics loop");
        return false;
    }

    m_PhysicsEntity[id] = model->GetCollider();
    ReleaseSRWLockExclusive(&m_Lock);
	LOG_SUCCESS("Added model in physics loop!");
    return true;
}

bool PhysicsManager::RemoveRigidBody(ID id)
{
    if (!m_PhysicsEntity.contains(id)) return false;

    AcquireSRWLockExclusive(&m_Lock);
    m_PhysicsEntity.erase(id);
    ReleaseSRWLockExclusive(&m_Lock);

    return true;
}

bool PhysicsManager::HasRigidBody(uint64_t id)
{
    AcquireSRWLockExclusive(&m_Lock);
    bool status = status = m_PhysicsEntity.contains(id);
    ReleaseSRWLockExclusive(&m_Lock);
    return status;
}

void PhysicsManager::Update(float dt, IntegrationType type)
{
    AcquireSRWLockShared(&m_Lock);

    // === Integrate bodies ===
    for (auto& collider : m_PhysicsEntity | std::views::values)
    {
        if (!collider) continue;

        RigidBody* body = collider->GetRigidBody();
        if (!body) continue;

        body->Integrate(dt, type);
    }

    size_t count = m_PhysicsEntity.size();
    std::vector<ICollider*> colliders;
    colliders.reserve(count);

    for (auto& collider : m_PhysicsEntity | std::views::values)
    {
        if (collider) colliders.push_back(collider);
    }

    // === Collision detection ===
    std::vector<Contact> contacts;
    for (size_t i = 0; i < colliders.size(); ++i)
    {
        for (size_t j = i + 1; j < colliders.size(); ++j)
        {
            ICollider* colliderA = colliders[i];
            ICollider* colliderB = colliders[j];

            if (!colliderA || !colliderB) continue;

            Contact contact;
            if (colliderA->CheckCollision(colliderB, contact))
            {

                if (colliderA->GetColliderType() == ColliderType::Capsule ||
                    colliderB->GetColliderType() == ColliderType::Capsule)
                {
                    RigidBody* bodyA = colliderA->GetRigidBody();
                    RigidBody* bodyB = colliderB->GetRigidBody();

                    using namespace DirectX;

                    XMFLOAT3 posA{}, velA{}, angVelA{};
                    XMFLOAT3 posB{}, velB{}, angVelB{};

                    if (colliderA->GetColliderType() == ColliderType::Capsule ||
                        colliderB->GetColliderType() == ColliderType::Capsule)
                    {
                        RigidBody* bodyA = colliderA->GetRigidBody();
                        RigidBody* bodyB = colliderB->GetRigidBody();

                        using namespace DirectX;

                        XMFLOAT3 posA{}, velA{}, angVelA{};
                        XMFLOAT3 posB{}, velB{}, angVelB{};

                        if (bodyA)
                        {
                            XMStoreFloat3(&posA, bodyA->GetPosition());
                            XMStoreFloat3(&velA, bodyA->GetVelocity());
                            XMStoreFloat3(&angVelA, bodyA->GetAngularVelocity());

                            LOG_INFO(std::string("Capsule A - Pos: (") +
                                std::to_string(posA.x) + ", " + std::to_string(posA.y) + ", " + std::to_string(posA.z) + "), Vel: (" +
                                std::to_string(velA.x) + ", " + std::to_string(velA.y) + ", " + std::to_string(velA.z) + "), AngVel: (" +
                                std::to_string(angVelA.x) + ", " + std::to_string(angVelA.y) + ", " + std::to_string(angVelA.z) + ")"
                            );
                        }

                        if (bodyB)
                        {
                            XMStoreFloat3(&posB, bodyB->GetPosition());
                            XMStoreFloat3(&velB, bodyB->GetVelocity());
                            XMStoreFloat3(&angVelB, bodyB->GetAngularVelocity());

                            LOG_INFO(std::string("Capsule B - Pos: (") +
                                std::to_string(posB.x) + ", " + std::to_string(posB.y) + ", " + std::to_string(posB.z) + "), Vel: (" +
                                std::to_string(velB.x) + ", " + std::to_string(velB.y) + ", " + std::to_string(velB.z) + "), AngVel: (" +
                                std::to_string(angVelB.x) + ", " + std::to_string(angVelB.y) + ", " + std::to_string(angVelB.z) + ")"
                            );
                        }
                    }
                }

                contacts.push_back(contact);
            }
        }
    }
    // === Resolve all detected contacts ===
    CollisionResolver::ResolveContacts(contacts, dt);

    ReleaseSRWLockShared(&m_Lock);
}
