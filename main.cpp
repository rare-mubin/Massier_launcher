#include "GUI.h"
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    AppLauncher launcher;
    LauncherGUI gui(&launcher);
    
    if (!gui.Create(hInstance)) {
        MessageBox(NULL, L"Failed to create window!", L"Error", MB_ICONERROR);
        return 1;
    }
    
    return gui.MessageLoop();
}
