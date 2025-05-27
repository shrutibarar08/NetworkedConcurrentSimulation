#include "PhysicsManager.h"

#include "CollisionResolver.h" 
#include "RenderManager/Model/IModel.h"
#include "Utils/Logger.h"

#include <ranges>
#include "Contact.h"
#include "RigidBody.h"

PhysicsManager::PhysicsManager()
{
    DirectX::XMVECTOR grav{ 0.f, -9.81f, 0.f };
    m_Gravity = std::make_unique<Gravity>(grav);
}

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
        constexpr float dt = 1.0f / 60.0f; // fixed timestep
        float time = m_Timer.Tick();
        Update(time, IntegrationType::Euler);
        Sleep(dt);
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
    m_ForceRegister.Add(model->GetCollider(), m_Gravity.get());
    int key = GetColliderKey(m_PhysicsEntity[id]);
    IncreaseCount(key);
    ReleaseSRWLockExclusive(&m_Lock);
	LOG_SUCCESS("Added model in physics loop!");
    return true;
}

bool PhysicsManager::RemoveRigidBody(ID id)
{
    if (!m_PhysicsEntity.contains(id)) return false;

    AcquireSRWLockExclusive(&m_Lock);
    m_ForceRegister.Remove(m_PhysicsEntity[id], m_Gravity.get());
    int key = GetColliderKey(m_PhysicsEntity[id]);
    DecreaseCount(key);

    m_PhysicsEntity.erase(id);
    ReleaseSRWLockExclusive(&m_Lock);

    return true;
}

bool PhysicsManager::HasRigidBody(uint64_t id)
{
    return m_PhysicsEntity.contains(id);
}

int PhysicsManager::GetCubeCounts()
{
    return m_ObjectInfo[1];
}

int PhysicsManager::GetSphereCounts()
{
    return m_ObjectInfo[0];
}

int PhysicsManager::GetCapsuleCounts()
{
    return m_ObjectInfo[2];
}

int PhysicsManager::GetTotalCounts()
{
    return GetCapsuleCounts() + GetCubeCounts() + GetSphereCounts();
}

Gravity* PhysicsManager::GetGravity() const
{
    if (m_Gravity) return m_Gravity.get();
    return nullptr;
}

int PhysicsManager::GetColliderKey(const ICollider* collider) const
{
    if (collider->GetColliderType() == ColliderType::Capsule)
    {
        return 2;
    }
    if (collider->GetColliderType() == ColliderType::Cube)
    {
        return 1;
    }
    if (collider->GetColliderType() == ColliderType::Sphere)
    {
        return 0;
    }
    return -1;
}

void PhysicsManager::IncreaseCount(int colliderKey)
{
    m_ObjectInfo[colliderKey]++;
}

void PhysicsManager::DecreaseCount(int colliderKey)
{
    m_ObjectInfo[colliderKey]--;
}

void PhysicsManager::Update(float dt, IntegrationType type)
{
    m_TotalTime += dt;
	// === Integrate bodies ===
    AcquireSRWLockShared(&m_Lock);
    size_t count = m_PhysicsEntity.size();
    std::vector<ICollider*> colliders;
    std::vector<uint64_t> ids;

    for (auto& collider : m_PhysicsEntity)
    {
        if (!collider.second) continue;

        RigidBody* body = collider.second->GetRigidBody();
        if (!body) continue;
        colliders.push_back(collider.second);
        ids.push_back(collider.first);

        body->Integrate(dt, type);
    }
    m_ForceRegister.UpdateForces(dt);
    ReleaseSRWLockShared(&m_Lock);

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
            }
        }
    }
    // === Resolve all detected contacts ===
    CollisionResolver::ResolveContacts(contacts, dt, m_TotalTime);
}
