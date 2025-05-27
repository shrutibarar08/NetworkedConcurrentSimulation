#include "PhysicsManagerUI.h"
#include "imgui.h"


PhysicsManagerUI::PhysicsManagerUI(PhysicsManager* physicsManager)
	: m_PhysicsManager(physicsManager)
{
}

void PhysicsManagerUI::RenderAsSystemItem()
{
	if (ImGui::MenuItem("Physics Settings"))
	{
		m_PopupPhysicsSettings = !m_PopupPhysicsSettings;
	}
}

std::string PhysicsManagerUI::MenuName() const
{
	return "Display Physics Settings";
}

void PhysicsManagerUI::RenderOnScreen()
{
	if (!m_PopupPhysicsSettings) return;


    if (!m_PhysicsManager || !m_PhysicsManager->GetGravity()) return;

    // Section for Physics Settings
    if (ImGui::CollapsingHeader("Physics Manager"))
    {
        // === Gravity Toggle ===
        bool gravityOn = m_PhysicsManager->GetGravity()->IsGravityOn();
        if (ImGui::Checkbox("Enable Gravity", &gravityOn))
        {
            m_PhysicsManager->GetGravity()->SetGravity(gravityOn);
        }

        ImGui::Separator();

        // === Object Counts ===
        int capsuleCount = m_PhysicsManager->GetCapsuleCounts();
        int sphereCount = m_PhysicsManager->GetSphereCounts();
        int cubeCount = m_PhysicsManager->GetCubeCounts();
        int totalCount = m_PhysicsManager->GetTotalCounts();

        ImGui::Text("Object Counts:");
        ImGui::BulletText("Capsules: %d", capsuleCount);
        ImGui::BulletText("Spheres:  %d", sphereCount);
        ImGui::BulletText("Cubes:    %d", cubeCount);
        ImGui::Separator();
        ImGui::Text("Total: %d", totalCount);
    }
}
