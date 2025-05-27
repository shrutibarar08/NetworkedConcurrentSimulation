#include "ModelSphereUI.h"

#include "imgui.h"

ModelSphereUI::ModelSphereUI(ModelSphere* sphere)
	: m_Sphere(sphere)
{

}

std::string ModelSphereUI::MenuName() const
{
	if (!m_Sphere) return "UNKNOWN";
	return m_Sphere->GetName();
}

void ModelSphereUI::RenderOnScreen()
{
    if (!m_bStatic) return;
    if (!m_RigidBody || !m_Collider) return;
    m_RigidBody = m_Sphere->GetRigidBody();
    m_Collider = dynamic_cast<SphereCollider*>(m_Sphere->GetCollider());
    m_bStatic = m_Collider->GetColliderState() == ColliderSate::Static;
    if (!m_bStatic) return;

    std::string headingEdit = "RigidBody (Edit) " + std::to_string(m_Sphere->GetModelId());
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

        if (m_Collider && ImGui::CollapsingHeader("Collider Properties"))
        {
            float radius = m_Sphere->GetRadius();
            if (ImGui::DragFloat("Sphere Radius", &radius, 0.01f, 0.01f, 100.0f))
                m_Sphere->SetRadius(radius);

            static const char* stateLabels[] = { "Dynamic", "Static", "Resting" };
            int currentStateIndex = static_cast<int>(m_Collider->GetColliderState());

            if (ImGui::Combo("Collider State", &currentStateIndex, stateLabels, IM_ARRAYSIZE(stateLabels)))
            {
                m_Collider->SetColliderState(static_cast<ColliderSate>(currentStateIndex));
            }
        }
    }
}
