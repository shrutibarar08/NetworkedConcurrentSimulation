#pragma once

#include <memory>
#include <unordered_map>
#include <queue>
#include <random>

#include "GuiManager/Widgets/IWidget.h"
#include "RenderManager/Model/IModel.h"
#include "Utils/LocalTimer.h"
#include "Utils/Randomizer.h"

typedef struct CREATE_SCENE_PAYLOAD
{
    // Position Range
    DirectX::XMFLOAT3 minPosition;
    DirectX::XMFLOAT3 maxPosition;

    // Velocity Range
    DirectX::XMFLOAT3 minVelocity;
    DirectX::XMFLOAT3 maxVelocity;

    // Acceleration Range
    DirectX::XMFLOAT3 minAcceleration;
    DirectX::XMFLOAT3 maxAcceleration;

    // Angular Velocity Range
    DirectX::XMFLOAT3 minAngularVelocity;
    DirectX::XMFLOAT3 maxAngularVelocity;

    // Scalar Ranges
    float minMass;
    float maxMass;

    float minElasticity;
    float maxElasticity;

    float minRestitution;
    float maxRestitution;

    float minFriction;
    float maxFriction;

    float minAngularDamping;
    float maxAngularDamping;

    float minLinearDamping;
    float maxLinearDamping;

    // Quantity to spawn
    int quantity;

    // Object type flags
    bool spawnCube;
    bool spawnSphere;
    bool spawnCapsule;

    float deltaSpawnTime;

} CREATE_SCENE_PAYLOAD;

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

	int AddObject(SPAWN_OBJECT obj);
	int AddObject(std::unique_ptr<IModel> model);
	void RemoveObject(unsigned int objId);

    void AutoSpawn(const CREATE_SCENE_PAYLOAD& payload);
    CREATE_PAYLOAD GeneratePayloadFromSceneSettings(SPAWN_OBJECT cube, const CREATE_SCENE_PAYLOAD& settings);

	std::unordered_map<unsigned int, IModel*> GetModels() const;

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
    std::priority_queue<CREATE_PAYLOAD> m_ObjectsToCreate;
    float m_DeltaTime{ 0.0f };
    Randomizer m_Randomizer{};
};
