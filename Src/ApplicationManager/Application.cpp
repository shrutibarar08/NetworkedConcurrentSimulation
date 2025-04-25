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
	//~ Loading Configuration
	mWindowSystem = std::make_unique<WindowsSystem>();
	mWindowSystem->SetCreateThread(false); // dont need thread for ui.

	mSystemHandler.Register(
		"WindowSystem",
		mWindowSystem.get()
	);

	//~ Initializing Systems in correct order
	return mSystemHandler.BuildAll(mSweetLoader);
}

bool Application::Run()
{
	SetEvent(mStartEventHandle);

	while (true)
	{
		if (WindowsSystem::ProcessMethod())
		{
			SetEvent(mEndEventHandle);
			break;
		}
	}

	mSystemHandler.WaitAll();
	return true;
}

void Application::Shutdown()
{
	//~ Shut Down all systems
	mSystemHandler.ShutdownAll();
}
