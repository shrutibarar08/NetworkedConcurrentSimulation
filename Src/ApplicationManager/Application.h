#pragma once

#include <memory>

#include "RenderManager/RenderManager.h"
#include "FileManager/FileLoader/SweetLoader.h"
#include "SystemManager/SystemHandler.h"
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
	std::unique_ptr<RenderManager> mRenderer{ nullptr };
	SweetLoader mSweetLoader{};

	HANDLE mStartEventHandle;
	HANDLE mEndEventHandle;
};
