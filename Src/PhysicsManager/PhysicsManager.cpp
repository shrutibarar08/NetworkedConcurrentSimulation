#include "PhysicsManager.h"

#include <algorithm>

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
        if (m_Pause)
        {
            m_Timer.Tick();
            Sleep(1);
            continue;
        }
        const float targetStep = 1.0f / static_cast<float>(m_TargetSimulationHz);
        if (m_Timer.HasElapsed(targetStep))
        {
            m_ActualSimulationFrameTime = m_Timer.Tick();
            m_ActualSimulationHz = 1.0f / m_ActualSimulationFrameTime;

            // Use actual frame time for variable step
            Update(m_ActualSimulationFrameTime, m_SelectedIntegration);

            m_Timer.Reset();
        }
        else
        {
            if (!m_WaitCleaning)
            {
                UseCache();
            }
            else Sleep(1);
        }
    }
    return true;
}

bool PhysicsManager::Build(SweetLoader& sweetLoader)
{
	return true;
}

bool PhysicsManager::AddModel(ICollider* model)
{
    m_CacheRequest.push(model);
    return false;
}

bool PhysicsManager::Clear()
{
    m_WaitCleaning = true;
    m_ForceRegister.Clear();

    ICollider* collider;
    while (m_PhysicsEntity.try_pop(collider)) { /* drop all */ }

    m_WaitCleaning = false;
    return true;
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

void PhysicsManager::SetTargetDeltaTime(float time)
{
    m_TargetDeltaTime = std::clamp(time, 0.001f, 10.0f);
    m_TargetSimulationHz = static_cast<int>(1.0f / m_TargetDeltaTime);
}

float PhysicsManager::GetTargetDeltaTime() const
{
    return m_TargetDeltaTime;
}

void PhysicsManager::SetTargetSimulationHz(int hz)
{
    m_TargetSimulationHz = std::clamp(hz, 1, 1000);
    m_TargetDeltaTime = 1.0f / static_cast<float>(m_TargetSimulationHz);
}

int PhysicsManager::GetTargetSimulationHz() const
{
    return m_TargetSimulationHz;
}

float PhysicsManager::GetActualSimulationFrameTime() const
{
    return m_ActualSimulationFrameTime;
}

float PhysicsManager::GetActualSimulationHz() const
{
    return m_ActualSimulationHz;
}

IntegrationType PhysicsManager::GetSelectedIntegration() const
{
    return m_SelectedIntegration;
}

void PhysicsManager::SetIntegration(IntegrationType type)
{
    m_SelectedIntegration = type;
}

int PhysicsManager::GetColliderKey(const ICollider* collider)
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
    std::vector<ICollider*> colliders;

    // Save off colliders for collision

    ICollider* collider = nullptr;
    while (m_PhysicsEntity.try_pop(collider))
    {
        if (!collider) continue;

        collider->Update(dt);

        RigidBody* body = collider->GetRigidBody();
        if (!body) continue;

        body->Integrate(dt, type);
        colliders.push_back(collider);
    }

    m_ForceRegister.UpdateForces(dt);

    // === Collision Detection ===
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
                colliderA->RegisterCollision(colliderB);
                colliderB->RegisterCollision(colliderA);
                contacts.push_back(contact);
            }
        }
    }

    // === Contact Resolution ===
    CollisionResolver::ResolveContacts(contacts, dt, m_TotalTime);

    // re-queue
    for (ICollider* collider : colliders)
    {
        m_PhysicsEntity.push(collider);
    }

}

void PhysicsManager::UseCache()
{
    ICollider* collider = nullptr;
    if (!m_CacheRequest.try_pop(collider) || !collider || collider == reinterpret_cast<ICollider*>(-1))
        return;

    switch (collider->GetColliderType())
    {
    case ColliderType::Capsule: m_ObjectInfo[2]++; break;
    case ColliderType::Cube:    m_ObjectInfo[1]++; break;
    case ColliderType::Sphere:  m_ObjectInfo[0]++; break;
    default: break;
    }

    m_ForceRegister.Add(collider, m_Gravity.get());
    m_PhysicsEntity.push(collider);
}
