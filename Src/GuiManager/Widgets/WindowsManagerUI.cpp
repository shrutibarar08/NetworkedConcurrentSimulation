#include "WindowsManagerUI.h"

#include "imgui_impl_win32.h"
#include "EventSystem/EventQueue.h"

WindowsManagerUI::WindowsManagerUI(WindowsSystem* windowsSystem)
	: m_WindowsSystem(windowsSystem)
{}

bool WindowsManagerUI::Init()
{
	if (!m_WindowsSystem) return false;
	ImGui_ImplWin32_Init(m_WindowsSystem->GetWindowHandle());
	return true;
}

void WindowsManagerUI::RenderAsSystemItem()
{
	DisplayWindowInfo();
}

void WindowsManagerUI::RenderPopups()
{
	PopupWindowInfo();
}

void WindowsManagerUI::DisplayWindowInfo()
{
	if (ImGui::MenuItem("Show Window Info"))
	{
		m_RequestDisplayPopup = true;
	}
}

void WindowsManagerUI::PopupWindowInfo()
{
	if (m_RequestDisplayPopup)
	{
		ImGui::OpenPopup("WindowInfoPopup");
		m_RequestDisplayPopup = false;
	}

	if (ImGui::BeginPopupModal("WindowInfoPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		float aspectRatio = m_WindowsSystem->GetAspectRatio();
		bool isFullscreen = m_WindowsSystem->IsFullScreen();
		int width = m_WindowsSystem->GetWindowsWidth();
		int height = m_WindowsSystem->GetWindowsHeight();
		HWND hwnd = m_WindowsSystem->GetWindowHandle();
		HINSTANCE hinst = m_WindowsSystem->GetWindowManagerInstance();

		ImGui::Text("Window Resolution: %d x %d", width, height);
		ImGui::Text("Aspect Ratio: %.2f", aspectRatio);
		ImGui::Text("Fullscreen Mode: %s", isFullscreen ? "Enabled" : "Disabled");
		ImGui::Separator();

		// Fullscreen toggle button
		if (ImGui::Button(isFullscreen ? "Exit Fullscreen" : "Enter Fullscreen"))
		{
			if (!isFullscreen) EventQueue::Push(EventType::WINDOW_EVENT_FULLSCREEN);
			else EventQueue::Push(EventType::WINDOW_EVENT_WINDOWED);
		}

		ImGui::Separator();
		ImGui::Text("HWND: 0x%p", hwnd);
		ImGui::Text("HINSTANCE: 0x%p", hinst);

		ImGui::Spacing();
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}
