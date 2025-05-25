#pragma once
#include "IWidget.h"
#include "ScenarioManager/Scene/Scene.h"

class SceneUI: public IWidget
{
public:
	SceneUI(Scene* scene);
	~SceneUI() override = default;
	void RenderMenu() override;
	void RenderOnScreen() override;
	std::string MenuName() const override;

private:
	void DisplayObjects() const;

private:
	bool m_ShowObjects{ false };
	Scene* m_Scene;
};
