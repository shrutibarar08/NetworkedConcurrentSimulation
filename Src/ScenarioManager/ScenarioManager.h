#pragma once
#include "GuiManager/GuiManager.h"
#include "Scene/Scene.h"
#include "SystemManager/Interface/ISystem.h"

class ScenarioManager final: public ISystem
{
public:
	ScenarioManager();
	~ScenarioManager() override = default;
	bool Shutdown() override;
	bool Run() override;
	bool Build(SweetLoader& sweetLoader) override;

	unsigned int CreateScene(const std::string& name);
	void RemoveScene(const std::string& name);
	void RemoveScene(unsigned int id);

	void AttachUiRep(GuiManager* guiManager);
	const std::vector<Scene*>& GetScenes();

	Scene* GetActiveScene() const;
	void SetActiveScene(Scene* scene);

	void OnLoad(Scene* scene);
	void OffLoad(Scene* scene);

private:
	GuiManager* m_GuiManager{ nullptr };
	std::unordered_map<ID, std::unique_ptr<Scene>> m_Scenes;
	std::vector<Scene*> m_ScenesPtr;
	Scene* m_ActiveScene{ nullptr };

	SRWLOCK m_Lock;
};

