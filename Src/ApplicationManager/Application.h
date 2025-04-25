#pragma once

#include <memory>

#include "FileManager/FileLoader/SweetLoader.h"
#include "SystemManager/SystemHandler.h"
#include "SystemManager/Interface/ISystem.h"
#include "WindowManager/WindowsSystem.h"

class Application
{
public:
	Application();
	~Application();

	bool Init();
	bool Run();
	void Shutdown();

private:
	SystemHandler mSystemHandler{};
	std::unique_ptr<WindowsSystem> mWindowSystem{ nullptr };
	SweetLoader mSweetLoader{};

	HANDLE mStartEventHandle;
	HANDLE mEndEventHandle;
};
