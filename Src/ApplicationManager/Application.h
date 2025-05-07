#pragma once

#include <memory>

#include "RenderManager/RenderManager.h"
#include "FileManager/FileLoader/SweetLoader.h"
#include "RenderManager/Model/Shapes/ModelCube.h"
#include "SystemManager/SystemHandler.h"
#include "WindowManager/WindowsSystem.h"

class Application
{
public:
	Application();
	~Application();

	Application(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(const Application&) = delete;
	Application& operator=(Application&&) = delete;

	bool Init();
	bool Run();
	void Shutdown();

private:
	SystemHandler m_SystemHandler{};
	std::unique_ptr<WindowsSystem> m_WindowSystem{ nullptr };
	std::unique_ptr<RenderManager> m_Renderer{ nullptr };
	SweetLoader mSweetLoader{};

	HANDLE m_StartEventHandle;
	HANDLE m_EndEventHandle;

	std::unique_ptr<ModelCube> m_Cube;
};
