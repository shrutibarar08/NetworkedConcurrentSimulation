#include "Application.h"
#include <iostream>


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
}

Application::~Application()
{
	ResetEvent(mStartEventHandle);
	SetEvent(mEndEventHandle);
	Shutdown();
	CloseHandle(mEndEventHandle);
	CloseHandle(mStartEventHandle);
}

bool Application::Init()
{
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
	return mSystemHandler.BuildAll(mSweetLoader);
}

bool Application::Run()
{
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
	return true;
}

void Application::Shutdown()
{
	//~ Shut Down all systems
	mSystemHandler.ShutdownAll();
}
