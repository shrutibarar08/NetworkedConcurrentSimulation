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
	void DisplayWindowInfo();
	void PopupWindowInfo();

private:
	WindowsSystem* m_WindowsSystem;
	bool m_RequestDisplayPopup{ false };
};
