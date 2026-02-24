#pragma once
#include <windows.h>
#include <shlobj.h>
#include <vector>
#include <string>
#include <algorithm>
#include <shobjidl.h>

struct App {
    std::wstring name;
    std::wstring path;
    std::wstring appId;
    IShellItem* item;  // For AppsFolder items
};

class AppLauncher {
private:
    std::vector<App> allApps;
    std::vector<App> filteredApps;
    
    void ScanAppsFolder();

public:
    AppLauncher();
    void ScanStartMenu(const std::wstring& folder);
    void FilterApps(const std::wstring& search);
    const std::vector<App>& GetFilteredApps() const;
    bool LaunchApp(int index);
    int GetFilteredCount() const;
    void Reset();
};
