#include "KeyboardHandler.h"
#include <format>
#include "Utils/Logger.h"

bool KeyboardHandler::m_Keys[256] = {};

void KeyboardHandler::KeyDown(unsigned int wParam, LPARAM lParam)
{
    m_Keys[wParam] = true;
    m_LastLParam = lParam;
    if (m_DebugInfo)
    {
        LOG_INFO(std::format("KeyDown: key = {}, lParam = 0x{:08X}", wParam, static_cast<unsigned int>(lParam)));
    }
}

void KeyboardHandler::KeyUp(unsigned int wParam, LPARAM lParam)
{
    m_Keys[wParam] = false;
    if (m_LastLParam == lParam) m_LastLParam = 0;
    m_LPramActive = false;

    if (m_DebugInfo)
    {
        LOG_INFO(std::format("KeyUp: key = {}, lParam = 0x{:08X}", wParam, static_cast<unsigned int>(lParam)));
    }
}

bool KeyboardHandler::IsKeyDown(unsigned int keyCode)
{
    bool state = m_Keys[keyCode];
    return state;
}

void KeyboardHandler::SetDebug(bool val)
{
    m_DebugInfo = val;
}

LPARAM KeyboardHandler::GetLastLParam()
{
    LPARAM result = m_LastLParam;
    m_LastLParam = 0;
    return result;
}

bool KeyboardHandler::IsAltPressedOnLastKey()
{
    bool altHeld = false;
    if (!m_LPramActive)
    {
        altHeld = (m_LastLParam & (1 << 29)) != 0;
        m_LPramActive = true;
    }
    return altHeld;
}
