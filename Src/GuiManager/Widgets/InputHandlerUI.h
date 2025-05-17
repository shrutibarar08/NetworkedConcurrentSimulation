#pragma once
#include "IWidget.h"
#include "ApplicationManager/InputHandler/InputHandler.h"
#include <string>

class InputHandlerUI : public IWidget
{
public:
    InputHandlerUI(InputHandler* inputHandler);
    ~InputHandlerUI() override = default;

    std::string MenuName() const override { return "InputHandler"; }
    bool Init() override;
    void RenderAsSystemItem() override;
    void RenderMenu() override;
    void RenderPopups() override;

private:
    InputHandler* m_InputHandler;

    // For keybinding editing
    char m_ForwardKey[2];
    char m_BackwardKey[2];
    char m_LeftKey[2];
    char m_RightKey[2];

    float m_SensitivityX;
    float m_SensitivityY;

    void SyncFromInputHandler();
    void SyncToInputHandler() const;

private:
    bool m_PopupInputSetting{ false };
};
