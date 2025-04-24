#pragma once
#include "SystemManager/Interface/ISystem.h"


class WindowsSystem final: public ISystem
{
public:
	WindowsSystem() = default;
	~WindowsSystem() override = default;

	WindowsSystem(const WindowsSystem&) = delete;
	WindowsSystem(WindowsSystem&&) = delete;
	WindowsSystem& operator=(const WindowsSystem&) = delete;
	WindowsSystem& operator=(WindowsSystem&&) = delete;

	bool Init() override;
	bool Shutdown() override;
	bool Run() override;
	bool BuildFromConfig(const SweetLoader* sweetLoader) override;
};
