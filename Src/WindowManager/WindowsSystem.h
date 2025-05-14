#pragma once
#include <windows.h>

#include "SystemManager/Interface/ISystem.h"

typedef struct RESOLUTION
{
	UINT Width;
	UINT Height;
}RESOLUTION;

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
	int ProcessMethod() const;

	float GetAspectRatio() const;
	bool IsFullScreen();
	void SetFullScreen(bool val);

	std::vector<RESOLUTION>& GetAvailableResolution();
	void UpdateResolution(const RESOLUTION* resolution);

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
	std::vector<RESOLUTION> m_PossibleResolution = {
		{640, 480},      // VGA (legacy)
		{800, 600},      // SVGA
		{1024, 768},     // XGA (4:3)
		{1280, 720},     // HD / 720p
		{1366, 768},     // WXGA
		{1440, 900},     // WXGA+ (16:10)
		{1600, 900},     // HD+ (16:9)
		{1680, 1050},    // WSXGA+ (16:10)
		{1920, 1080},    // Full HD / 1080p
		{1920, 1200},    // WUXGA (16:10)
		{2048, 1152},    // QWXGA
		{2560, 1080},    // Ultrawide FHD (21:9)
		{2560, 1440},    // QHD / 1440p
		{2560, 1600},    // WQXGA (16:10)
		{2880, 1620},    // QHD+ upscale
		{3440, 1440},    // Ultrawide QHD
		{3840, 1600},    // UWQHD+
		{3840, 2160},    // 4K UHD
		{5120, 1440},    // Dual QHD / Super Ultrawide
		{5120, 2160},    // 5K Ultrawide
		{5760, 1080},    // Triple-monitor 1080p
		{7680, 4320},    // 8K UHD
	};
};
