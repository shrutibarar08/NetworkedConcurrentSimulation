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
		if (ImGui::MenuItem("Create"))
		{
			m_CreateWindowPopup = true;
		}

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

void ScenarioManagerUI::RenderPopups()
{
	CreateWindowPopup();
}

void ScenarioManagerUI::RenderOnScreen()
{
}

bool ScenarioManagerUI::IsSceneExists(const std::string& name) const
{
    for (auto& scene: m_ScenarioManager->GetScenes())
    {
        if (scene->GetName() == name) return true;
    }
    return false;
}

std::string ScenarioManagerUI::MenuName() const
{
	return "Scenarios";
}

void ScenarioManagerUI::CreateWindowPopup()
{
    static char sceneName[128] = "";
    static bool nameExists = false;

    if (m_CreateWindowPopup)
    {
        ImGui::OpenPopup("CreateScene");
        m_CreateWindowPopup = false;
        nameExists = false;
        strcpy_s(sceneName, "");
    }

    if (ImGui::BeginPopupModal("CreateScene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter Scene Name:");
        ImGui::InputText("##SceneName", sceneName, IM_ARRAYSIZE(sceneName));

    	if (nameExists)
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Scene name already exists!");
        }

        if (ImGui::Button("OK"))
        {
            std::string nameStr(sceneName);

            if (IsSceneExists(nameStr))
            {
                nameExists = true;
            }
            else
            {
                m_ScenarioManager->CreateScene(nameStr);
                strcpy_s(sceneName, "");
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
