#pragma once
#include "IWidget.h"
#include "ScenarioManager/Scene/Scene.h"

class SceneUI: public IWidget
{
public:
	SceneUI(Scene* scene);
	~SceneUI() override = default;
	void RenderMenu() override;
	void RenderAsSystemItem() override;
	void RenderPopups() override;
	std::string MenuName() const override;

private:
	Scene* m_Scene;
};
