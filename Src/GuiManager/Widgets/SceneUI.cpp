#include "SceneUI.h"

#include "imgui.h"

SceneUI::SceneUI(Scene* scene)
	: m_Scene(scene)
{}

void SceneUI::RenderMenu()
{
	if (ImGui::BeginMenu("Add Object"))
	{
		if (ImGui::MenuItem("Add Cube"))
		{
			m_Scene->AddObject(SPAWN_OBJECT::CUBE);
		}
		ImGui::EndMenu();
	}
}

void SceneUI::RenderAsSystemItem()
{
}

void SceneUI::RenderPopups()
{
}

std::string SceneUI::MenuName() const
{
	return "Scene";
}
