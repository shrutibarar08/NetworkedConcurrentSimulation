#include "ISystem.h"

void ISystem::SetGlobalEvent(const SYSTEM_EVENT_HANDLE* eventHandles)
{
	mGlobalEvent.GlobalEndEvent = eventHandles->GlobalEndEvent;
	mGlobalEvent.GlobalStartEvent = eventHandles->GlobalStartEvent;
}

bool ISystem::Init()
{
	if (!mCreateThread) return true;
	mThreadHandle = CreateThread(
		nullptr,
		0,
		ThreadCall,
		this,
		0,
		nullptr
	);
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(mThreadHandle, THREAD_PRIORITY_TIME_CRITICAL);

	mInitializedEventHandle = CreateEvent(
		nullptr,
		TRUE,
		FALSE,
		nullptr
	);

	return mThreadHandle != nullptr;
}

bool ISystem::Shutdown()
{
	if (mGlobalEvent.GlobalEndEvent)
	{
		WaitForSingleObject(mGlobalEvent.GlobalEndEvent,
			INFINITE);
	}
	if (mThreadHandle)
	{
		CloseHandle(mThreadHandle);
		mThreadHandle = nullptr;
	}

	if (mInitializedEventHandle)
	{
		CloseHandle(mInitializedEventHandle);
		mInitializedEventHandle = nullptr;
	}

	return true;
}

bool ISystem::Run()
{
	SetEvent(mInitializedEventHandle);
	if (mGlobalEvent.GlobalStartEvent)
	{
		WaitForSingleObject(mGlobalEvent.GlobalStartEvent,
			INFINITE);
	}
	return true;
}

HANDLE ISystem::GetThreadHandle() const
{
	return mThreadHandle;
}

HANDLE ISystem::GetInitializedEventHandle() const
{
	return mInitializedEventHandle;
}

DWORD __stdcall ISystem::ThreadCall(LPVOID ptr)
{
	ISystem* system = static_cast<ISystem*>(ptr);
	return system->Run();
}
