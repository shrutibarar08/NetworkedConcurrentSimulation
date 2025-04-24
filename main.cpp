#include <windows.h>

#include "ApplicationManager/Application.h"

void EnableConsole()
{
    AllocConsole();

    // Redirect stdout
    FILE* fpOut;
    freopen_s(&fpOut, "CONOUT$", "w", stdout);
    freopen_s(&fpOut, "CONOUT$", "w", stderr);

    // Redirect stdin
    FILE* fpIn;
    freopen_s(&fpIn, "CONIN$", "r", stdin);

    // Optional: sync C++ streams with stdio
    std::ios::sync_with_stdio(true);
}

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    EnableConsole();
    Application app{};

    if (!app.Init())
    {
        return E_FAIL;
    }

    return app.Run();
}
