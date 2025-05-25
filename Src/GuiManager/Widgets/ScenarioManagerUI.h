#pragma once
#include "IWidget.h"
#include "ScenarioManager/ScenarioManager.h"

class ScenarioManagerUI final: public IWidget
{
public:
	ScenarioManagerUI(ScenarioManager* sceneManager);
	void RenderMenu() override;
	void RenderPopups() override;
	void RenderOnScreen() override;

	bool IsSceneExists(const std::string& name) const;

	std::string MenuName() const override;

private:
	void CreateWindowPopup();

private:
	ScenarioManager* m_ScenarioManager{ nullptr };

	bool m_CreateWindowPopup{ false };
};
