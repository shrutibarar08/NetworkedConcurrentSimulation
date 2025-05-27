#include "ISystem.h"

#include "Utils/Logger.h"

void ISystem::SetGlobalEvent(const SYSTEM_EVENT_HANDLE* eventHandles)
{
	mGlobalEvent.GlobalEndEvent = eventHandles->GlobalEndEvent;
	mGlobalEvent.GlobalStartEvent = eventHandles->GlobalStartEvent;
}

bool ISystem::Init()
{
    if (!mCreateThread)
        return true;

    // === Create the thread ===
    mThreadHandle = CreateThread(
        nullptr,
        0,
        ThreadCall,
        this,
        0,
        nullptr
    );

    if (!mThreadHandle)
        return false;

    // === Apply Thread Priority if explicitly set ===
    if (m_ThreadPriority != THREAD_PRIORITY_NORMAL)
    {
        if (!SetThreadPriority(mThreadHandle, m_ThreadPriority))
        {
            LOG_WARNING("Failed to set thread priority]!");
        }
    }

    // === Apply Thread Affinity if explicitly set ===
    if (m_ThreadAffinityMask != 0)
    {
        if (!SetThreadAffinityMask(mThreadHandle, m_ThreadAffinityMask))
        {
            LOG_WARNING("Failed to use thread affinity!");
        }
    }

    // === Create the event for thread sync ===
    mInitializedEventHandle = CreateEvent(
        nullptr,
        TRUE,   // Manual reset
        FALSE,
        nullptr
    );

    return true;
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
