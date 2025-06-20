#pragma once
#include "IWidget.h"
#include "RenderManager/RenderManager.h"


class RenderManagerUI: public IWidget
{
public:
	RenderManagerUI(RenderManager* renderer);
	virtual ~RenderManagerUI() override = default;
	std::string MenuName() const override { return "Render Settings"; }
	void RenderAsSystemItem() override;
	void RenderOnScreen() override;
	void RenderPopups() override;
	bool Init() override;

private:
	//~ Menus
	void DisplayDescription();

	//~ Popups
	void PopupDescription();
	void DrawRenderDetails() const;
	void DrawMSAAOptions() const;

private:
	RenderManager* m_RenderManager;
	bool m_RequestOpenDescription{ false };
};
