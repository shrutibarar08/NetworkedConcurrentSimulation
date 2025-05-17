#pragma once
#include "IWidget.h"
#include "ScenarioManager/ScenarioManager.h"

class ScenarioManagerUI final: public IWidget
{
public:
	ScenarioManagerUI(ScenarioManager* sceneManager);
	void RenderMenu() override;
	void RenderAsSystemItem() override;
	void RenderPopups() override;
	std::string MenuName() const override;

private:
	ScenarioManager* m_ScenarioManager{ nullptr };
};
