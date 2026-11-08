#ifdef _WIN32
#include "headers/hide_console.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

void hide_console_for_children()
{
    if (GetConsoleWindow() == nullptr && AllocConsole())
        ShowWindow(GetConsoleWindow(), SW_HIDE);
}
#endif
