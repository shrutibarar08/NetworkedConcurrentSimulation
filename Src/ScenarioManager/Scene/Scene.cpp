#include "Scene.h"
#include "RenderManager/Render/Render3DQueue.h"
#include "RenderManager/Model/Shapes/ModelCube.h"
#include <ranges>

#include "GuiManager/Widgets/SceneUI.h"
#include "RenderManager/Model/Shapes/ModelCapsule.h"
#include "RenderManager/Model/Shapes/ModelSphere.h"
#include "Utils/Logger.h"

#include "ICollider.h"

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
	m_DeltaTime += deltaTime;

	if (!m_ObjectsToCreate.empty())
	{
		const CREATE_PAYLOAD& next = m_ObjectsToCreate.top();

		if (next.SpawnTime <= m_DeltaTime)
		{
			int key = AddObject(next.SpawnObject);
			m_Models[key]->SetPayload(next);
			m_ObjectsToCreate.pop();
			m_DeltaTime = 0.0f;
		}
	}
}

int Scene::AddObject(SPAWN_OBJECT obj)
{
	int key = -1;
	switch (obj)
	{
	case SPAWN_OBJECT::CUBE:
	{
		MODEL_INIT_DESC desc{};
		desc.PixelShaderPath = "Shaders/CubeShader/CubePS.hlsl";
		desc.VertexShaderPath = "Shaders/CubeShader/CubeVS.hlsl";
		desc.ModelName = "Default Cube";
		auto model = std::make_unique<ModelCube>(&desc);
		key = model->GetModelId();
		m_SafePointer[key] = model.get();
		m_Models[key] = std::move(model);

		if (m_State == State::LOADED) Render3DQueue::AddModel(m_Models[key].get());
		return key;
	}

	case SPAWN_OBJECT::SPHERE:
	{
		MODEL_INIT_DESC sphereDesc{};
		sphereDesc.PixelShaderPath = "Shaders/CubeShader/CubePS.hlsl";
		sphereDesc.VertexShaderPath = "Shaders/CubeShader/CubeVS.hlsl";
		sphereDesc.ModelName = "Default Sphere";
		auto modelSphere = std::make_unique<ModelSphere>(&sphereDesc);
		key = modelSphere->GetModelId();
		m_SafePointer[key] = modelSphere.get();
		m_Models[key] = std::move(modelSphere);
		if (m_State == State::LOADED) Render3DQueue::AddModel(m_Models[key].get());
		return key;
	}
	case SPAWN_OBJECT::CAPSULE:
	{
		MODEL_INIT_DESC capsuleDesc{};
		capsuleDesc.PixelShaderPath = "Shaders/CubeShader/CubePS.hlsl";
		capsuleDesc.VertexShaderPath = "Shaders/CubeShader/CubeVS.hlsl";
		capsuleDesc.ModelName = "Default Capsule";
		auto modelCapsule = std::make_unique<ModelCapsule>(&capsuleDesc);
		key = modelCapsule->GetModelId();
		m_SafePointer[key] = modelCapsule.get();
		m_Models[key] = std::move(modelCapsule);
		LOG_WARNING("Created Capsule with ID: " + std::to_string(key));

		if (m_State == State::LOADED) Render3DQueue::AddModel(m_Models[key].get());
		return key;
	}
	}
	return key;
}

int Scene::AddObject(CREATE_PAYLOAD& payload)
{
	payload.SpawnTime = 0.01f;
	m_ObjectsToCreate.push(payload);
	return 0;
}

