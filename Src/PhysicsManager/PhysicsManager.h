#pragma once
#include "ForceRegistry.h"
#include "Gravity.h"
#include "ICollider.h"
#include "ApplicationManager/Clock/SystemClock.h"
#include "SystemManager/Interface/ISystem.h"
#include "IntegrationType.h"
#include "Utils/LocalTimer.h"

class IModel;

typedef struct HIT_CACHE
{
	uint64_t HitOn;
	int HitCount;
	float LastTime;
}HIT_CACHE;

class PhysicsManager final: public ISystem
{
public:
	PhysicsManager();
	bool Shutdown() override;
	bool Run() override;
	bool Build(SweetLoader& sweetLoader) override;

	bool AddRigidBody(const IModel* model);
	bool RemoveRigidBody(ID id);
	bool HasRigidBody(uint64_t id);

	int GetCubeCounts();
	int GetSphereCounts();
	int GetCapsuleCounts();
	int GetTotalCounts();

	Gravity* GetGravity() const;

private:
	int GetColliderKey(const ICollider* collider) const;
	void IncreaseCount(int colliderKey);
	void DecreaseCount(int colliderKey);

	void Update(float dt, IntegrationType type = IntegrationType::SemiImplicitEuler);

private:
	mutable SRWLOCK m_Lock{ SRWLOCK_INIT };

	std::unordered_map<int, int> m_ObjectInfo{};

	ForceRegistry m_ForceRegister{};
	std::unique_ptr<Gravity>  m_Gravity{ nullptr };
	float m_TotalTime{ 0.0f };
	std::unordered_map<uint64_t, ICollider*> m_PhysicsEntity;
	LocalTimer m_Timer{};
};
