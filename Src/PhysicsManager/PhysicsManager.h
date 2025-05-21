#pragma once
#include "SystemManager/Interface/ISystem.h"


class PhysicsManager final: public ISystem
{
public:
	bool Shutdown() override;
	bool Run() override;
	bool Build(SweetLoader& sweetLoader) override;
};

