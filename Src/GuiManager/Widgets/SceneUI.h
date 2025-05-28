#pragma once
#include "IWidget.h"
#include "ScenarioManager/Scene/Scene.h"

class SceneUI: public IWidget
{
public:
	SceneUI(Scene* scene);
	~SceneUI() override = default;
	void RenderMenu() override;
	void RenderOnScreen() override;
	void RenderPopups() override;
	std::string MenuName() const override;

private:
    void DisplaySpawnerPop();
    void DisplayCreateObjectPopup();
	void DisplayObjects() const;

private:
	bool m_ShowObjects{ false };
    bool m_PopUpCreateObject{ false };
	bool m_PopUpSpawner{ false };
    SPAWN_OBJECT m_WhatToCreate{};
	Scene* m_Scene;

    CREATE_PAYLOAD m_Payload{};

	CREATE_SCENE_PAYLOAD m_ScenePayload
	{
    {-10.f, 0.f, -10.f}, {10.f, 5.f, 10.f},
    {-5.f, 0.f, -5.f},   {5.f, 10.f, 5.f},
    {0.f, 0.f, 0.f},     {0.f, -9.8f, 0.f},
    {-1.f, -1.f, -1.f},  {1.f, 1.f, 1.f},
    1.f, 10.f,  // mass
    0.f, 1.f,   // elasticity
    0.f, 1.f,   // restitution
    0.f, 1.f,   // friction
    0.f, 1.f,   // angular damping
    0.f, 1.f,   // linear damping
    10,         // quantity
    true, true, true, // spawn all types
    0.1f};
};
