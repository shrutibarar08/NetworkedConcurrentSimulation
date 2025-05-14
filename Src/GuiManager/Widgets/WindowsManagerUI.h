#pragma once
#include "IWidget.h"
#include "WindowManager/WindowsSystem.h"


class WindowsManagerUI final : public IWidget
{
public:
	WindowsManagerUI(WindowsSystem* windowsSystem);
	~WindowsManagerUI() override = default;

	std::string MenuName() const override { return "Display Settings"; }

	bool Init() override;
	void RenderAsSystemItem() override;
	void RenderPopups() override;

private:
	void DisplayWindowSettings();
	void PopupWindowSettings();
	void DisplayWindowInfo();
	void DisplayFullscreenToggle();
	void DisplayResolutionSelector();

private:
	WindowsSystem* m_WindowsSystem;
	bool m_RequestDisplayPopup{ false };
	int m_SelectedResolutionIndex = 0;
};
