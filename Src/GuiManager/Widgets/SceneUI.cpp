#include "SceneUI.h"

#include <ranges>
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
        if (ImGui::MenuItem("Add Sphere"))
        {
            m_Scene->AddObject(SPAWN_OBJECT::SPHERE);
        }
        if (ImGui::MenuItem("Add Capsule"))
        {
            m_Scene->AddObject(SPAWN_OBJECT::CAPSULE);
        }
		ImGui::EndMenu();
	}

	if (ImGui::MenuItem("View Objects"))
	{
		m_ShowObjects = !m_ShowObjects;
	}
}

void SceneUI::RenderOnScreen()
{
	DisplayObjects();
}

std::string SceneUI::MenuName() const
{
	return "Scene";
}

void SceneUI::DisplayObjects() const
{
    for (auto& object : m_Scene->GetModels() | std::views::values)
    {
        if (!object) continue;

        if (IWidget* widget = object->GetWidget())
        {
            ImGui::PushID(object); // Unique ID for each widget section

            // Optional: get a name or type for header
            std::string name = object->GetName(); // Assuming you have GetName()
            if (name.empty()) name = "Unnamed Model";

            // Add spacing between model UIs
            ImGui::Spacing();
            ImGui::Spacing();

            // Frame it as a collapsing section
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.5f, 0.7f, 1.0f)); // Optional styling
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.6f, 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));

            if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));

                // Draw a framed child window to group the content
                ImGui::BeginChild("ModelContent", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
                widget->RenderOnScreen(); // Renders the RigidBody/Collider customization
                ImGui::EndChild();

                ImGui::PopStyleVar(2);
            }

            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }
    }

}
