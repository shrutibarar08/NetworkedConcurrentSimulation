#include "WindowsSystem.h"
#include "ExceptionManager/WindowsException.h"
#include "Core/DefineDefault.h"

#include "Components/KeyboardHandler.h"
#include "Components/MouseHandler.h"

#include <windowsx.h>
#include "imgui.h"
#include "EventSystem/EventQueue.h"

WindowsSystem::WindowsSystem()
{
    InitializeSRWLock(&m_Lock);
    m_Fullscreen = Draco::Windows::FULL_SCREEN;
}

bool WindowsSystem::Build(SweetLoader& sweetLoader)
{
    if (!InitParameters(sweetLoader)) return false;
    //~ Initializing Window
    if (!InitWindowClass()) return false;

    LOG_SUCCESS("Built Windows System!");

    return true;
}

bool WindowsSystem::Shutdown()
{
    ShowCursor(true);
    if (Draco::Windows::FULL_SCREEN)
    {
        ChangeDisplaySettings(nullptr, 0);
    }
    DestroyWindow(m_HandleWindow);
    m_HandleWindow = nullptr;

    UnregisterClass(m_WindowName.c_str(), m_HandleInstance);
    m_HandleInstance = nullptr;

    return ISystem::Shutdown();
}

int WindowsSystem::ProcessMethod() const
{
    MSG msg{};
    bool quit = false;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT)
        {
            quit = true;
        }
    }

    return quit;
}

float WindowsSystem::GetAspectRatio() const
{
    return static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight);
}

bool WindowsSystem::IsFullScreen()
{
    bool status = false;

    AcquireSRWLockShared(&m_Lock);
    status = m_Fullscreen;
    ReleaseSRWLockShared(&m_Lock);

    return status;
}

void WindowsSystem::SetFullScreen(bool val)
{
    AcquireSRWLockExclusive(&m_Lock);
    if (val != m_Fullscreen)
    {
        m_Fullscreen = val;
        m_Fullscreen ? ApplyFullScreen() : ApplyWindowedScreen();
        UpdateWindow(GetWindowHandle());
    }
    ReleaseSRWLockExclusive(&m_Lock);
}

std::vector<RESOLUTION>& WindowsSystem::GetAvailableResolution()
{
    return m_PossibleResolution;
}

void WindowsSystem::UpdateResolution(const RESOLUTION* resolution)
{
    if (!resolution || !m_HandleWindow)
        return;
    if (resolution->Height == m_WindowHeight && resolution->Width == m_WindowWidth) return;

    RECT desiredRect = { 0, 0, static_cast<LONG>(resolution->Width), static_cast<LONG>(resolution->Height) };
    AdjustWindowRect(&desiredRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = desiredRect.right - desiredRect.left;
    int windowHeight = desiredRect.bottom - desiredRect.top;

    // Step 2: Resize the window while keeping its current position
    SetWindowPos(
        m_HandleWindow,
        nullptr,
        0, 0,
        windowWidth,
        windowHeight,
        SWP_NOZORDER | SWP_NOMOVE
    );

    m_WindowWidth = resolution->Width;
    m_WindowHeight = resolution->Height;

    ApplyFullScreen();

    EventQueue::Push(EventType::RENDER_EVENT_RESIZE);
}

HWND WindowsSystem::GetWindowHandle() const
{
    return m_HandleWindow;
}

HINSTANCE WindowsSystem::GetWindowManagerInstance() const
{
    return m_HandleInstance;
}

bool WindowsSystem::InitParameters(SweetLoader& sweetLoader)
{
    std::string widthKey = "Width";
    std::string heightKey = "Height";
    std::string windowNameKey = "WindowName";

    //~ Loading / Saving Width
    if (sweetLoader.Contains(widthKey))
    {
        m_WindowWidth = std::stoi(sweetLoader[widthKey].GetValue());
    }
    else
    {
        m_WindowWidth = Draco::Windows::DEFAULT_WIDTH;
        sweetLoader[widthKey] = std::to_string(m_WindowWidth);
    }

    //~ Loading / Saving Height
    if (sweetLoader.Contains(heightKey))
    {
        m_WindowHeight = std::stoi(sweetLoader[heightKey].GetValue());
    }
    else
    {
        m_WindowHeight = Draco::Windows::DEFAULT_HEIGHT;
        sweetLoader[heightKey] = std::to_string(m_WindowHeight);
    }

    //~ Loading / Saving Windows Name
    if (sweetLoader.Contains(windowNameKey))
    {
        std::string name = sweetLoader[windowNameKey].GetValue();
        m_WindowName = std::wstring(name.begin(), name.end());
    }
    else
    {
        m_WindowName = Draco::Windows::DEFAULT_WINDOW_NAME;
        sweetLoader[windowNameKey] = std::string(m_WindowName.begin(), m_WindowName.end());
    }
    return true;
}

bool WindowsSystem::InitWindowClass()
{
    int posX = 0, posY = 0;

    m_HandleInstance = GetModuleHandle(nullptr);

    // Define the window class
    WNDCLASS wc = {};
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = HandleMsgSetup;
    wc.hInstance = m_HandleInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = L"RunningWindowClass";

    // Register the window class
    BOOL result = RegisterClass(&wc);
    THROW_WINDOWS_EXCEPTION_IF_FAILED(result);

    // Set up style and dimensions
    DWORD style = Draco::Windows::FULL_SCREEN ? WS_POPUP : WS_OVERLAPPEDWINDOW;

    // Fullscreen uses native resolution
    if (Draco::Windows::FULL_SCREEN)
    {
        m_WindowWidth = GetSystemMetrics(SM_CXSCREEN);
        m_WindowHeight = GetSystemMetrics(SM_CYSCREEN);
        posX = 0;
        posY = 0;
        m_Fullscreen = true;
    }
    else
    {
        // Calculate adjusted window rectangle for desired client size
        RECT rect = { 0, 0, m_WindowWidth, m_WindowHeight };
        AdjustWindowRect(&rect, style, FALSE);
        m_WindowWidth = rect.right - rect.left;
        m_WindowHeight = rect.bottom - rect.top;

        posX = (GetSystemMetrics(SM_CXSCREEN) - m_WindowWidth) / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - m_WindowHeight) / 2;
    }

    // Create the actual window
    m_HandleWindow = CreateWindowEx(
        WS_EX_APPWINDOW,
        wc.lpszClassName,
        m_WindowName.c_str(),
        style,
        posX, posY,
        m_WindowWidth, m_WindowHeight,
        nullptr, nullptr,
        wc.hInstance,
        this
    );

    result = m_HandleWindow != nullptr;
    THROW_WINDOWS_EXCEPTION_IF_FAILED(result);

    //~ Configure for raw mouse input
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02; // Mouse
    rid.dwFlags = RIDEV_INPUTSINK; // or RIDEV_NOLEGACY to disable WM_MOUSEMOVE
    rid.hwndTarget = m_HandleWindow;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
    SetCursor(nullptr);

    // Show and activate the window
    ShowWindow(m_HandleWindow, SW_SHOW);
    SetForegroundWindow(m_HandleWindow);
    SetFocus(m_HandleWindow);
    UpdateWindow(m_HandleWindow);

    return true;
}

