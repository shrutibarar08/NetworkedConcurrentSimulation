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

    if (ImGui::BeginMenu("Utility"))
    {
        if (ImGui::MenuItem("View Objects"))
        {
            m_ShowObjects = !m_ShowObjects;
        }
        if (ImGui::MenuItem("Open Spawner"))
        {
            m_PopUpSpawner = true;
        }
        ImGui::EndMenu();
    }
}

void SceneUI::RenderOnScreen()
{
	// DisplayObjects();
}

void SceneUI::RenderPopups()
{
    if (m_PopUpSpawner)
    {
        ImGui::OpenPopup("Auto Object Spawner");
        m_PopUpSpawner = false;
    }

    if (ImGui::BeginPopupModal("Auto Object Spawner", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        CREATE_SCENE_PAYLOAD& p = m_ScenePayload;

        ImGui::Text("Spawn Position Range:");
        ImGui::DragFloat3("Min Position", &p.minPosition.x, 0.1f);
        ImGui::DragFloat3("Max Position", &p.maxPosition.x, 0.1f);

        ImGui::Text("Velocity Range:");
        ImGui::DragFloat3("Min Velocity", &p.minVelocity.x, 0.1f);
        ImGui::DragFloat3("Max Velocity", &p.maxVelocity.x, 0.1f);

        ImGui::Text("Acceleration Range:");
        ImGui::DragFloat3("Min Acceleration", &p.minAcceleration.x, 0.1f);
        ImGui::DragFloat3("Max Acceleration", &p.maxAcceleration.x, 0.1f);

        ImGui::Text("Angular Velocity Range:");
        ImGui::DragFloat3("Min Angular Velocity", &p.minAngularVelocity.x, 0.1f);
        ImGui::DragFloat3("Max Angular Velocity", &p.maxAngularVelocity.x, 0.1f);

        ImGui::DragFloat("Min Mass", &p.minMass, 0.1f, 0.01f, 100.f);
        ImGui::DragFloat("Max Mass", &p.maxMass, 0.1f, 0.01f, 100.f);

        ImGui::DragFloat("Min Elasticity", &p.minElasticity, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Max Elasticity", &p.maxElasticity, 0.01f, 0.0f, 1.0f);

        ImGui::DragFloat("Min Restitution", &p.minRestitution, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Max Restitution", &p.maxRestitution, 0.01f, 0.0f, 1.0f);

        ImGui::DragFloat("Min Friction", &p.minFriction, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Max Friction", &p.maxFriction, 0.01f, 0.0f, 1.0f);

        ImGui::DragFloat("Min Angular Damping", &p.minAngularDamping, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Max Angular Damping", &p.maxAngularDamping, 0.01f, 0.0f, 1.0f);

        ImGui::DragFloat("Min Linear Damping", &p.minLinearDamping, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Max Linear Damping", &p.maxLinearDamping, 0.01f, 0.0f, 1.0f);

        ImGui::DragInt("Spawn Quantity", &p.quantity, 1, 1, 1000);

        ImGui::Text("Types to Spawn:");
        ImGui::Checkbox("Cube", &p.spawnCube);
        ImGui::Checkbox("Sphere", &p.spawnSphere);
        ImGui::Checkbox("Capsule", &p.spawnCapsule);
        ImGui::DragFloat("Delta Spawn Time (s)", &p.deltaSpawnTime, 0.01f, 0.0f, 10.0f);

        ImGui::Separator();

        if (ImGui::Button("Spawn Objects"))
        {
        	if (m_Scene) m_Scene->AutoSpawn(m_ScenePayload);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
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
        if (object->GetCollider()->GetColliderState() != ColliderSate::Static) return;

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
