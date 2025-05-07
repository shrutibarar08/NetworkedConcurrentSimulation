#include "MouseHandler.h"
#include <format>
#include "Utils/Logger.h"

bool MouseHandler::m_Buttons[3] = {};
int MouseHandler::m_PosX = 0;
int MouseHandler::m_PosY = 0;
int MouseHandler::m_DeltaX = 0;
int MouseHandler::m_DeltaY = 0;


void MouseHandler::ButtonDown(MOUSE_BUTTON button)
{
    int index = static_cast<int>(button);
    if (index < 0 || index >= 3) return;

    bool debug = false;

    AcquireSRWLockExclusive(&m_Lock);
    m_Buttons[index] = true;
    debug = m_DebugInfo;
    ReleaseSRWLockExclusive(&m_Lock);

    if (debug)
    {
        LOG_INFO(std::format("Mouse Button Down: {}", index));
    }
}

void MouseHandler::ButtonUp(MOUSE_BUTTON button)
{
    int index = static_cast<int>(button);
    if (index < 0 || index >= 3) return;

    bool debug = false;

    AcquireSRWLockExclusive(&m_Lock);
    m_Buttons[index] = false;
    debug = m_DebugInfo;
    ReleaseSRWLockExclusive(&m_Lock);

    if (debug)
    {
        LOG_INFO(std::format("Mouse Button Up: {}", index));
    }
}

bool MouseHandler::IsButtonDown(MOUSE_BUTTON button)
{
    int index = static_cast<int>(button);
    if (index < 0 || index >= 3) return false;

    AcquireSRWLockShared(&m_Lock);
    bool state = m_Buttons[index];
    ReleaseSRWLockShared(&m_Lock);
    return state;
}

void MouseHandler::SetDebug(bool val)
{
    AcquireSRWLockExclusive(&m_Lock);
    m_DebugInfo = val;
    ReleaseSRWLockExclusive(&m_Lock);
}

void MouseHandler::SetPosition(int x, int y)
{
    int dx = 0, dy = 0;
    bool debug = false;

    AcquireSRWLockExclusive(&m_Lock);
    dx = x - m_LastX;
    dy = y - m_LastY;
    m_LastX = x;
    m_LastY = y;
    m_PosX = x;
    m_PosY = y;
    m_DeltaX = dx;
    m_DeltaY = dy;
    debug = m_DebugInfo;
    ReleaseSRWLockExclusive(&m_Lock);

    if (debug)
    {
        LOG_INFO(std::format("Mouse moved to: x = {}, y = {}", x, y));
        LOG_INFO(std::format("Delta: dx = {}, dy = {}", dx, dy));
    }
}

void MouseHandler::GetPosition(int& x, int& y)
{
    AcquireSRWLockShared(&m_Lock);
    x = m_PosX;
    y = m_PosY;
    ReleaseSRWLockShared(&m_Lock);
}

void MouseHandler::GetDelta(int& dx, int& dy)
{
    AcquireSRWLockShared(&m_Lock);
    dx = m_DeltaX;
    dy = m_DeltaY;
    ReleaseSRWLockShared(&m_Lock);
}

void MouseHandler::ResetDelta()
{
    bool debug = false;

    AcquireSRWLockExclusive(&m_Lock);
    m_DeltaX = 0;
    m_DeltaY = 0;
    debug = m_DebugInfo;
    ReleaseSRWLockExclusive(&m_Lock);

    if (debug)
    {
        LOG_INFO("Mouse delta reset to 0.");
    }
}
