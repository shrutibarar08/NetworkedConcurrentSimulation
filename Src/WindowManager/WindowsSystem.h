#pragma once
#include <windows.h>

#include "SystemManager/Interface/ISystem.h"


class WindowsSystem final: public ISystem
{
public:
	WindowsSystem();
	~WindowsSystem() override = default;

	WindowsSystem(const WindowsSystem&) = delete;
	WindowsSystem(WindowsSystem&&) = delete;
	WindowsSystem& operator=(const WindowsSystem&) = delete;
	WindowsSystem& operator=(WindowsSystem&&) = delete;

	bool Build(SweetLoader& sweetLoader) override;
	bool Shutdown() override;
	static int ProcessMethod();

	float GetAspectRatio() const;
	bool IsFullScreen();
	void SetFullScreen(bool val);

	HWND GetWindowHandle() const;
	HINSTANCE GetWindowManagerInstance() const;

	int GetWindowsWidth() const { return m_WindowWidth; }
	int GetWindowsHeight() const { return m_WindowHeight; }

private:
	bool InitParameters(SweetLoader& sweetLoader);
	bool InitWindowClass();

	void ApplyFullScreen();
	void ApplyWindowedScreen() const;

	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT HandleMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;

private:
	HWND m_HandleWindow{ nullptr };
	HINSTANCE m_HandleInstance{ nullptr };
	std::wstring m_WindowName{};
	int m_WindowWidth{};
	int m_WindowHeight{};
	bool m_Fullscreen;
	WINDOWPLACEMENT m_WindowPlacement = { sizeof(m_WindowPlacement) };
	SRWLOCK m_Lock;
};
