#include "Application.h"
#include <iostream>

#include "Utils/Logger.h"
#include "Utils/Helper.h"
#include "WindowManager/Components/KeyboardHandler.h"

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

	MODEL_INIT_DESC desc{};
	desc.ModelName = "Cube Model";
	desc.VertexShaderPath = "Shaders/CubeShader/CubeVS.hlsl";
	desc.PixelShaderPath = "Shaders/CubeShader/CubePS.hlsl";
	m_Cube = std::make_unique<ModelCube>(&desc);
}

Application::~Application()
{
	ResetEvent(m_StartEventHandle);
	SetEvent(m_EndEventHandle);
	Shutdown();
	CloseHandle(m_EndEventHandle);
	CloseHandle(m_StartEventHandle);

	LOG_INFO("Application shutdown sequence completed.");
}

bool Application::Init()
{
	LOG_INFO("Application initialization started.");

	SYSTEM_EVENT_HANDLE globalEvent;
	globalEvent.GlobalStartEvent = m_StartEventHandle;
	globalEvent.GlobalEndEvent = m_EndEventHandle;

	//~ Loading Configuration
	m_WindowSystem = std::make_unique<WindowsSystem>();

	m_SystemHandler.Register(
		"WindowSystem",
		m_WindowSystem.get()
	);

	// Rendering Engine.
	m_Renderer = std::make_unique<RenderManager>(m_WindowSystem.get());

	m_SystemHandler.Register("RenderManager", m_Renderer.get());
	m_SystemHandler.AddDependency(
		"RenderManager",
		"WindowSystem"	// RenderManager Depends upon.
	);

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
	//~ Test only
	m_Renderer->BuildModel(m_Cube.get());
	Render3DQueue::AddModel(m_Cube.get());

	//~ Test End
	LOG_INFO("Application main loop starting.");
	m_SystemHandler.WaitStart(); 
	SetEvent(m_StartEventHandle);
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
			if (m_WindowSystem->IsFullScreen())
			{
				m_WindowSystem->SetFullScreen(false);
				m_Renderer->ResizeSwapChain();
			}
			else
			{
				m_WindowSystem->SetFullScreen(true);
				m_Renderer->ResizeSwapChain();
			}
		}

		if (WindowsSystem::ProcessMethod())
		{
			SetEvent(m_EndEventHandle);
			break;
		}
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
