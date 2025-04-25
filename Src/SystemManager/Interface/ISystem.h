#pragma once

#include <windows.h>
#include "FileManager/FileLoader/SweetLoader.h"

typedef struct SYSTEM_EVENT_HANDLE
{
    HANDLE GlobalStartEvent;
    HANDLE GlobalEndEvent;
}SYSTEM_EVENT_HANDLE;

class ISystem
{
public:
    ISystem() = default;
    virtual ~ISystem() = default;

    ISystem(const ISystem&) = delete;
    ISystem(ISystem&&) = delete;
    ISystem& operator=(const ISystem&) = delete;
    ISystem& operator=(ISystem&&) = delete;

    // Load initial state/config from some ConfigManager
    auto SetGlobalEvent(const SYSTEM_EVENT_HANDLE* eventHandles) -> void;

	bool Init();
	bool Shutdown();
    //~ Will be launched after initializing it. Use Event lock to prevent it.
    virtual bool Run();
    void SetCreateThread(bool status) { mCreateThread = status; }
    virtual bool Build(SweetLoader& sweetLoader) = 0;

    HANDLE GetThreadHandle() const;
    HANDLE GetInitializedEventHandle() const;

private:
    static DWORD __stdcall ThreadCall(LPVOID ptr);

protected:
    SYSTEM_EVENT_HANDLE mGlobalEvent;
    HANDLE mInitializedEventHandle;

    HANDLE mThreadHandle;
    bool mCreateThread{ true };
};
