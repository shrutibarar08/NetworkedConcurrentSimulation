#pragma once
#include <Windows.h>
#include <unordered_map>

class KeyboardHandler
{
public:
    ~KeyboardHandler() = default;

    KeyboardHandler(const KeyboardHandler&) = delete;
    KeyboardHandler(KeyboardHandler&&) = delete;
    KeyboardHandler& operator=(const KeyboardHandler&) = delete;
    KeyboardHandler& operator=(KeyboardHandler&&) = delete;

    static void KeyDown(unsigned int wParam, LPARAM lParam);
    static void KeyUp(unsigned int wParam, LPARAM lParam);
    static bool IsKeyDown(unsigned int keyCode);
    static void SetDebug(bool val);

    static LPARAM GetLastLParam();
    static bool IsAltPressedOnLastKey();

private:
    inline static bool m_DebugInfo = false;
	static bool m_Keys[256];
	inline static LPARAM m_LastLParam = 0;
    inline static bool m_LPramActive{ false };
};
