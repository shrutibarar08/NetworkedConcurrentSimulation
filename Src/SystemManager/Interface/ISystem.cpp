#include "ISystem.h"

void ISystem::SetEvent(const SYSTEM_EVENT_HANDLE* eventHandles)
{
	mStartEventHandle = eventHandles->StartEvent;
	mEndEventHandle = eventHandles->EndEvent;
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
	return mThreadHandle != nullptr;
}

bool ISystem::Shutdown()
{
	if (mEndEventHandle)
	{
		WaitForSingleObject(mEndEventHandle, INFINITE);
	}
	if (mThreadHandle)
	{
		CloseHandle(mThreadHandle);
		mThreadHandle = nullptr;
	}
	return true;
}

bool ISystem::Run()
{
	if (mStartEventHandle)
	{
		WaitForSingleObject(mStartEventHandle, INFINITE);
	}
	return true;
}

HANDLE ISystem::GetThreadHandle() const
{
	return mThreadHandle;
}

DWORD __stdcall ISystem::ThreadCall(LPVOID ptr)
{
	ISystem* system = static_cast<ISystem*>(ptr);
	return system->Run();
}
