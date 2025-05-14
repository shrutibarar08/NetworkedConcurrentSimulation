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
	DisplayWindowSettings();
}

void WindowsManagerUI::RenderPopups()
{
	PopupWindowSettings();
}

void WindowsManagerUI::DisplayWindowSettings()
{
	if (ImGui::MenuItem("Show Window Info"))
	{
		m_RequestDisplayPopup = true;
	}
}

void WindowsManagerUI::PopupWindowSettings()
{
    if (m_RequestDisplayPopup)
    {
        ImGui::OpenPopup("WindowInfoPopup");
        m_RequestDisplayPopup = false;
    }

    if (ImGui::BeginPopupModal("WindowInfoPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        DisplayWindowInfo();
        DisplayFullscreenToggle();
        DisplayResolutionSelector();

        ImGui::Spacing();
        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

void WindowsManagerUI::DisplayWindowInfo()
{
    float aspectRatio = m_WindowsSystem->GetAspectRatio();
    int width =         m_WindowsSystem->GetWindowsWidth();
    int height =        m_WindowsSystem->GetWindowsHeight();
    bool isFullscreen = m_WindowsSystem->IsFullScreen();

    ImGui::Text("Window Resolution: %d x %d", width, height);
    ImGui::Text("Aspect Ratio: %.2f", aspectRatio);
    ImGui::Text("Fullscreen Mode: %s", isFullscreen ? "Enabled" : "Disabled");
    ImGui::Separator();
}

void WindowsManagerUI::DisplayFullscreenToggle()
{
    bool isFullscreen = m_WindowsSystem->IsFullScreen();

    if (ImGui::Button(isFullscreen ? "Exit Fullscreen" : "Enter Fullscreen"))
    {
        if (!isFullscreen)
            EventQueue::Push(EventType::WINDOW_EVENT_FULLSCREEN);
        else
            EventQueue::Push(EventType::WINDOW_EVENT_WINDOWED);
    }
}

void WindowsManagerUI::DisplayResolutionSelector()
{
    static std::string preview;
    auto& resolutions = m_WindowsSystem->GetAvailableResolution();

    // Create preview string
    if (m_SelectedResolutionIndex >= 0 && m_SelectedResolutionIndex < resolutions.size())
    {
        const RESOLUTION& res = resolutions[m_SelectedResolutionIndex];
        preview = std::to_string(res.Width) + " x " + std::to_string(res.Height);
    }

    if (ImGui::BeginCombo("Resolution", preview.c_str()))
    {
        for (int i = 0; i < resolutions.size(); ++i)
        {
            const RESOLUTION& res = resolutions[i];
            std::string label = std::to_string(res.Width) + " x " + std::to_string(res.Height);
            bool isSelected = (i == m_SelectedResolutionIndex);

            if (ImGui::Selectable(label.c_str(), isSelected))
            {
                m_SelectedResolutionIndex = i;
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Apply Resolution"))
    {
        m_WindowsSystem->UpdateResolution(&resolutions[m_SelectedResolutionIndex]);
    }
}
