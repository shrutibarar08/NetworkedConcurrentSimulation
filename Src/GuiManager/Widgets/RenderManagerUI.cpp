#include "RenderManagerUI.h"
#include "imgui_impl_dx11.h"


RenderManagerUI::RenderManagerUI(RenderManager* renderer)
	: m_RenderManager(renderer)
{}

void RenderManagerUI::RenderAsSystemItem()
{
	DisplayDescription();
}

void RenderManagerUI::RenderPopups()
{
	PopupDescription();
}

bool RenderManagerUI::Init()
{
	ImGui_ImplDX11_Init(
		m_RenderManager->GetDevice(),
		m_RenderManager->GetContext()
	);

	return true;
}

void RenderManagerUI::DisplayDescription()
{
	if (ImGui::MenuItem("Graphics Settings"))
	{
		m_RequestOpenDescription = true;
	}
}

void RenderManagerUI::PopupDescription()
{
	if (m_RequestOpenDescription)
	{
		ImGui::OpenPopup("RenderSettingsPopup");
		m_RequestOpenDescription = false;
	}

	if (ImGui::BeginPopupModal("RenderSettingsPopup",
		nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		DrawRenderDetails();
		ImGui::Separator();
		DrawMSAAOptions();
		ImGui::Spacing();
		ImGui::Separator();

		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void RenderManagerUI::DrawRenderDetails() const
{
	DXGI_ADAPTER_DESC desc = m_RenderManager->GetAdapterInformation();
	int refreshRate = m_RenderManager->GetRefreshRate();
	int selectedMSAA = m_RenderManager->GetSelectedMSAA();

	char adapterName[128];
	WideCharToMultiByte(
		CP_UTF8,
		0,
		desc.Description,
		-1,
		adapterName,
		sizeof(adapterName),
		nullptr,
		nullptr);
	adapterName[sizeof(adapterName) - 1] = '\0';

	ImGui::Text("Adapter Name: %s", adapterName);
	ImGui::Text("Refresh Rate: %d Hz", refreshRate);
	ImGui::Text("Selected MSAA: %dx", selectedMSAA);
}

void RenderManagerUI::DrawMSAAOptions() const
{
	int selectedMSAA = m_RenderManager->GetSelectedMSAA();
	const std::vector<UINT>& allMSAAs = m_RenderManager->GetAllAvailableMSAA();

	ImGui::Text("Select MSAA Option:");
	static int selectedIndex = -1;

	for (size_t i = 0; i < allMSAAs.size(); ++i)
	{
		std::string label = std::to_string(allMSAAs[i]) + "x MSAA";
		if (ImGui::RadioButton(label.c_str(), selectedMSAA == static_cast<int>(allMSAAs[i])))
		{
			selectedIndex = static_cast<int>(i);
			if (allMSAAs[selectedIndex] != selectedMSAA)
			{
				m_RenderManager->ChangeMSAA(allMSAAs[selectedIndex]);
			}
		}
	}
}
