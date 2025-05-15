#include "SceneUI.h"

#include "imgui.h"

SceneUI::SceneUI(Scene* scene)
	: m_Scene(scene)
{}

void SceneUI::RenderMenu()
{
	if (ImGui::BeginMenu(MenuName().c_str()))
	{
		if (ImGui::MenuItem("Reload Scene"))
			m_Scene->OnLoad();
		if (ImGui::MenuItem("Unload Scene"))
			m_Scene->OnOffLoad();
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
