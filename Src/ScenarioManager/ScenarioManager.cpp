#include "ScenarioManager.h"
#include <ranges>

#include "ApplicationManager/Clock/SystemClock.h"
#include "GuiManager/Widgets/ScenarioManagerUI.h"
#include "Utils/Logger.h"

ScenarioManager::ScenarioManager()
{
	InitializeSRWLock(&m_Lock);

	SetWidget(std::make_unique<ScenarioManagerUI>(this));
}

bool ScenarioManager::Shutdown()
{
	return ISystem::Shutdown();
}

bool ScenarioManager::Run()
{
	if (ISystem::Run())
	{
		float deltaTime = m_LocalTimer.Tick();
		if (m_ActiveScene)
		{
			m_ActiveScene->OnUpdate(deltaTime);
		}
		else m_LocalTimer.Reset();
	}
	return true;
}

bool ScenarioManager::Build(SweetLoader& sweetLoader)
{
	if (m_GuiManager) m_GuiManager->AddUI(GetWidget());
	return true;
}

unsigned int ScenarioManager::CreateScene(const std::string& name)
{
	auto scene = std::make_unique<Scene>(name);
	ID id = scene->GetId();

	m_Scenes[id] = std::move(scene);
	m_ScenesPtr.push_back(m_Scenes[id].get());
	return id;
}

void ScenarioManager::RemoveScene(const std::string& name)
{
	ID id = INT_MAX;
	for (auto& scene: m_Scenes | std::views::values)
	{
		if (scene->GetName() == name)
		{
			id = scene->GetId();
			break;
		}
	}

	if (id != INT_MAX)
	{
		m_Scenes.erase(id);
	}
}

void ScenarioManager::RemoveScene(unsigned int id)
{
	if (m_Scenes.contains(id))
	{
		m_Scenes.erase(id);
	}
}

void ScenarioManager::AttachUiRep(GuiManager* guiManager)
{
	m_GuiManager = guiManager;
}

const std::vector<Scene*>& ScenarioManager::GetScenes()
{
	if (m_ScenesPtr.empty())
	{
		for (auto& scene: m_Scenes | std::views::values)
		{
			m_ScenesPtr.emplace_back(scene.get());
		}
	}
	return m_ScenesPtr;
}

Scene* ScenarioManager::GetActiveScene() const
{
	return m_ActiveScene;
}

void ScenarioManager::SetActiveScene(Scene* scene)
{
	m_ActiveScene = scene;
}

void ScenarioManager::OnLoad(Scene* scene)
{
	if (m_ActiveScene)
	{
		LOG_WARNING("Offloading Active: " + scene->GetName());
		m_ActiveScene->OnOffLoad();
		m_GuiManager->RemoveUI(m_ActiveScene->GetWidget());
		m_ActiveScene = nullptr;
	}
	LOG_INFO("Loading: " + scene->GetName());
	m_ActiveScene = scene;
	m_GuiManager->AddUI(m_ActiveScene->GetWidget());
	LOG_INFO("GUI added!");
	m_ActiveScene->OnLoad();
	LOG_SUCCESS("Loaded!");
}

void ScenarioManager::OffLoad(Scene* scene)
{
	LOG_INFO("Offloading scene: " + scene->GetName());
	scene->OnOffLoad();
	m_GuiManager->RemoveUI(scene->GetWidget());
	if (scene->GetId() == m_ActiveScene->GetId())
	{
		m_ActiveScene = nullptr;
	}
}
