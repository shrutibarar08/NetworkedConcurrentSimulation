#include "PhysicsManager.h"

#include "CollisionResolver.h" 
#include "RenderManager/Model/IModel.h"
#include "Utils/Logger.h"

#include <ranges>
#include "Contact.h"


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
    LOG_INFO("Adding A model in Physics Manager!");
    ID id = model->GetModelId();
    if (m_PhysicsEntity.contains(id))
    {
		LOG_WARNING("Failed To add model into physics loop");
        return false;
    }

    AcquireSRWLockExclusive(&m_Lock);
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
        RigidBody* body = collider->GetRigidBody();
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
                contacts.push_back(contact);

                // Debug log (optional)
                LOG_INFO("Collision Detected!");
                LOG_INFO("Collider A: " + std::to_string(reinterpret_cast<uintptr_t>(colliderA)));
                LOG_INFO("Collider B: " + std::to_string(reinterpret_cast<uintptr_t>(colliderB)));
                LOG_INFO("Contact Point: (" +
                    std::to_string(contact.ContactPoint.x) + ", " +
                    std::to_string(contact.ContactPoint.y) + ", " +
                    std::to_string(contact.ContactPoint.z) + ")");
                LOG_INFO("Contact Normal: (" +
                    std::to_string(contact.ContactNormal.x) + ", " +
                    std::to_string(contact.ContactNormal.y) + ", " +
                    std::to_string(contact.ContactNormal.z) + ")");
                LOG_INFO("Penetration Depth: " + std::to_string(contact.PenetrationDepth));
                LOG_INFO("Restitution: " + std::to_string(contact.Restitution));
                LOG_INFO("Friction: " + std::to_string(contact.Friction));
                LOG_INFO("Elasticity: " + std::to_string(contact.Elasticity));
                LOG_INFO("-----------------------------------");
            }
        }
    }

    // === Resolve all detected contacts ===
    CollisionResolver::ResolveContacts(contacts, dt);

    ReleaseSRWLockShared(&m_Lock);
}
