#pragma once
#include "ICollider.h"
#include "ApplicationManager/Clock/SystemClock.h"
#include "SystemManager/Interface/ISystem.h"
#include "IntegrationType.h"

class IModel;

class PhysicsManager final: public ISystem
{
public:
	PhysicsManager() = default;
	bool Shutdown() override;
	bool Run() override;
	bool Build(SweetLoader& sweetLoader) override;

	bool AddRigidBody(const IModel* model);
	bool RemoveRigidBody(ID id);
	bool HasRigidBody(uint64_t id);

private:
	void Update(float dt, IntegrationType type = IntegrationType::SemiImplicitEuler);

private:
	SRWLOCK m_Lock{ SRWLOCK_INIT };

	std::unordered_map<uint64_t, ICollider*> m_PhysicsEntity;
};
