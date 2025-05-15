#include "GuiManager.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "Utils/Logger.h"

#include <ranges>

GuiManager::GuiManager()
{
	InitializeSRWLock(&m_Lock);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.FontGlobalScale = 1.25f;
	ImGui::StyleColorsDark();
}

bool GuiManager::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	return ISystem::Shutdown();
}

bool GuiManager::Run()
{
	ISystem::Run();

	BeginScene();
	RenderScene();
	EndScene();

	return true;
}

bool GuiManager::Build(SweetLoader& sweetLoader)
{
	bool status = true;
	AcquireSRWLockShared(&m_Lock);

	for (auto& widget: m_Widgets | std::views::values)
	{
		if (widget) widget->Init();
	}

	ReleaseSRWLockShared(&m_Lock);

	if (status) LOG_SUCCESS("Imgui Initialized!.");
	else LOG_FAIL("Imgui Failed to Initialize!.");

	return status;
}

void GuiManager::AddUI(IWidget* widget)
{
	if (widget) m_Widgets[widget->GetId()] = widget;
}

void GuiManager::RemoveUI(IWidget* widget)
{
	if (m_Widgets.contains(widget->GetId()))
	{
		m_Widgets.erase(widget->GetId());
	}
}

void GuiManager::RemoveUI(ID id)
{
	if (m_Widgets.contains(id))
	{
		m_Widgets.erase(id);
	}
}

void GuiManager::ResizeViewport(float width, float height)
{
	AcquireSRWLockShared(&m_Lock);
	if (width != m_PrevWidth && m_PrevHeight != height)
	{
		LOG_INFO("Changing IMGUI Viewport from: ("
			+ std::to_string(m_PrevWidth) + ", " + std::to_string(m_PrevHeight)
			+ ") -> (" + std::to_string(width) + ", " + std::to_string(height) + ")");

		m_PrevHeight = height;
		m_PrevWidth = width;
		ReleaseSRWLockShared(&m_Lock);
	}
	else
	{
		ReleaseSRWLockShared(&m_Lock);
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	AcquireSRWLockExclusive(&m_Lock);
	io.DisplaySize = ImVec2(width, height);
	ReleaseSRWLockExclusive(&m_Lock);
}

void GuiManager::BeginScene()
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();
}

void GuiManager::RenderScene()
{
	AcquireSRWLockShared(&m_Lock);
	//~ Render Main Menu Bar
	if (ImGui::BeginMainMenuBar())
	{
		for (auto& widget: m_Widgets | std::views::values)
		{
			if (ImGui::BeginMenu("Settings"))
			{
				widget->RenderAsSystemItem();
				ImGui::EndMenu();
			}
			widget->RenderMenu();
		}
		ImGui::EndMainMenuBar();
	}
	//~ Render Popups
	for (auto& widget : m_Widgets | std::views::values)
	{
		widget->RenderPopups();
	}
	ReleaseSRWLockShared(&m_Lock);
}

void GuiManager::EndScene()
{
	ImGui::Render();
}
