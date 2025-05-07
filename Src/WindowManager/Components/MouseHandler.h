#pragma once
#include <windows.h>
#include "Core/DefineDefault.h"

class MouseHandler
{
public:
    ~MouseHandler() = default;

    MouseHandler(const MouseHandler&) = delete;
    MouseHandler(MouseHandler&&) = delete;
    MouseHandler& operator=(const MouseHandler&) = delete;
    MouseHandler& operator=(MouseHandler&&) = delete;

    static void ButtonDown(MOUSE_BUTTON button);
    static void ButtonUp(MOUSE_BUTTON button);
    static bool IsButtonDown(MOUSE_BUTTON button);

    static void SetDebug(bool val);

    static void SetPosition(int x, int y);
    static void GetPosition(int& x, int& y);
    static void GetDelta(int& dx, int& dy);
    static void ResetDelta();

private:
    inline static SRWLOCK m_Lock = SRWLOCK_INIT;

    static bool m_Buttons[3];

    static int m_PosX, m_PosY;
    static int m_DeltaX, m_DeltaY;
    inline static int m_LastX = 0, m_LastY = 0;

    inline  static bool m_DebugInfo = false;
};
