#include "KeyboardHandler.h"
#include <format>
#include "Utils/Logger.h"

bool KeyboardHandler::m_Keys[256] = {};

void KeyboardHandler::KeyDown(unsigned int wParam, LPARAM lParam)
{
    AcquireSRWLockExclusive(&m_Lock);
    m_Keys[wParam] = true;
    m_LastLParam = lParam;
	ReleaseSRWLockExclusive(&m_Lock);

    if (m_DebugInfo)
    {
        LOG_INFO(std::format("KeyDown: key = {}, lParam = 0x{:08X}", wParam, static_cast<unsigned int>(lParam)));
    }
}

void KeyboardHandler::KeyUp(unsigned int wParam, LPARAM lParam)
{
    AcquireSRWLockExclusive(&m_Lock);
    m_Keys[wParam] = false;
    if (m_LastLParam == lParam) m_LastLParam = 0;
    m_LPramActive = false;
    ReleaseSRWLockExclusive(&m_Lock);

    if (m_DebugInfo)
    {
        LOG_INFO(std::format("KeyUp: key = {}, lParam = 0x{:08X}", wParam, static_cast<unsigned int>(lParam)));
    }
}

bool KeyboardHandler::IsKeyDown(unsigned int keyCode)
{
    AcquireSRWLockShared(&m_Lock);
    bool state = m_Keys[keyCode];
    ReleaseSRWLockShared(&m_Lock);
    return state;
}

void KeyboardHandler::SetDebug(bool val)
{
    AcquireSRWLockExclusive(&m_Lock);
    m_DebugInfo = val;
    ReleaseSRWLockExclusive(&m_Lock);
}

LPARAM KeyboardHandler::GetLastLParam()
{
    AcquireSRWLockShared(&m_Lock);
    LPARAM result = m_LastLParam;
    m_LastLParam = 0;
    ReleaseSRWLockShared(&m_Lock);
    return result;
}

bool KeyboardHandler::IsAltPressedOnLastKey()
{
    bool altHeld = false;
    AcquireSRWLockShared(&m_Lock);
    if (!m_LPramActive)
    {
        altHeld = (m_LastLParam & (1 << 29)) != 0;
        m_LPramActive = true;
    }
    ReleaseSRWLockShared(&m_Lock);
    return altHeld;
}
