#include "WindowsSystem.h"
#include "ExceptionManager/WindowsException.h"
#include "Core/DefineDefault.h"

bool WindowsSystem::Build(SweetLoader& sweetLoader)
{
    if (!InitParameters(sweetLoader)) return false;
    //~ Initializing Window
    if (!InitWindowClass()) return false;

    return true;
}

int WindowsSystem::ProcessMethod()
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
    return mWindowWidth / mWindowHeight;
}

HWND WindowsSystem::GetWindowHandle() const
{
    return mHandleWindow;
}

HINSTANCE WindowsSystem::GetWindowInstance() const
{
    return mHandleInstance;
}

bool WindowsSystem::InitParameters(SweetLoader& sweetLoader)
{
    std::string widthKey = "Width";
    std::string heightKey = "Height";
    std::string windowNameKey = "WindowName";

    //~ Loading / Saving Width
    if (sweetLoader.Contains(widthKey))
    {
        mWindowWidth = std::stoi(sweetLoader[widthKey].GetValue());
    }
    else
    {
        mWindowWidth = Barar::Windows::DEFAULT_WIDTH;
        sweetLoader[widthKey] = std::to_string(mWindowWidth);
    }

    //~ Loading / Saving Height
    if (sweetLoader.Contains(heightKey))
    {
        mWindowHeight = std::stoi(sweetLoader[heightKey].GetValue());
    }
    else
    {
        mWindowHeight = Barar::Windows::DEFAULT_HEIGHT;
        sweetLoader[heightKey] = std::to_string(mWindowHeight);
    }

    //~ Loading / Saving Windows Name
    if (sweetLoader.Contains(windowNameKey))
    {
        std::string name = sweetLoader[windowNameKey].GetValue();
        mWindowName = std::wstring(name.begin(), name.end());
    }
    else
    {
        mWindowName = Barar::Windows::DEFAULT_WINDOW_NAME;
        sweetLoader[windowNameKey] = std::string(mWindowName.begin(), mWindowName.end());
    }
    return true;
}

bool WindowsSystem::InitWindowClass()
{
    mHandleInstance = GetModuleHandle(nullptr);
    WNDCLASS wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = HandleMsgSetup;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = mHandleInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = L"RunningWindowClass";

    DWORD style = WS_OVERLAPPEDWINDOW;
    RECT rect = { 0, 0,
        mWindowWidth, mWindowHeight };

    // Adjust window rectangle to fit the desired client size
    THROW_WINDOWS_EXCEPTION_IF_FAILED(AdjustWindowRect(&rect, style, FALSE));
    THROW_WINDOWS_EXCEPTION_IF_FAILED(RegisterClass(&wc));

    int winWidth = rect.right - rect.left;
    int winHeight = rect.bottom - rect.top;

    mHandleWindow = CreateWindowEx(
        0,
        wc.lpszClassName,
        mWindowName.c_str(),
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        winWidth, winHeight,
        nullptr, nullptr,
        wc.hInstance,
        this
    );

    THROW_WINDOWS_EXCEPTION_IF_FAILED(mHandleWindow);

    ShowWindow(mHandleWindow, SW_SHOW);
    UpdateWindow(mHandleWindow);

    return true;
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

LRESULT WindowsSystem::HandleMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (message)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;

    case WM_DESTROY:
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
