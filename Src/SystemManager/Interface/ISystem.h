#pragma once

#include "Core/DefineDefault.h"
#include <windows.h>
#include "FileManager/FileLoader/SweetLoader.h"
#include "GuiManager/Widgets/IWidget.h"


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
	virtual bool Shutdown();
    //~ Will be launched after initializing it. Use Event lock to prevent it.
    virtual bool Run();
    void CreateOnThread(bool status) { mCreateThread = status; }
    virtual bool Build(SweetLoader& sweetLoader) = 0;

    HANDLE GetThreadHandle() const;
    HANDLE GetInitializedEventHandle() const;

    void SetWidget(std::unique_ptr<IWidget> widget)
    {
        m_Widget = std::move(widget);
    }
    IWidget* GetWidget() const { return m_Widget.get(); }

private:
    static DWORD __stdcall ThreadCall(LPVOID ptr);

protected:
    SYSTEM_EVENT_HANDLE mGlobalEvent;
    HANDLE mInitializedEventHandle;

    HANDLE mThreadHandle;
    bool mCreateThread{ false };
    std::unique_ptr<IWidget> m_Widget{ nullptr };
};
