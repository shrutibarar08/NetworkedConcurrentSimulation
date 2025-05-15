#pragma once

#include <functional>
#include <memory>

#include "EventSystem/EventQueue.h"
#include "RenderManager/RenderManager.h"
#include "FileManager/FileLoader/SweetLoader.h"
#include "GuiManager/GuiManager.h"
#include "InputHandler/InputHandler.h"
#include "RenderManager/Model/Shapes/ModelCube.h"
#include "ScenarioManager/ScenarioManager.h"
#include "ScenarioManager/Scene/Scene.h"
#include "SystemManager/SystemHandler.h"
#include "WindowManager/WindowsSystem.h"


using EventHandler = std::function<void()>;

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
	void HandleEvents();
	void BuildEventHandler();

private:
	SystemHandler m_SystemHandler{};
	std::unique_ptr<WindowsSystem> m_WindowSystem{ nullptr };
	std::unique_ptr<RenderManager> m_Renderer{ nullptr };
	std::unique_ptr<GuiManager> m_GuiManager{ nullptr };
	std::unique_ptr<InputHandler> m_InputHandler{ nullptr };
	std::unique_ptr<ScenarioManager> m_ScenarioManager{ nullptr };

	SweetLoader mSweetLoader{};

	std::unordered_map<EventType, EventHandler> m_EventHandlers;

	HANDLE m_StartEventHandle;
	HANDLE m_EndEventHandle;
};
