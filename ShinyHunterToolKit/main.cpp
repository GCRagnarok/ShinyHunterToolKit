#include <windows.h>

#ifdef APIENTRY
#undef APIENTRY
#endif

#include <glfw3.h>
#include "include/ImGuiApp.h"

#ifdef _WIN32
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    ImGuiApp app;
    app.Run();
    return 0;
}
#endif

int main()
{
    ImGuiApp app;
    app.Run();
    return 0;
}