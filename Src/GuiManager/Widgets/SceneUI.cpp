#include "SceneUI.h"

#include <ranges>
#include "imgui.h"
#include "Utils/Logger.h"

SceneUI::SceneUI(Scene* scene)
	: m_Scene(scene)
{}

void SceneUI::RenderMenu()
{
	if (ImGui::BeginMenu("Add Object"))
	{
		if (ImGui::MenuItem("Add Cube"))
		{
            m_PopUpCreateObject = true;
            m_WhatToCreate = SPAWN_OBJECT::CUBE;
		}
        if (ImGui::MenuItem("Add Sphere"))
        {
            m_PopUpCreateObject = true;
            m_WhatToCreate = SPAWN_OBJECT::SPHERE;
        }
        if (ImGui::MenuItem("Add Capsule"))
        {
            m_PopUpCreateObject = true;
            m_WhatToCreate = SPAWN_OBJECT::CAPSULE;
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
	DisplayObjects();
}

void SceneUI::RenderPopups()
{
    DisplaySpawnerPop();
    DisplayCreateObjectPopup();
}

std::string SceneUI::MenuName() const
{
	return "Scene";
}

void SceneUI::DisplaySpawnerPop()
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

        ImGui::DragFloat("Min Radius", &p.minRadius, 0.01f, 0.1f, 10.0f);
        ImGui::DragFloat("Max Radius", &p.maxRadius, 0.01f, 0.1f, 10.0f);

        ImGui::DragFloat("Min Height", &p.minHeight, 0.01f, 0.1f, 10.0f);
        ImGui::DragFloat("Max Height", &p.maxHeight, 0.01f, 0.1f, 10.0f);

        ImGui::DragFloat("Min Width", &p.minWidth, 0.01f, 0.1f, 10.0f);
        ImGui::DragFloat("Max Width", &p.maxWidth, 0.01f, 0.1f, 10.0f);

        ImGui::DragFloat("Min Depth", &p.minDepth, 0.01f, 0.1f, 10.0f);
        ImGui::DragFloat("Max Depth", &p.maxDepth, 0.01f, 0.1f, 10.0f);

        ImGui::DragInt("Spawn Quantity", &p.quantity, 1, 1, 300);

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

void SceneUI::DisplayCreateObjectPopup()
{
    if (!m_PopUpCreateObject) return;

    ImGui::OpenPopup("Create Object");
    
    m_Payload.SpawnObject = m_WhatToCreate;

    if (ImGui::BeginPopupModal("Create Object", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Create a new %s", Scene::SpawnObjectToString(m_Payload.SpawnObject).c_str());
        ImGui::Separator();

        // === Position ===
        ImGui::DragFloat3("Position", &m_Payload.Position.x, 0.1f);

        // === Orientation (Quaternion) ===
        float r = m_Payload.Orientation.GetR();
        float i = m_Payload.Orientation.GetI();
        float j = m_Payload.Orientation.GetJ();
        float k = m_Payload.Orientation.GetK();

        bool changed = false;
        changed |= ImGui::DragFloat("Orientation R", &r, 0.01f);
        changed |= ImGui::DragFloat("Orientation I", &i, 0.01f);
        changed |= ImGui::DragFloat("Orientation J", &j, 0.01f);
        changed |= ImGui::DragFloat("Orientation K", &k, 0.01f);

        if (changed)
        {
            m_Payload.Orientation = Quaternion(r, i, j, k);
            m_Payload.Orientation.Normalize();
        }

        // === Velocity, Acceleration, Angular Velocity ===
        ImGui::DragFloat3("Velocity", &m_Payload.Velocity.x, 0.1f);
        ImGui::DragFloat3("Acceleration", &m_Payload.Acceleration.x, 0.1f);
        ImGui::DragFloat3("Angular Velocity", &m_Payload.AngularVelocity.x, 0.1f);

        // === Physics Scalars ===
        ImGui::DragFloat("Mass", &m_Payload.Mass, 0.1f, 0.001f, 1000.0f);
        ImGui::DragFloat("Elasticity", &m_Payload.Elasticity, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Restitution", &m_Payload.Restitution, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Friction", &m_Payload.Friction, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("Angular Damping", &m_Payload.AngularDamping, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Linear Damping", &m_Payload.LinearDamping, 0.01f, 0.0f, 1.0f);

        // === Spawn Time and Static flag ===
        ImGui::DragFloat("Spawn Delay (s)", &m_Payload.SpawnTime, 0.01f, 0.0f, 10.0f);
        ImGui::Checkbox("Is Static?", &m_Payload.Static);
        ImGui::Checkbox("Need Ui Control?", &m_Payload.UiControlNeeded);

        ImGui::Separator();

        if (ImGui::Button("Create"))
        {
            m_Scene->AddObject(m_Payload);
            ImGui::CloseCurrentPopup();
            m_PopUpCreateObject = false;
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
            m_PopUpCreateObject = false;
        }

        ImGui::EndPopup();
    }
}

void SceneUI::DisplayObjects() const
{
    if (ImGui::CollapsingHeader("Object Data", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (auto& object : m_Scene->GetModels() | std::views::values)
        {
            if (!object || !object->IsUiControlNeeded()) continue;

            if (IWidget* widget = object->GetWidget())
            {
                const int modelId = object->GetModelId();
                ImGui::PushID(modelId);

                std::string label = "Object##" + std::to_string(modelId);
                if (ImGui::CollapsingHeader(label.c_str()))
                {
                    widget->RenderOnScreen();
                }

                ImGui::PopID();
            }
        }

    }
}
