#include "Application.h"

#include <iostream>

#include "FileManager/FileLoader/FileSystem.h"

bool Application::Run()
{
	return true;
}

bool Application::Init()
{
	//~ Loading Configuration
	mWindowSystem = std::make_unique<WindowsSystem>();

	mDependencyHandler.Register(
		"WindowSystem", 
		mWindowSystem.get()
	);

	//~ Initializing Systems in correct order
	return mDependencyHandler.InitAll();
}

bool Application::Shutdown()
{
	return mDependencyHandler.ShutdownAll();
}

bool Application::BuildFromConfig(const SweetLoader* sweetLoader)
{
	return false;
}
