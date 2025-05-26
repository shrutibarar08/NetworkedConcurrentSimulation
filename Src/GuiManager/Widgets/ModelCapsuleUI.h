#pragma once


#pragma once
#include "GuiManager/Widgets/IWidget.h"
#include "RenderManager/Model/Shapes/ModelCapsule.h"


class ModelCapsuleUI final : public IWidget
{
public:
    ModelCapsuleUI(ModelCapsule* capsule);
    std::string MenuName() const override;
    void RenderOnScreen() override;

private:
    ModelCapsule* m_Capsule;
    RigidBody* m_RigidBody{ nullptr };
    CapsuleCollider* m_Collider{ nullptr };

    // Editable fields
    DirectX::XMFLOAT3 m_Pos{};
    DirectX::XMFLOAT3 m_Scale{};
    DirectX::XMFLOAT3 m_Vel{};
    DirectX::XMFLOAT3 m_Acc{};
    DirectX::XMFLOAT3 m_AngVel{};
    float m_Orientation[4]{};
    float m_Mass = 1.0f;
    float m_Damping = 0.99f;
    float m_Elasticity = 0.5f;
    bool m_bStatic{ false };
    float m_Restitution = 0.0f;
    float m_Friction = 0.0f;

    bool m_InitializedFromRigidBody = false;
};
