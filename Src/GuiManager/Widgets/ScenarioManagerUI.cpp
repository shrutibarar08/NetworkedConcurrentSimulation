#include "ScenarioManagerUI.h"

#include "imgui.h"

ScenarioManagerUI::ScenarioManagerUI(ScenarioManager* sceneManager)
{
	m_ScenarioManager = sceneManager;
}

void ScenarioManagerUI::RenderMenu()
{
	if (ImGui::BeginMenu("Scenarios"))
	{
		for (const auto& scene: m_ScenarioManager->GetScenes())
		{
			if (ImGui::MenuItem(scene->GetName().c_str()))
			{
				if (!scene->IsLoaded())
				{
					m_ScenarioManager->OnLoad(scene);
				}
				else
				{
					m_ScenarioManager->OffLoad(scene);
				}
			}
		}
		ImGui::EndMenu();
	}
}

void ScenarioManagerUI::RenderAsSystemItem()
{
}

void ScenarioManagerUI::RenderPopups()
{
}

std::string ScenarioManagerUI::MenuName() const
{
	return "Scenarios";
}
