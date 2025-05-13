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
	if (ImGui::MenuItem("Display Description"))
	{
		m_RequestOpenDescription = true;
	}
}

void RenderManagerUI::PopupDescription()
{
	if (m_RequestOpenDescription)
	{
		ImGui::OpenPopup("RenderDescriptionPopup");
		m_RequestOpenDescription = false;
	}

	if (ImGui::BeginPopupModal("RenderDescriptionPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		DXGI_ADAPTER_DESC desc = m_RenderManager->GetAdapterInformation();
		int refreshRate = m_RenderManager->GetRefreshRate();
		int selectedMSAA = m_RenderManager->GetSelectedMSAA();
		std::vector<UINT> allMSAAs = m_RenderManager->GetAllAvailableMSAA();

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

		ImGui::Separator();
		ImGui::Text("All Available MSAA Options:");
		for (UINT sampleCount : allMSAAs)
		{
			ImGui::BulletText("%dx", sampleCount);
		}

		ImGui::Spacing();
		ImGui::Separator();

		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}
