#include "GUI.h"
#include <commctrl.h>
#include <windowsx.h>
#include <cmath>
#include <string>

#define IDC_SEARCH 101
#define IDC_LISTBOX 102

LauncherGUI::LauncherGUI(AppLauncher* appLauncher) : launcher(appLauncher), hwnd(NULL), searchBox(NULL), listBox(NULL), oldEditProc(NULL), oldListProc(NULL), isVisible(false) {}

LRESULT CALLBACK LauncherGUI::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LauncherGUI* gui = NULL;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        gui = (LauncherGUI*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)gui);
    } else {
        gui = (LauncherGUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    if (gui) {
        switch (uMsg) {
            case WM_CREATE:
                gui->OnCreate(hwnd);
                return 0;
            
            case WM_HOTKEY:
                if (wParam == HOTKEY_ID) {
                    gui->ToggleVisibility();
                }
                return 0;
                
            case WM_COMMAND:
                if (LOWORD(wParam) == IDC_SEARCH && HIWORD(wParam) == EN_CHANGE) {
                    gui->OnSearch();
                }
                else if (LOWORD(wParam) == IDC_LISTBOX && HIWORD(wParam) == LBN_DBLCLK) {
                    gui->OnItemSelected();
                    gui->Hide();
                    SetFocus(gui->searchBox);
                }
                else if (LOWORD(wParam) == IDC_LISTBOX && HIWORD(wParam) == LBN_SELCHANGE) {
                    SetFocus(gui->searchBox);
                }
                return 0;
            
            case WM_SETFOCUS:
                SetFocus(gui->searchBox);
                return 0;
            
            case WM_CTLCOLOREDIT: {
                HDC hdcEdit = (HDC)wParam;
                SetTextColor(hdcEdit, RGB(255, 255, 255));
                SetBkColor(hdcEdit, RGB(0, 0, 0));
                return (LRESULT)GetStockObject(BLACK_BRUSH);
            }
            
            case WM_MOUSEWHEEL: {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                int count = (int)SendMessage(gui->listBox, LB_GETCOUNT, 0, 0);
                if (count > 0) {
                    int current = (int)SendMessage(gui->listBox, LB_GETCURSEL, 0, 0);
                    if (current == LB_ERR) current = 0;
                    
                    if (delta > 0) {
                        if (current > 0) {
                            SendMessage(gui->listBox, LB_SETCURSEL, current - 1, 0);
                        }
                    } else {
                        if (current < count - 1) {
                            SendMessage(gui->listBox, LB_SETCURSEL, current + 1, 0);
                        }
                    }
                }
                return 0;
            }
                
            case WM_KEYDOWN:
                if (wParam == VK_DOWN) {
                    int count = (int)SendMessage(gui->listBox, LB_GETCOUNT, 0, 0);
                    if (count > 0) {
                        int current = (int)SendMessage(gui->listBox, LB_GETCURSEL, 0, 0);
                        if (current == LB_ERR) current = -1;
                        if (current < count - 1) {
                            SendMessage(gui->listBox, LB_SETCURSEL, current + 1, 0);
                        }
                    }
                    SetFocus(gui->searchBox);
                    return 0;
                }
                else if (wParam == VK_UP) {
                    int current = (int)SendMessage(gui->listBox, LB_GETCURSEL, 0, 0);
                    if (current > 0) {
                        SendMessage(gui->listBox, LB_SETCURSEL, current - 1, 0);
                    }
                    SetFocus(gui->searchBox);
                    return 0;
                }
                else if (wParam == VK_RETURN) {
                    int current = (int)SendMessage(gui->listBox, LB_GETCURSEL, 0, 0);
                    if (current != LB_ERR) {
                        gui->OnItemSelected();
                        gui->Hide();
                    }
                    SetFocus(gui->searchBox);
                    return 0;
                }
                else if (wParam == VK_ESCAPE) {
                    gui->Hide();
                    return 0;
                }
                break;
            
            case WM_ACTIVATE:
                if (LOWORD(wParam) == WA_INACTIVE) {
                    gui->Hide();
                }
                return 0;
            
            case WM_CTLCOLORLISTBOX: {
                HDC hdcList = (HDC)wParam;
                SetTextColor(hdcList, RGB(255, 255, 255));
                SetBkColor(hdcList, RGB(0, 0, 0));
                return (LRESULT)CreateSolidBrush(RGB(0, 0, 0));
            }
            
            case WM_DRAWITEM: {
                DRAWITEMSTRUCT* pDIS = (DRAWITEMSTRUCT*)lParam;
                if (pDIS->CtlID == IDC_LISTBOX) {
                    if (pDIS->itemID == (UINT)-1) break;
                    
                    COLORREF bgColor = (pDIS->itemState & ODS_SELECTED) ? RGB(60, 60, 60) : RGB(0, 0, 0);
                    COLORREF textColor = RGB(255, 255, 255);
                    
                    SetBkColor(pDIS->hDC, bgColor);
                    SetTextColor(pDIS->hDC, textColor);
                    
                    HBRUSH hBrush = CreateSolidBrush(bgColor);
                    FillRect(pDIS->hDC, &pDIS->rcItem, hBrush);
                    DeleteObject(hBrush);
                    
                    WCHAR text[256];
                    SendMessage(pDIS->hwndItem, LB_GETTEXT, pDIS->itemID, (LPARAM)text);
                    
                    // Add padding to the text area
                    RECT textRect = pDIS->rcItem;
                    textRect.left += 10;  // Left padding
                    textRect.right -= 10; // Right padding
                    
                    DrawText(pDIS->hDC, text, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                    
                    return TRUE;
                }
                break;
            }
            
            case WM_DESTROY:
                UnregisterHotKey(hwnd, HOTKEY_ID);
                PostQuitMessage(0);
                return 0;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK LauncherGUI::EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_GETDLGCODE) {
        return DLGC_WANTALLKEYS;
    }
    
    if (uMsg == WM_CHAR) {
        if (wParam == VK_RETURN || wParam == VK_ESCAPE) {
            return 0;
        }
    }
    
    if (uMsg == WM_KEYDOWN) {
        LauncherGUI* gui = (LauncherGUI*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
        if (gui) {
            if (wParam == VK_DOWN) {
                int count = (int)SendMessage(gui->listBox, LB_GETCOUNT, 0, 0);
                if (count > 0) {
                    int current = (int)SendMessage(gui->listBox, LB_GETCURSEL, 0, 0);
                    if (current == LB_ERR) current = -1;
                    if (current < count - 1) {
                        SendMessage(gui->listBox, LB_SETCURSEL, current + 1, 0);
                    }
                }
                return 0;
            }
            else if (wParam == VK_UP) {
                int current = (int)SendMessage(gui->listBox, LB_GETCURSEL, 0, 0);
                if (current > 0) {
                    SendMessage(gui->listBox, LB_SETCURSEL, current - 1, 0);
                }
                return 0;
            }
            else if (wParam == VK_RETURN) {
                int current = (int)SendMessage(gui->listBox, LB_GETCURSEL, 0, 0);
                if (current != LB_ERR) {
                    gui->OnItemSelected();
                    gui->Hide();
                }
                return 0;
            }
            else if (wParam == VK_ESCAPE) {
                gui->Hide();
                return 0;
            }
        }
    }
    
    WNDPROC oldProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    return CallWindowProc(oldProc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK LauncherGUI::ListSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WNDPROC oldProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    return CallWindowProc(oldProc, hwnd, uMsg, wParam, lParam);
}

double LauncherGUI::EvaluateExpression(const std::wstring& expr) {
    std::wstring expression = expr;
    // Remove spaces
    expression.erase(std::remove(expression.begin(), expression.end(), L' '), expression.end());
    
    double num1 = 0, num2 = 0;
    wchar_t op = 0;
    size_t opPos = std::wstring::npos;
    
    // Find operator (search from position 1 to allow negative numbers)
    for (size_t i = 1; i < expression.length(); i++) {
        if (expression[i] == L'+' || expression[i] == L'-' || 
            expression[i] == L'*' || expression[i] == L'/') {
            op = expression[i];
            opPos = i;
            break;
        }
    }
    
    if (opPos == std::wstring::npos) {
        return NAN;
    }
    
    try {
        num1 = std::stod(expression.substr(0, opPos));
        num2 = std::stod(expression.substr(opPos + 1));
    } catch (...) {
        return NAN;
    }
    
    switch (op) {
        case L'+': return num1 + num2;
        case L'-': return num1 - num2;
        case L'*': return num1 * num2;
        case L'/': return (num2 != 0) ? (num1 / num2) : NAN;
        default: return NAN;
    }
}

void LauncherGUI::OnCreate(HWND hwnd) {
    this->hwnd = hwnd;
    
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int windowWidth = clientRect.right - clientRect.left;
    int controlWidth = 380;
    int leftMargin = (windowWidth - controlWidth) / 2;
    
    searchBox = CreateWindowEx(
        0,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
        leftMargin, 15, controlWidth, 30,
        hwnd,
        (HMENU)IDC_SEARCH,
        GetModuleHandle(NULL),
        NULL
    );
    
    HFONT hFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    
    SendMessage(searchBox, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(searchBox, 0x1501, FALSE, (LPARAM)L"Search apps...");
    
    listBox = CreateWindowEx(
        0,
        L"LISTBOX",
        L"",
        WS_CHILD | LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
        leftMargin, 52, controlWidth, 0,
        hwnd,
        (HMENU)IDC_LISTBOX,
        GetModuleHandle(NULL),
        NULL
    );
    
    SendMessage(listBox, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(listBox, LB_SETITEMHEIGHT, 0, 35);  // Set item height with gap
    
    oldEditProc = (WNDPROC)SetWindowLongPtr(searchBox, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
    SetWindowLongPtr(searchBox, GWLP_USERDATA, (LONG_PTR)oldEditProc);
    
    oldListProc = (WNDPROC)SetWindowLongPtr(listBox, GWLP_WNDPROC, (LONG_PTR)ListSubclassProc);
    SetWindowLongPtr(listBox, GWLP_USERDATA, (LONG_PTR)oldListProc);
    
    SetFocus(searchBox);
}

void LauncherGUI::OnSearch() {
    WCHAR buffer[256] = {0};
    int len = GetWindowText(searchBox, buffer, 256);
    
    if (len == 0) {
        ShowWindow(listBox, SW_HIDE);
        RECT rect;
        GetWindowRect(hwnd, &rect);
        int windowWidth = rect.right - rect.left;
        
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        int frameHeight = (rect.bottom - rect.top) - clientRect.bottom;
        
        SetWindowPos(hwnd, NULL, 0, 0, windowWidth, 50 + frameHeight, SWP_NOMOVE | SWP_NOZORDER);
    } else {
        // Check if input is a math expression
        std::wstring input(buffer);
        bool isMathExpression = false;
        for (wchar_t c : input) {
            if (c == '+' || c == '-' || c == '*' || c == '/') {
                isMathExpression = true;
                break;
            }
        }

        if (isMathExpression) {
            double result = EvaluateExpression(input);
            if (!std::isnan(result)) {
                // Show calculation result
                SendMessage(listBox, LB_RESETCONTENT, 0, 0);
                
                WCHAR resultStr[512];
                // Format result - use integer format if result is whole number

                if (result == (long long)result) {
                    swprintf(resultStr, 512, L"%lld",(long long)result);
                } else {
                    swprintf(resultStr, 512, L"%g",result);
                }

                std::wstring resultstring = input + L" = " + resultStr;

                // SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)resultStr);
                SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)resultstring.c_str());
                
                ShowWindow(listBox, SW_SHOW);
                SendMessage(listBox, LB_SETCURSEL, 0, 0);
                
                // Resize window for one item
                int itemHeight = (int)SendMessage(listBox, LB_GETITEMHEIGHT, 0, 0);
                int listHeight = itemHeight + 6;
                
                RECT rect;
                GetWindowRect(hwnd, &rect);
                int windowWidth = rect.right - rect.left;
                
                RECT clientRect;
                GetClientRect(hwnd, &clientRect);
                int frameHeight = (rect.bottom - rect.top) - clientRect.bottom;
                int controlWidth = 380;
                int leftMargin = (clientRect.right - controlWidth) / 2;
                
                int totalHeight = 45 + listHeight + 10 + frameHeight;
                SetWindowPos(hwnd, NULL, 0, 0, windowWidth, totalHeight, SWP_NOMOVE | SWP_NOZORDER);
                SetWindowPos(listBox, NULL, leftMargin, 52, controlWidth, listHeight, SWP_NOZORDER);
                
                // Draw separator line
                HDC hdc = GetDC(hwnd);
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(60, 60, 60));
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                MoveToEx(hdc, leftMargin, 50, NULL);
                LineTo(hdc, leftMargin + controlWidth, 50);
                SelectObject(hdc, hOldPen);
                DeleteObject(hPen);
                ReleaseDC(hwnd, hdc);
                return;
            }
        }
        
        // Normal app search
        launcher->FilterApps(buffer);
        UpdateList();
    }
}

void LauncherGUI::OnItemSelected() {
    int index = (int)SendMessage(listBox, LB_GETCURSEL, 0, 0);
    if (index != LB_ERR) {
        launcher->LaunchApp(index);
    }
}

void LauncherGUI::UpdateList() {
    SendMessage(listBox, LB_RESETCONTENT, 0, 0);
    
    const auto& apps = launcher->GetFilteredApps();
    for (const auto& app : apps) {
        SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)app.name.c_str());
    }
    
    if (apps.size() > 0) {
        ShowWindow(listBox, SW_SHOW);
        SendMessage(listBox, LB_SETCURSEL, 0, 0);
        
        int itemHeight = (int)SendMessage(listBox, LB_GETITEMHEIGHT, 0, 0);
        int maxItems = (apps.size() < 8) ? (int)apps.size() : 8;
        int listHeight = maxItems * itemHeight + 6;
        
        RECT rect;
        GetWindowRect(hwnd, &rect);
        int windowWidth = rect.right - rect.left;
        
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        int frameHeight = (rect.bottom - rect.top) - clientRect.bottom;
        
        int totalHeight = 45 + listHeight + 10 + frameHeight;
        
        SetWindowPos(hwnd, NULL, 0, 0, windowWidth, totalHeight, SWP_NOMOVE | SWP_NOZORDER);
        
        GetClientRect(hwnd, &clientRect);
        int controlWidth = 380;
        int leftMargin = (clientRect.right - controlWidth) / 2;
        SetWindowPos(listBox, NULL, leftMargin, 52, controlWidth, listHeight, SWP_NOZORDER);
        
        // Draw 2px separator line between textbox and listbox
        HDC hdc = GetDC(hwnd);
        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(60, 60, 60));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, leftMargin, 50, NULL);
        LineTo(hdc, leftMargin + controlWidth, 50);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
        ReleaseDC(hwnd, hdc);
    } else {
        ShowWindow(listBox, SW_HIDE);
        RECT rect;
        GetWindowRect(hwnd, &rect);
        int windowWidth = rect.right - rect.left;
        SetWindowPos(hwnd, NULL, 0, 0, windowWidth, 94, SWP_NOMOVE | SWP_NOZORDER);
    }
}

bool LauncherGUI::Create(HINSTANCE hInstance) {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MassierLauncherClass";
    wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    if (!RegisterClassEx(&wc)) {
        return false;
    }
    
    hwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED,
        L"MassierLauncherClass",
        L"Massier Launcher",
        WS_POPUP | WS_BORDER,
        CW_USEDEFAULT, CW_USEDEFAULT, 416, 50,
        NULL, NULL, hInstance, this
    );
    
    if (hwnd) {
        // Set transparency (255 = opaque, 0 = fully transparent)
        SetLayeredWindowAttributes(hwnd, 0, 220, LWA_ALPHA);
        
        RECT rect;
        GetWindowRect(hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        
        int x = (screenWidth - width) / 2;
        int y = (screenHeight - height) / 3;
        
        SetWindowPos(hwnd, HWND_TOPMOST, x, y, width, height, SWP_NOACTIVATE);
        
        RegisterHotKey(hwnd, HOTKEY_ID, MOD_ALT, VK_SPACE);
    }
    
    return hwnd != NULL;
}

void LauncherGUI::Show() {
    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    SetFocus(searchBox);
    isVisible = true;
}

void LauncherGUI::Hide() {
    ShowWindow(hwnd, SW_HIDE);
    SetWindowText(searchBox, L"");
    SendMessage(listBox, LB_RESETCONTENT, 0, 0);
    ShowWindow(listBox, SW_HIDE);
    isVisible = false;
}

void LauncherGUI::ToggleVisibility() {
    if (isVisible) {
        Hide();
    } else {
        Show();
    }
}

int LauncherGUI::MessageLoop() {
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
