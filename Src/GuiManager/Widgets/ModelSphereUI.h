#pragma once
#include "GuiManager/Widgets/IWidget.h"
#include "RenderManager/Model/Shapes/ModelSphere.h"


class ModelSphereUI final : public IWidget
{
public:
    ModelSphereUI(ModelSphere* sphere);
    std::string MenuName() const override;
    void RenderOnScreen() override;

private:
    ModelSphere* m_Sphere;
    RigidBody* m_RigidBody{ nullptr };
    SphereCollider* m_Collider{ nullptr };

    // Editable fields
    DirectX::XMFLOAT3 m_Pos{};
    DirectX::XMFLOAT3 m_Vel{};
    DirectX::XMFLOAT3 m_Acc{};
    DirectX::XMFLOAT3 m_AngVel{};
    float m_Orientation[4]{};
    float m_Mass = 1.0f;
    float m_Damping = 0.99f;
    float m_Elasticity = 0.5f;
    float m_AngularDamping = 0.99f;
    bool m_bStatic{ false };
    bool m_UiNeeded{ false };
    float m_Restitution = 0.0f;
    float m_Friction = 0.0f;
    char m_NameBuffer[128] = {};
};
