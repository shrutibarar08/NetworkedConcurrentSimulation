#pragma once

#include <memory>
#include <unordered_map>

#include "GuiManager/Widgets/IWidget.h"
#include "RenderManager/Model/IModel.h"

enum class SPAWN_OBJECT: uint8_t
{
	CUBE
};

enum class State : uint8_t
{
	LOADED,
	UNLOADED
};

class Scene: public IEntity
{
public:
	Scene(const std::string& name);
	Scene();
	~Scene() = default;

	Scene(const Scene&) = delete;
	Scene(Scene&&) = delete;
	Scene& operator=(Scene&&) = delete;
	Scene& operator=(const Scene&) = delete;

	void OnLoad();
	void OnOffLoad();
	void OnUpdate(float deltaTime);

	unsigned int AddObject(SPAWN_OBJECT obj);
	unsigned int AddObject(std::unique_ptr<IModel> model);
	void RemoveObject(unsigned int objId);

	std::unordered_map<unsigned int, IModel*> GetModels();

	IWidget* GetWidget() const;
	std::string GetName() const;
	void SetName(const std::string& name);

	bool IsLoaded() const;

private:
	std::unordered_map<unsigned int, std::unique_ptr<IModel>> m_Models;
	std::unordered_map<unsigned int, IModel*> m_SafePointer;
	std::unique_ptr<IWidget> m_Widget;
	State m_State = State::UNLOADED;
	std::string m_Name{ "Default Scene" };
};
