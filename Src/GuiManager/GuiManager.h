#pragma once
#include "SystemManager/Interface/ISystem.h"
#include "Widgets/IWidget.h"


class GuiManager final: public ISystem
{
public:
	GuiManager();
	~GuiManager() override = default;

	bool Shutdown() override;
	bool Run() override;
	bool Build(SweetLoader& sweetLoader) override;

	void AddUI(std::unique_ptr<IWidget> widget);
	void AddUI(IWidget* widget);
	void ResizeViewport(float width, float height);

private:
	void BeginScene();
	void RenderScene();
	void EndScene();

private:
	SRWLOCK m_Lock;
	std::vector<std::unique_ptr<IWidget>> m_Widgets;
	std::vector<IWidget*> m_WidgetsSafe;
	bool m_RequestNewPopup = false;

	//~ Cache windows value
	float m_PrevWidth{ 0.f };
	float m_PrevHeight{ 0.0f };
};
