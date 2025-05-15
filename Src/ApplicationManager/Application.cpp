#include "Application.h"
#include <iostream>

#include "Clock/SystemClock.h"
#include "EventSystem/EventQueue.h"
#include "GuiManager/Widgets/InputHandlerUI.h"
#include "GuiManager/Widgets/RenderManagerUI.h"
#include "GuiManager/Widgets/WindowsManagerUI.h"
#include "Utils/Logger.h"
#include "Utils/Helper.h"
#include "WindowManager/Components/KeyboardHandler.h"
#include "WindowManager/Components/MouseHandler.h"

Application::Application()
{
	m_StartEventHandle = CreateEvent(
		nullptr,
		TRUE,
		FALSE,
		nullptr);

	m_EndEventHandle = CreateEvent(
		nullptr,
		TRUE,
		FALSE,
		nullptr
	);

	if (m_StartEventHandle && m_EndEventHandle) 
		LOG_SUCCESS("Application events (Start/End) created successfully.");
	else LOG_FAIL("Failed to create application events.");
}

Application::~Application()
{
	ResetEvent(m_StartEventHandle);
	SetEvent(m_EndEventHandle);
	Shutdown();
	EventQueue::Shutdown();
	CloseHandle(m_EndEventHandle);
	CloseHandle(m_StartEventHandle);

	LOG_INFO("Application shutdown sequence completed.");
}

bool Application::Init()
{
	LOG_INFO("Application initialization started.");
	EventQueue::Init();
	BuildEventHandler();

	SYSTEM_EVENT_HANDLE globalEvent;
	globalEvent.GlobalStartEvent = m_StartEventHandle;
	globalEvent.GlobalEndEvent = m_EndEventHandle;

	//~ Loading Configuration
	m_WindowSystem = std::make_unique<WindowsSystem>();
	m_SystemHandler.Register(
		"WindowsSystem",
		m_WindowSystem.get()
	);

	// Rendering Engine.
	m_Renderer = std::make_unique<RenderManager>(m_WindowSystem.get());
	m_SystemHandler.Register("RenderManager", m_Renderer.get());
	m_SystemHandler.AddDependency(
		"RenderManager",
		"WindowsSystem"	// RenderManager Depends upon it.
	);

	//~ Creating Input Handler
	m_InputHandler = std::make_unique<InputHandler>();
	m_InputHandler->AttachCamera(m_Renderer->GetActiveCamera());
	m_InputHandler->AttachWindows(m_WindowSystem.get());

	m_SystemHandler.Register("InputHandler", m_InputHandler.get());
	m_SystemHandler.AddDependency("InputHandler",
		"WindowsSystem",
		"RenderManager");

	//~ Gui Manager
	m_GuiManager = std::make_unique<GuiManager>();
	m_GuiManager->AddUI(m_Renderer->GetWidget());
	m_GuiManager->AddUI(m_WindowSystem->GetWidget());
	m_GuiManager->AddUI(m_InputHandler->GetWidget());

	m_SystemHandler.Register("GuiManager", m_GuiManager.get());
	m_SystemHandler.AddDependency(
		"GuiManager",
		"WindowsSystem",
		"RenderManager");

	//~ Scene Manager
	m_ScenarioManager = std::make_unique<ScenarioManager>();
	m_ScenarioManager->AttachUiRep(m_GuiManager.get());

	m_SystemHandler.Register("ScenarioManager", m_ScenarioManager.get());
	m_SystemHandler.AddDependency(
		"ScenarioManager",
		"WindowsSystem",
		"RenderManager",
		"GuiManager");

	//~ Initializing Systems in correct order
	if (m_SystemHandler.BuildAll(mSweetLoader))
	{
		LOG_SUCCESS("All systems initialized successfully.");
		return true;
	}
	LOG_FAIL("Failed to initialize all systems.");
	return false;
}

bool Application::Run()
{
	m_ScenarioManager->CreateScene("Test Scene_1");
	m_ScenarioManager->CreateScene("Test Scene_2");
	m_ScenarioManager->CreateScene("Test Scene_3");

	LOG_INFO("Application main loop starting.");
	m_SystemHandler.WaitStart(); 
	SetEvent(m_StartEventHandle);

	SystemClock::Start();

	while (true)
	{
		if (KeyboardHandler::IsKeyDown(VK_ESCAPE))
		{
			PostQuitMessage(0);
			SetEvent(m_EndEventHandle);
			break;
		}

		if (KeyboardHandler::IsKeyDown(VK_RETURN) 
			&& KeyboardHandler::IsAltPressedOnLastKey())
		{
			EventQueue::Push(EventType::WINDOW_EVENT_SCREEN_TOGGLE);
		}

		m_InputHandler->HandleInput();
		HandleEvents();

		if (m_WindowSystem->ProcessMethod())
		{
			SetEvent(m_EndEventHandle);
			break;
		}
		m_GuiManager->Run();
		m_Renderer->Run();
	}
	std::cout << "Waiting for Finishing\n";
	m_SystemHandler.WaitFinish();

	LOG_SUCCESS("Application main loop finished.");
	return true;
}

void Application::Shutdown()
{
	//~ Shut Down all systems
	LOG_INFO("Shutting down all systems.");
	m_SystemHandler.ShutdownAll();
}

void Application::HandleEvents()
{
	while (!EventQueue::IsEmpty())
	{
		EventType type;
		EventQueue::Pop(type);

		auto it = m_EventHandlers.find(type);
		if (it != m_EventHandlers.end())
		{
			it->second();
		}else
		{
			LOG_WARNING("UNKNOWN EVENT!");
		}
	}
}

void Application::BuildEventHandler()
{
	m_EventHandlers[EventType::WINDOW_EVENT_FULLSCREEN] = [this]()
	{
		LOG_INFO("Popped FullScreen Event");
		if (!m_WindowSystem->IsFullScreen())
		{
			m_WindowSystem->SetFullScreen(true);
			m_Renderer->ResizeSwapChain();
			m_GuiManager->ResizeViewport(
				m_WindowSystem->GetWindowsWidth(),
				m_WindowSystem->GetWindowsHeight());
		}
	};

	m_EventHandlers[EventType::WINDOW_EVENT_WINDOWED] = [this]()
	{
		LOG_INFO("Popped Windowed Event");
		if (m_WindowSystem->IsFullScreen())
		{
			m_WindowSystem->SetFullScreen(false);
			m_Renderer->ResizeSwapChain();
			m_GuiManager->ResizeViewport(
				m_WindowSystem->GetWindowsWidth(),
				m_WindowSystem->GetWindowsHeight());
		}
	};

	m_EventHandlers[EventType::WINDOW_EVENT_SCREEN_TOGGLE] = [this]()
	{
		LOG_INFO("Popped Toggle Screen Event");
		bool fs = !m_WindowSystem->IsFullScreen();
		m_WindowSystem->SetFullScreen(fs);
		m_Renderer->ResizeSwapChain();
		m_GuiManager->ResizeViewport(
			m_WindowSystem->GetWindowsWidth(),
			m_WindowSystem->GetWindowsHeight());
	};

	m_EventHandlers[EventType::RENDER_EVENT_RESIZE] = [this]()
	{
		LOG_INFO("Popped Window Resize Event");
		m_Renderer->ResizeSwapChain(true);
		m_GuiManager->ResizeViewport(
			m_WindowSystem->GetWindowsWidth(),
			m_WindowSystem->GetWindowsHeight());
	};
}
