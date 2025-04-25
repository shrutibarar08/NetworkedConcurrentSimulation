#include "Application.h"
#include <iostream>

#include "Utils/Logger.h"

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
	{
		LOG_SUCCESS("Application events (Start/End) created successfully.");
	}
	else
	{
		LOG_FAIL("Failed to create application events.");
	}
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
	mWindowSystem->SetCreateThread(false); // dont need thread for ui.

	mSystemHandler.Register(
		"WindowSystem",
		mWindowSystem.get()
	);

	// Rendering Engine.
	mRenderer = std::make_unique<RenderManager>(mWindowSystem.get());
	mRenderer->SetGlobalEvent(&globalEvent);

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
	else
	{
		LOG_FAIL("Failed to initialize all systems.");
		return false;
	}
}

bool Application::Run()
{
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
	}
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
