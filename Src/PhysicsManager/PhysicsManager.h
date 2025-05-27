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

class PhysicsManager final : public ISystem
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

	void SetTargetDeltaTime(float time);
	float GetTargetDeltaTime() const;
	void SetTargetSimulationHz(int hz);
	int GetTargetSimulationHz() const;
	float GetActualSimulationFrameTime() const;
	float GetActualSimulationHz() const;

	void PauseSimulation() { m_Pause = true; }
	void ResumeSimulation() { m_Pause = false; }
	bool IsSimulationPause() const { return m_Pause; }

	IntegrationType GetSelectedIntegration() const;
	void SetIntegration(IntegrationType type);

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

	int m_TargetSimulationHz{ 60 };
	float m_TargetDeltaTime{ 1.f / 60.f };
	float m_ActualSimulationFrameTime{ 0.0f };
	float m_ActualSimulationHz{ 0.0f };
	LocalTimer m_Timer{};
	bool m_Pause{ false };
	IntegrationType m_SelectedIntegration{ IntegrationType::SemiImplicitEuler };
};
