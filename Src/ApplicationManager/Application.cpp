#include "Application.h"
#include <iostream>

#include "Utils/Logger.h"
#include "Utils/Helper.h"

Application::Application()
{
	mStartEventHandle = CreateEvent(
		nullptr,
		TRUE,
		FALSE,
		nullptr);

	mEndEventHandle = CreateEvent(
		nullptr,
		TRUE,
		FALSE,
		nullptr
	);

	if (mStartEventHandle && mEndEventHandle) 
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
	ResetEvent(mStartEventHandle);
	SetEvent(mEndEventHandle);
	Shutdown();
	CloseHandle(mEndEventHandle);
	CloseHandle(mStartEventHandle);

	LOG_INFO("Application shutdown sequence completed.");
}

bool Application::Init()
{
	LOG_INFO("Application initialization started.");

	SYSTEM_EVENT_HANDLE globalEvent;
	globalEvent.GlobalStartEvent = mStartEventHandle;
	globalEvent.GlobalEndEvent = mEndEventHandle;

	//~ Loading Configuration
	mWindowSystem = std::make_unique<WindowsSystem>();

	mSystemHandler.Register(
		"WindowSystem",
		mWindowSystem.get()
	);

	// Rendering Engine.
	mRenderer = std::make_unique<RenderManager>(mWindowSystem.get());

	mSystemHandler.Register("RenderManager", mRenderer.get());
	mSystemHandler.AddDependency(
		"RenderManager",
		"WindowSystem"	// RenderManager Depends upon.
	);

	//~ Initializing Systems in correct order
	if (mSystemHandler.BuildAll(mSweetLoader))
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
	mRenderer->BuildModel(m_Cube.get());
	Render3DQueue::AddModel(m_Cube.get());

	//~ Test End
	LOG_INFO("Application main loop starting.");
	mSystemHandler.WaitStart(); 
	SetEvent(mStartEventHandle);
	while (true)
	{
		if (WindowsSystem::ProcessMethod())
		{
			SetEvent(mEndEventHandle);
			break;
		}
		mRenderer->Run();
	}
	std::cout << "Waiting for Finishing\n";
	mSystemHandler.WaitFinish();

	LOG_SUCCESS("Application main loop finished.");
	return true;
}

void Application::Shutdown()
{
	//~ Shut Down all systems
	LOG_INFO("Shutting down all systems.");
	mSystemHandler.ShutdownAll();
}