void WindowsSystem::ApplyFullScreen()
{
    if (m_Fullscreen)
    {
        GetWindowPlacement
    	(
            GetWindowHandle(),
            &m_WindowPlacement
        );

        SetWindowLong(GetWindowHandle(), GWL_STYLE, WS_POPUP);
        SetWindowPos(
            GetWindowHandle(),
            HWND_TOP,
            0, 0,
            GetSystemMetrics(SM_CXSCREEN),
            GetSystemMetrics(SM_CYSCREEN),
            SWP_FRAMECHANGED | SWP_SHOWWINDOW
        );
    }
}

void WindowsSystem::ApplyWindowedScreen() const
{
    if (!m_Fullscreen)
    {
        SetWindowLong(GetWindowHandle(), GWL_STYLE, WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(GetWindowHandle(), &m_WindowPlacement);
        SetWindowPos
        (
            GetWindowHandle(),
            nullptr,
            0, 0,
            0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW
        );
    }
}

LRESULT WindowsSystem::HandleMsgSetup(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    if (message == WM_NCCREATE)
    {
        const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        WindowsSystem* const pWnd = static_cast<WindowsSystem*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WindowsSystem::HandleMsgThunk));
        return pWnd->HandleMsg(hWnd, message, wParam, lParam);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT WindowsSystem::HandleMsgThunk(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    WindowsSystem* const pWnd = reinterpret_cast<WindowsSystem*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    return pWnd->HandleMsg(hWnd, message, wParam, lParam);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WindowsSystem::HandleMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    // mKeyboard input
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        KeyboardHandler::KeyDown(static_cast<unsigned int>(wParam), lParam);
        return 0;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        KeyboardHandler::KeyUp(static_cast<unsigned int>(wParam), lParam);
    	return 0;

    // mMouse buttons
    case WM_LBUTTONDOWN:
        MouseHandler::ButtonDown(MOUSE_BUTTON::LEFT_MOUSE);
        return 0;

    case WM_LBUTTONUP:
        MouseHandler::ButtonUp(MOUSE_BUTTON::LEFT_MOUSE);
        return 0;

    case WM_RBUTTONDOWN:
        MouseHandler::ButtonDown(MOUSE_BUTTON::RIGHT_MOUSE);
        return 0;

    case WM_RBUTTONUP:
        MouseHandler::ButtonUp(MOUSE_BUTTON::RIGHT_MOUSE);
        return 0;

    case WM_MBUTTONDOWN:
        MouseHandler::ButtonDown(MOUSE_BUTTON::MIDDLE_MOUSE);
        return 0;

    case WM_MBUTTONUP:
        MouseHandler::ButtonUp(MOUSE_BUTTON::MIDDLE_MOUSE);
        return 0;

    // mMouse movement
    case WM_MOUSEMOVE:
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        MouseHandler::SetPosition(x, y);
        return 0;
    }
    case WM_INPUT:
    {
        UINT dwSize = 0;
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
            nullptr, &dwSize, sizeof(RAWINPUTHEADER)) != 0)
            return 0;

        BYTE* lpb = new BYTE[dwSize];

        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
            lpb, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
        {
            RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb);
            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                int dx = raw->data.mouse.lLastX;
                int dy = raw->data.mouse.lLastY;

                MouseHandler::AddRawDelta(dx, dy);
            }
        }
        delete[] lpb;
        return 0;
    }
    case WM_SIZE:
    {
        EventQueue::Push(EventType::RENDER_EVENT_RESIZE);
        return 0;
    }
    // Window messages
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;

    case WM_DESTROY:
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
