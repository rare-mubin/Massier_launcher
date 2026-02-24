#ifndef GUI_H
#define GUI_H

#include <windows.h>
#include <string>
#include "AppLauncher.h"

#define WM_TRAYICON (WM_USER + 1)
#define WM_RESET_DONE (WM_USER + 2)
#define HOTKEY_ID 1

class LauncherGUI {
private:
    HWND hwnd;
    HWND searchBox;
    HWND listBox;
    AppLauncher* launcher;
    WNDPROC oldEditProc;
    WNDPROC oldListProc;
    bool isVisible;
    bool isResetting;
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ListSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnCreate(HWND hwnd);
    void OnSearch();
    void OnItemSelected();
    void UpdateList();
    void ToggleVisibility();
    void ResetAndRefresh();
    double EvaluateExpression(const std::wstring& expr);

public:
    LauncherGUI(AppLauncher* appLauncher);
    bool Create(HINSTANCE hInstance);
    void Show();
    void Hide();
    int MessageLoop();
};

#endif