int Scene::AddObject(std::unique_ptr<IModel> model)
{
	int key = model->GetModelId();
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

void Scene::AutoSpawn(const CREATE_SCENE_PAYLOAD& settings)
{
	for (int i = 0; i < settings.quantity; ++i)
	{
		if (settings.spawnCube)
		{
			CREATE_PAYLOAD payload = GeneratePayloadFromSceneSettings(SPAWN_OBJECT::CUBE, settings);
			payload.SpawnTime = settings.deltaSpawnTime;
			m_ObjectsToCreate.push(payload);
		}

		if (settings.spawnSphere)
		{
			CREATE_PAYLOAD payload = GeneratePayloadFromSceneSettings(SPAWN_OBJECT::SPHERE, settings);
			payload.SpawnTime = settings.deltaSpawnTime;
			m_ObjectsToCreate.push(payload);
		}

		if (settings.spawnCapsule)
		{
			CREATE_PAYLOAD payload = GeneratePayloadFromSceneSettings(SPAWN_OBJECT::CAPSULE, settings);
			payload.SpawnTime = settings.deltaSpawnTime;
			m_ObjectsToCreate.push(payload);
		}
	}
	LOG_INFO("Added Objects to Spawn: " + std::to_string(m_ObjectsToCreate.size()));
}

CREATE_PAYLOAD Scene::GeneratePayloadFromSceneSettings(SPAWN_OBJECT type, const CREATE_SCENE_PAYLOAD& settings)
{
	CREATE_PAYLOAD payload;
	payload.SpawnObject = type;

	// Use Randomizer class to generate all values
	payload.Position = m_Randomizer.Vec3(settings.minPosition, settings.maxPosition);
	payload.Velocity = m_Randomizer.Vec3(settings.minVelocity, settings.maxVelocity);
	payload.Acceleration = m_Randomizer.Vec3(settings.minAcceleration, settings.maxAcceleration);
	payload.AngularVelocity = m_Randomizer.Vec3(settings.minAngularVelocity, settings.maxAngularVelocity);

	payload.Mass = m_Randomizer.Float(settings.minMass, settings.maxMass);
	payload.Elasticity = m_Randomizer.Float(settings.minElasticity, settings.maxElasticity);
	payload.Restitution = m_Randomizer.Float(settings.minRestitution, settings.maxRestitution);
	payload.Friction = m_Randomizer.Float(settings.minFriction, settings.maxFriction);
	payload.AngularDamping = m_Randomizer.Float(settings.minAngularDamping, settings.maxAngularDamping);
	payload.LinearDamping = m_Randomizer.Float(settings.minLinearDamping, settings.maxLinearDamping);

	payload.SpawnTime = 0.0f;

	return payload;
}

std::unordered_map<unsigned int, IModel*> Scene::GetModels() const
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

void Scene::LoadFromSweetData(SweetLoader& sweetData)
{
	for (auto& [index, entry] : sweetData)
	{
		std::string typeStr = entry["Type"].GetValue();
		SPAWN_OBJECT type = StringToSpawnObject(typeStr);
		int key = AddObject(type);
		m_Models[key]->LoadFromSweetData(entry[typeStr]);
	}
}

SweetLoader Scene::SaveSweetData()
{
	SweetLoader sl{};
	for (int i = 0; i < static_cast<int>(m_Models.size()); ++i)
	{
		auto& modelPtr = m_Models[i];
		if (!modelPtr) continue;  // Skip if the model is null

		//~ Only Save Static Objects
		if (modelPtr->GetCollider()->GetColliderState() != ColliderSate::Static) continue;;

		auto* collider = modelPtr->GetCollider();
		if (!collider) continue;  // Skip if collider is null

		std::string indexStr = std::to_string(i);
		const char* typeStr = collider->ToString();

		sl[indexStr]["Type"] = typeStr;
		sl[indexStr][typeStr] = modelPtr->GetSweetData();
	}
	return sl;
}

SPAWN_OBJECT Scene::StringToSpawnObject(const std::string& name)
{
	if (name == "Cube") return SPAWN_OBJECT::CUBE;
	if (name == "Capsule") return SPAWN_OBJECT::CAPSULE;
	if (name == "Sphere") return SPAWN_OBJECT::SPHERE;

	return SPAWN_OBJECT();
}

std::string Scene::SpawnObjectToString(SPAWN_OBJECT so)
{
	if (so == SPAWN_OBJECT::CUBE) return "Cube";
	if (so == SPAWN_OBJECT::CAPSULE) return "Capsule";
	if (so == SPAWN_OBJECT::SPHERE) return "Sphere";

	return "unknown";
}
