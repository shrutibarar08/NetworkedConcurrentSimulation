#pragma once
#include <windows.h>

#include "SystemManager/Interface/ISystem.h"
#include "Core/DefineDefault.h"


class WindowsSystem final: public ISystem
{
public:
	WindowsSystem() = default;
	~WindowsSystem() override = default;

	WindowsSystem(const WindowsSystem&) = delete;
	WindowsSystem(WindowsSystem&&) = delete;
	WindowsSystem& operator=(const WindowsSystem&) = delete;
	WindowsSystem& operator=(WindowsSystem&&) = delete;

	bool Shutdown() override;
	bool Build(SweetLoader& sweetLoader) override;
	static int ProcessMethod();

private:
	bool InitParameters(SweetLoader& sweetLoader);
	bool InitWindowClass();

	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT HandleMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;

private:
	HWND mHandleWindow{ nullptr };
	HINSTANCE mHandleInstance{ nullptr };
	std::wstring mWindowName{};
	int mWindowWidth{};
	int mWindowHeight{};
};
