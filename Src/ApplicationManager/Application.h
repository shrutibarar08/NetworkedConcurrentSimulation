#pragma once

#include <memory>

#include "FileManager/FileLoader/SweetLoader.h"
#include "SystemManager/DependencyHandler.h"
#include "SystemManager/Interface/ISystem.h"
#include "WindowManager/WindowsSystem.h"

class Application: public ISystem
{
public:
	Application() = default;
	~Application() override = default;

	bool Init() override;
	bool Run() override;
	bool Shutdown() override;
	bool BuildFromConfig(const SweetLoader* sweetLoader) override;

private:
	DependencyHandler mDependencyHandler{};
	std::unique_ptr<WindowsSystem> mWindowSystem{ nullptr };
	SweetLoader mSweetLoader{};
};
