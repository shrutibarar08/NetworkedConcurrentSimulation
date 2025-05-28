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

	ImGui::Text("Physics Manager");

	ImGui::Begin("Physics Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// === Target Simulation Frequency Control ===
	int simHz = m_PhysicsManager->GetTargetSimulationHz();
	if (ImGui::SliderInt("Target Simulation Hz", &simHz, 1, 240))
	{
		m_PhysicsManager->SetTargetSimulationHz(simHz);
	}

	// === Target Simulation Delta Time Control ===
	float simDelta = m_PhysicsManager->GetTargetDeltaTime();
	if (ImGui::SliderFloat("Target Delta Time (s)", &simDelta, 0.005f, .550f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
	{
		m_PhysicsManager->SetTargetDeltaTime(simDelta);
	}

	// === Actual Simulation Stats ===
	ImGui::Text("Actual Simulation Hz: %.1f", m_PhysicsManager->GetActualSimulationHz());
	ImGui::Text("Actual Step Time: %.2f ms", m_PhysicsManager->GetActualSimulationFrameTime() * 1000.0f);

	ImGui::Separator();

	// === Integration Method Selection ===
	static const char* integrationModes[] = { "Semi-Implicit Euler", "Euler", "Verlet" };
	int currentIndex = static_cast<int>(m_PhysicsManager->GetSelectedIntegration());

	if (ImGui::Combo("Integration Method", &currentIndex, integrationModes, IM_ARRAYSIZE(integrationModes)))
	{
		m_PhysicsManager->SetIntegration(static_cast<IntegrationType>(currentIndex));
	}

	// === Gravity Toggle ===
	bool gravityOn = m_PhysicsManager->GetGravity()->IsGravityOn();
	if (ImGui::Checkbox("Enable Gravity", &gravityOn))
	{
		m_PhysicsManager->GetGravity()->SetGravity(gravityOn);
	}

	if (ImGui::Button("Reverse Gravity"))
	{
		m_PhysicsManager->GetGravity()->ReverseGravity();
	}

	DirectX::XMFLOAT3 gravityVec{};
	DirectX::XMStoreFloat3(&gravityVec, m_PhysicsManager->GetGravity()->GetGravityForce());
	ImGui::Text("Gravity Force: (%.2f, %.2f, %.2f)", gravityVec.x, gravityVec.y, gravityVec.z);

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
	ImGui::Text("Total: %d", totalCount);

	ImGui::Separator();

	// === Simulation Pause/Resume ===
	bool isPaused = m_PhysicsManager->IsSimulationPause();
	if (ImGui::Checkbox("Pause Simulation", &isPaused))
	{
		isPaused ? m_PhysicsManager->PauseSimulation()
			: m_PhysicsManager->ResumeSimulation();
	}

	ImGui::End();
}
