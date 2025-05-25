#include "Scene.h"
#include "RenderManager/Render/Render3DQueue.h"
#include "RenderManager/Model/Shapes/ModelCube.h"
#include <ranges>

#include "GuiManager/Widgets/SceneUI.h"
#include "Utils/Logger.h"

Scene::Scene(const std::string& name)
	: m_Name(name)
{
	m_Widget = std::make_unique<SceneUI>(this);
}

Scene::Scene()
{
	m_Widget = std::make_unique<SceneUI>(this);
}

void Scene::OnLoad()
{
	if (m_State == State::LOADED) return;
	m_State = State::LOADED;

	LOG_INFO("Loading Scene...");
	for (auto& model: m_Models | std::views::values)
	{
		Render3DQueue::AddModel(model.get());
	}
}

void Scene::OnOffLoad()
{
	//~ Safe because I think one 1 thread gonna load it anyways
	if (m_State == State::UNLOADED) return;
	m_State = State::UNLOADED;
	LOG_INFO("Off Loading Scene...");

	for (auto& model : m_Models | std::views::values)
	{
		Render3DQueue::RemoveModel(model.get());
	}
}

void Scene::OnUpdate(float deltaTime)
{
}

unsigned int Scene::AddObject(SPAWN_OBJECT obj)
{
	int key = 0;
	switch (obj)
	{
	case SPAWN_OBJECT::CUBE:
		MODEL_INIT_DESC desc{};
		desc.PixelShaderPath = "Shaders/CubeShader/CubePS.hlsl";
		desc.VertexShaderPath = "Shaders/CubeShader/CubeVS.hlsl";
		desc.ModelName = "Default Cube";
		auto model = std::make_unique<ModelCube>(&desc);
		key = model->GetModelId();
		model->GetRigidBody()->SetMass(1);
		m_SafePointer[key] = model.get();
		m_Models[key] = std::move(model);

		if (m_State == State::LOADED) Render3DQueue::AddModel(m_Models[key].get());
	}
	return key;
}

unsigned int Scene::AddObject(std::unique_ptr<IModel> model)
{
	unsigned int key = model->GetModelId();
	m_SafePointer[key] = model.get();
	m_Models[key] = std::move(model);
	if (m_State == State::LOADED) Render3DQueue::AddModel(m_Models[key].get());
	return key;
}

void Scene::RemoveObject(unsigned int objId)
{
	if (!m_Models.contains(objId))
	{
		return;
	}

	if (m_State == State::LOADED) Render3DQueue::RemoveModel(objId);
	m_Models.erase(objId);
	m_SafePointer.erase(objId);
}

std::unordered_map<unsigned int, IModel*> Scene::GetModels()
{
	auto copy = m_SafePointer;
	return copy;
}

IWidget* Scene::GetWidget() const
{
	return m_Widget.get();
}

std::string Scene::GetName() const
{
	return m_Name;
}

void Scene::SetName(const std::string& name)
{
	m_Name = name;
}

bool Scene::IsLoaded() const
{
	return m_State == State::LOADED;
}
