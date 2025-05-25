#include "ModelCubeUI.h"

#include <iostream>

#include "imgui.h"
#include "RigidBody.h"
#include "Utils/Logger.h"

ModelCubeUI::ModelCubeUI(ModelCube* cube)
	: m_Cube(cube)
{
    Init();
}

std::string ModelCubeUI::MenuName() const
{
	if (!m_Cube) return "UNKNOWN";
	return m_Cube->GetName();
}

void ModelCubeUI::RenderOnScreen()
{
    m_RigidBody = m_Cube->GetRigidBody();
    m_Collider = dynamic_cast<CubeCollider*>(m_Cube->GetCollider());
    if (!m_RigidBody) return;

    std::string headingEdit = "RigidBody (Edit) " + std::to_string(m_Cube->GetModelId());
    if (ImGui::CollapsingHeader(headingEdit.c_str()))
    {
        DirectX::XMStoreFloat3(&m_Pos, m_RigidBody->GetPosition());
        DirectX::XMStoreFloat3(&m_Vel, m_RigidBody->GetVelocity());
        DirectX::XMStoreFloat3(&m_Acc, m_RigidBody->GetAcceleration());
        DirectX::XMStoreFloat3(&m_AngVel, m_RigidBody->GetAngularVelocity());

        Quaternion q = m_RigidBody->GetOrientation();
        m_Orientation[0] = q.I;
        m_Orientation[1] = q.J;
        m_Orientation[2] = q.K;
        m_Orientation[3] = q.R;

        m_Mass = m_RigidBody->GetMass();
        m_Damping = m_RigidBody->GetDamping();
        m_Elasticity = m_RigidBody->GetElasticity();
        m_Restitution = m_RigidBody->GetRestitution();
        m_Friction = m_RigidBody->GetFriction();

        // === Live Editable Fields ===
        if (ImGui::DragFloat3("Position", &m_Pos.x, 0.1f))
            m_RigidBody->SetPosition(DirectX::XMLoadFloat3(&m_Pos));

        if (ImGui::DragFloat3("Velocity", &m_Vel.x, 0.1f))
            m_RigidBody->SetVelocity(DirectX::XMLoadFloat3(&m_Vel));

        if (ImGui::DragFloat3("Acceleration", &m_Acc.x, 0.1f))
            m_RigidBody->SetAcceleration(DirectX::XMLoadFloat3(&m_Acc));

        if (ImGui::DragFloat3("Angular Velocity", &m_AngVel.x, 0.1f))
            m_RigidBody->SetAngularVelocity(DirectX::XMLoadFloat3(&m_AngVel));

        if (ImGui::DragFloat4("Orientation (I, J, K, R)", m_Orientation, 0.01f))
        {
            Quaternion q(m_Orientation[3], m_Orientation[0], m_Orientation[1], m_Orientation[2]);
            m_RigidBody->SetOrientation(q);
        }

        if (ImGui::DragFloat("Mass", &m_Mass, 0.1f, 0.001f, 1000.0f))
            m_RigidBody->SetMass(m_Mass);

        if (ImGui::DragFloat("Damping", &m_Damping, 0.01f, 0.0f, 1.0f))
            m_RigidBody->SetDamping(m_Damping);

        if (ImGui::DragFloat("Elasticity", &m_Elasticity, 0.01f, 0.0f, 1.0f))
            m_RigidBody->SetElasticity(m_Elasticity);

        if (ImGui::DragFloat("Restitution", &m_Restitution, 0.01f, 0.0f, 1.0f))
            m_RigidBody->SetRestitution(m_Restitution);

        if (ImGui::DragFloat("Friction", &m_Friction, 0.01f, 0.0f, 5.0f))
            m_RigidBody->SetFriction(m_Friction);

        // === Collider State Combo ===
        if (m_Collider)
        {
            static const char* stateLabels[] = { "Dynamic", "Static", "Resting" };
            int currentStateIndex = static_cast<int>(m_Collider->GetColliderState());

            if (ImGui::Combo("Collider State", &currentStateIndex, stateLabels, IM_ARRAYSIZE(stateLabels)))
            {
                m_Collider->SetColliderState(static_cast<ColliderSate>(currentStateIndex));
            }
        }
    }
}

bool ModelCubeUI::Init()
{
    m_RigidBody = m_Cube->GetRigidBody();
    return true;
}

void ModelCubeUI::InitCollider()
{
    if (CubeCollider* cube = dynamic_cast<CubeCollider*>(m_Cube->GetCollider()))
    {
        m_Collider = cube;
    }
    else LOG_WARNING("Warning Failed to Cast Cube Collider!");
}
