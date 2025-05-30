#include "ModelSphereUI.h"

#include "imgui.h"

ModelSphereUI::ModelSphereUI(ModelSphere* sphere)
	: m_Sphere(sphere)
{
    m_UiNeeded = m_Sphere->IsUiControlNeeded();
}

std::string ModelSphereUI::MenuName() const
{
	if (!m_Sphere) return "UNKNOWN";
	return m_Sphere->GetName();
}

void ModelSphereUI::RenderOnScreen()
{
    if (ImGui::Checkbox("Enable Gravity Tick", &m_UiNeeded))
    {
        m_Sphere->SetUiControlNeeded(m_UiNeeded);
    }
    if (!m_UiNeeded)
    {
        return;
    }
    m_RigidBody = m_Sphere->GetRigidBody();
    m_Collider = dynamic_cast<SphereCollider*>(m_Sphere->GetCollider());
    if (!m_RigidBody || !m_Collider) return;

    // === Pull RigidBody State ===
    DirectX::XMStoreFloat3(&m_Pos, m_RigidBody->GetPosition());
    DirectX::XMStoreFloat3(&m_Vel, m_RigidBody->GetVelocity());
    DirectX::XMStoreFloat3(&m_Acc, m_RigidBody->GetAcceleration());
    DirectX::XMStoreFloat3(&m_AngVel, m_RigidBody->GetAngularVelocity());



    Quaternion q = m_RigidBody->GetOrientation();
    m_Orientation[0] = q.GetI();
    m_Orientation[1] = q.GetJ();
    m_Orientation[2] = q.GetK();
    m_Orientation[3] = q.GetR();

    m_Mass = m_RigidBody->GetMass();
    m_Damping = m_RigidBody->GetDamping();
    m_Elasticity = m_RigidBody->GetElasticity();
    m_Restitution = m_RigidBody->GetRestitution();
    m_Friction = m_RigidBody->GetFriction();

    ImGui::Text("Object Identity");
    ImGui::Separator();

    ImGui::InputText("Name", m_NameBuffer, sizeof(m_NameBuffer));

    if (ImGui::Button("Apply Name"))
    {
        m_Sphere->SetName(std::string(m_NameBuffer));
    }

    ImGui::Text("Rigidbody Properties");
    ImGui::Separator();

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
        Quaternion updated(m_Orientation[3], m_Orientation[0], m_Orientation[1], m_Orientation[2]);
        m_RigidBody->SetOrientation(updated);
    }

    if (ImGui::DragFloat("Mass", &m_Mass, 0.1f, 0.001f, 1000.0f))
        m_RigidBody->SetMass(m_Mass);

    if (ImGui::DragFloat("Damping", &m_Damping, 0.01f, 0.0f, 1.0f))
        m_RigidBody->SetDamping(m_Damping);

    if (ImGui::DragFloat("Angular Damping", &m_AngularDamping, 0.01f, 0.0f, 1.0f))
        m_RigidBody->SetAngularDamping(m_AngularDamping);


    if (ImGui::DragFloat("Elasticity", &m_Elasticity, 0.01f, 0.0f, 1.0f))
        m_RigidBody->SetElasticity(m_Elasticity);

    if (ImGui::DragFloat("Restitution", &m_Restitution, 0.01f, 0.0f, 1.0f))
        m_RigidBody->SetRestitution(m_Restitution);

    if (ImGui::DragFloat("Friction", &m_Friction, 0.01f, 0.0f, 5.0f))
        m_RigidBody->SetFriction(m_Friction);

    bool isResting = m_RigidBody->GetRestingState();
    ImGui::TextColored(isResting ? ImVec4(0, 1, 0, 1) :
        ImVec4(1, 0, 0, 1),
        "Resting State: %s", isResting ? "Yes" : "No");

    // === Collider Section ===
    ImGui::Spacing();
    ImGui::Text("Collider Properties");
    ImGui::Separator();

    float radius = m_Sphere->GetRadius();
    if (ImGui::DragFloat("Sphere Radius", &radius, 0.01f, 0.01f, 100.0f))
        m_Sphere->SetRadius(radius);

    static const char* stateLabels[] = { "Dynamic", "Static", "Resting" };
    int stateIdx = static_cast<int>(m_Collider->GetColliderState());

    if (ImGui::Combo("Collider State", &stateIdx, stateLabels, IM_ARRAYSIZE(stateLabels)))
    {
        m_Collider->SetColliderState(static_cast<ColliderState>(stateIdx));
    }
}
