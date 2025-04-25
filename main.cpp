#include <windows.h>
#include "ApplicationManager/Application.h"
#include "Utils/Logger.h"

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
#ifdef _DEBUG
    Logger::Init();
#endif

	Application app{};

    if (!app.Init())
    {
        return E_FAIL;
    }

    bool result = app.Run();

#ifdef _DEBUG
    Logger::Close();
#endif

    return result;
}
