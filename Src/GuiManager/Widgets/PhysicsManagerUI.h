#pragma once
#include "IWidget.h"

#include "PhysicsManager/PhysicsManager.h"

class PhysicsManagerUI: public IWidget
{
public:
	PhysicsManagerUI(PhysicsManager* physicsManager);
	void RenderAsSystemItem() override;
	std::string MenuName() const override;
	void RenderOnScreen() override;

private:
	PhysicsManager* m_PhysicsManager{ nullptr };
	bool m_PopupPhysicsSettings{ true };
};

