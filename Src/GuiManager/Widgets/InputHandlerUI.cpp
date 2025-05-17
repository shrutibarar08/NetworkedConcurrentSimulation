#include "InputHandlerUI.h"
#include "imgui.h"
#include "EventSystem/EventQueue.h"
#include "Utils/Logger.h"

InputHandlerUI::InputHandlerUI(InputHandler* inputHandler)
    : m_InputHandler(inputHandler)
{
    SyncFromInputHandler();
}

bool InputHandlerUI::Init()
{
    return m_InputHandler != nullptr;
}

void InputHandlerUI::RenderAsSystemItem()
{
    if (ImGui::MenuItem("Inputs Settings"))
    {
        m_PopupInputSetting = true;
    }
}

void InputHandlerUI::RenderMenu()
{
    if (!m_InputHandler) return;

    if (ImGui::MenuItem("Move Freely"))
    {
        if (m_InputHandler->IsMouseOnScreen())
        {
            m_InputHandler->SetMouseOnScreen(false);
        }
        else
        {
            m_InputHandler->SetMouseOnScreen(true);
        }
    }
}

void InputHandlerUI::RenderPopups()
{
    if (m_PopupInputSetting)
    {
        ImGui::OpenPopup("InputSettings");
        m_PopupInputSetting = false;
    }
    if (ImGui::BeginPopupModal("InputSettings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Remap Movement Keys");
        ImGui::Separator();

        if (ImGui::InputText("Move Forward", m_ForwardKey, IM_ARRAYSIZE(m_ForwardKey), ImGuiInputTextFlags_CharsUppercase))
            SyncToInputHandler();

        if (ImGui::InputText("Move Backward", m_BackwardKey, IM_ARRAYSIZE(m_BackwardKey), ImGuiInputTextFlags_CharsUppercase))
            SyncToInputHandler();

        if (ImGui::InputText("Move Left", m_LeftKey, IM_ARRAYSIZE(m_LeftKey), ImGuiInputTextFlags_CharsUppercase))
            SyncToInputHandler();

        if (ImGui::InputText("Move Right", m_RightKey, IM_ARRAYSIZE(m_RightKey), ImGuiInputTextFlags_CharsUppercase))
            SyncToInputHandler();

        ImGui::Separator();
        ImGui::Text("Mouse Sensitivity");

        if (ImGui::SliderFloat("Sensitivity X", &m_SensitivityX, 0.01f, 2.0f))
            m_InputHandler->SetMouseSensitivityX(m_SensitivityX);

        if (ImGui::SliderFloat("Sensitivity Y", &m_SensitivityY, 0.01f, 2.0f))
            m_InputHandler->SetMouseSensitivityY(m_SensitivityY);

        ImGui::Separator();

        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

void InputHandlerUI::SyncFromInputHandler()
{
    if (!m_InputHandler) return;

    m_ForwardKey[0] = static_cast<char>(m_InputHandler->GetMoveForwardKey());
    m_BackwardKey[0] = static_cast<char>(m_InputHandler->GetMoveBackwardKey());
    m_LeftKey[0] = static_cast<char>(m_InputHandler->GetMoveLeftKey());
    m_RightKey[0] = static_cast<char>(m_InputHandler->GetMoveRightKey());

    m_ForwardKey[1] = m_BackwardKey[1] = m_LeftKey[1] = m_RightKey[1] = '\0';

    m_SensitivityX = m_InputHandler->GetMouseSensitivityX();
    m_SensitivityY = m_InputHandler->GetMouseSensitivityY();
}

void InputHandlerUI::SyncToInputHandler() const
{
    if (!m_InputHandler) return;

    if (m_ForwardKey[0] != '\0') m_InputHandler->SetMoveForwardKey(static_cast<KeyCode>(m_ForwardKey[0]));
    if (m_BackwardKey[0] != '\0') m_InputHandler->SetMoveBackwardKey(static_cast<KeyCode>(m_BackwardKey[0]));
    if (m_LeftKey[0] != '\0') m_InputHandler->SetMoveLeftKey(static_cast<KeyCode>(m_LeftKey[0]));
    if (m_RightKey[0] != '\0') m_InputHandler->SetMoveRightKey(static_cast<KeyCode>(m_RightKey[0]));
}
