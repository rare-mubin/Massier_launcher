#include "AppLauncher.h"
#include "IApplicationActivationManager.h"
#include <shlobj.h>
#include <propvarutil.h>
#include <propkey.h>
#include <shobjidl.h>

AppLauncher::AppLauncher() {
    CoInitialize(NULL);
    
    WCHAR path[MAX_PATH];

    if (SHGetFolderPathW(NULL, CSIDL_COMMON_PROGRAMS, NULL, 0, path) == S_OK)
        ScanStartMenu(path);

    if (SHGetFolderPathW(NULL, CSIDL_PROGRAMS, NULL, 0, path) == S_OK)
        ScanStartMenu(path);

    ScanAppsFolder();

    std::sort(allApps.begin(), allApps.end(), [](const App& a, const App& b) {
        return a.name < b.name;
    });

    auto newEnd = std::unique(allApps.begin(), allApps.end(), [](const App& a, const App& b) {
        return a.name == b.name;
    });
    // Destroy icons of duplicates before erasing
    for (auto it = newEnd; it != allApps.end(); ++it)
        if (it->icon) { DestroyIcon(it->icon); it->icon = nullptr; }
    allApps.erase(newEnd, allApps.end());

    filteredApps = allApps;
    
    CoUninitialize();
}

void AppLauncher::ScanStartMenu(const std::wstring& folder) {
    WIN32_FIND_DATAW data;
    HANDLE hFind = FindFirstFileW((folder + L"\\*").c_str(), &data);

    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (data.cFileName[0] == L'.') continue;

        std::wstring path = folder + L"\\" + data.cFileName;

        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            ScanStartMenu(path);
        } else {
            std::wstring name = data.cFileName;
            // case-insensitive check for .lnk extension
            std::wstring nameLower = name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::towlower);
            if (nameLower.size() > 4 && nameLower.substr(nameLower.size() - 4) == L".lnk") {
                IShellLinkW* psl = nullptr;
                if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&psl))) {
                    IPersistFile* ppf = nullptr;
                    if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf))) {
                        if (SUCCEEDED(ppf->Load(path.c_str(), STGM_READ))) {
                            
                            psl->Resolve(NULL, SLR_NO_UI);

                            WCHAR targetPath[MAX_PATH] = {0};
                            WIN32_FIND_DATAW findData = {0};
                            if (SUCCEEDED(psl->GetPath(targetPath, MAX_PATH, &findData, 0)) && wcslen(targetPath) > 0) {
                                std::wstring resolved = targetPath;

                                // Trim surrounding quotes and whitespace
                                auto trimQuotesAndSpaces = [](std::wstring &s) {
                                    // trim leading spaces
                                    while (!s.empty() && iswspace(s.front())) s.erase(s.begin());
                                    // trim trailing spaces
                                    while (!s.empty() && iswspace(s.back())) s.pop_back();
                                    // remove surrounding quotes
                                    if (s.size() >= 2 && s.front() == L'"' && s.back() == L'"')
                                        s = s.substr(1, s.size() - 2);
                                };

                                trimQuotesAndSpaces(resolved);

                                // If the path contains arguments (e.g. "C:\app.exe" -arg or C:\app.exe -arg), strip after the executable
                                auto stripArgs = [](const std::wstring &s) {
                                    if (s.empty()) return s;
                                    if (s.front() == L'"') {
                                        size_t endq = s.find(L'"', 1);
                                        if (endq != std::wstring::npos) return s.substr(1, endq - 1);
                                    }
                                    size_t sp = s.find(L' ');
                                    if (sp != std::wstring::npos) return s.substr(0, sp);
                                    return s;
                                };

                                std::wstring candidate = stripArgs(resolved);

                                DWORD attrs = GetFileAttributesW(candidate.c_str());
                                if (attrs == INVALID_FILE_ATTRIBUTES) {
                                    // try without quotes (already handled) or fallback to original resolved
                                    attrs = GetFileAttributesW(resolved.c_str());
                                } else {
                                    // candidate exists, use it
                                    resolved = candidate;
                                }

                                if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                                    std::wstring targetLower = resolved;
                                    std::transform(targetLower.begin(), targetLower.end(), targetLower.begin(), ::towlower);

                                    // check extension at the end of the path (safer than find)
                                    if (targetLower.size() >= 4) {
                                        std::wstring ext = targetLower.substr(targetLower.size() - 4);
                                        if (ext == L".exe" || ext == L".bat" || ext == L".cmd") {
                                            std::wstring displayName = name;
                                            if (displayName.size() > 4) displayName = displayName.substr(0, displayName.size() - 4);
                                            HICON hIcon = nullptr;
                                            SHFILEINFOW sfi = {0};
                                            if (SHGetFileInfoW(path.c_str(), 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON))
                                                hIcon = sfi.hIcon;
                                            allApps.push_back({displayName, path, L"", nullptr, hIcon});
                                        }
                                    }
                                }
                            }
                        }
                        ppf->Release();
                    }
                    psl->Release();
                }
            }
        }
    } while (FindNextFileW(hFind, &data));

    FindClose(hFind);
}

void AppLauncher::ScanAppsFolder() {
    IShellItem* pAppsFolder = nullptr;
    HRESULT hr = SHGetKnownFolderItem(FOLDERID_AppsFolder, KF_FLAG_DEFAULT, nullptr, IID_PPV_ARGS(&pAppsFolder));
    if (FAILED(hr)) return;

    IEnumShellItems* pEnum = nullptr;
    hr = pAppsFolder->BindToHandler(nullptr, BHID_EnumItems, IID_PPV_ARGS(&pEnum));
    if (FAILED(hr)) {
        pAppsFolder->Release();
        return;
    }

    IShellItem* pItem = nullptr;
    while (pEnum->Next(1, &pItem, nullptr) == S_OK) {
        PWSTR displayName = nullptr;
        if (SUCCEEDED(pItem->GetDisplayName(SIGDN_NORMALDISPLAY, &displayName))) {
            std::wstring appName = displayName;
            CoTaskMemFree(displayName);

            std::wstring appId;
            IPropertyStore* pStore = nullptr;
            if (SUCCEEDED(pItem->BindToHandler(nullptr, BHID_PropertyStore, IID_PPV_ARGS(&pStore)))) {
                PROPVARIANT prop;
                PropVariantInit(&prop);
                if (SUCCEEDED(pStore->GetValue(PKEY_AppUserModel_ID, &prop))) {
                    if (prop.vt == VT_LPWSTR && prop.pwszVal)
                        appId = prop.pwszVal;
                    PropVariantClear(&prop);
                }
                pStore->Release();
            }

            PWSTR pszPath = nullptr;
            std::wstring filePath;
            if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                filePath = pszPath;
                CoTaskMemFree(pszPath);
            }

            HICON hIcon = nullptr;

            // Use IShellItemImageFactory for UWP/Store apps — correctly renders package PNG assets
            IShellItemImageFactory* pImgFactory = nullptr;
            if (SUCCEEDED(pItem->QueryInterface(IID_PPV_ARGS(&pImgFactory)))) {
                SIZE sz = {20, 20};
                HBITMAP hBmp = nullptr;
                if (SUCCEEDED(pImgFactory->GetImage(sz, SIIGBF_RESIZETOFIT | SIIGBF_ICONONLY, &hBmp))) {
                    ICONINFO ii = {};
                    ii.fIcon  = TRUE;
                    ii.hbmColor = hBmp;
                    ii.hbmMask  = CreateBitmap(20, 20, 1, 1, nullptr);
                    hIcon = CreateIconIndirect(&ii);
                    DeleteObject(ii.hbmMask);
                    DeleteObject(hBmp);
                }
                // Fallback: try without SIIGBF_ICONONLY (some apps need SIIGBF_THUMBNAILONLY)
                if (!hIcon) {
                    if (SUCCEEDED(pImgFactory->GetImage(sz, SIIGBF_RESIZETOFIT, &hBmp))) {
                        ICONINFO ii = {};
                        ii.fIcon    = TRUE;
                        ii.hbmColor = hBmp;
                        ii.hbmMask  = CreateBitmap(20, 20, 1, 1, nullptr);
                        hIcon = CreateIconIndirect(&ii);
                        DeleteObject(ii.hbmMask);
                        DeleteObject(hBmp);
                    }
                }
                pImgFactory->Release();
            }

            // Final fallback: PIDL via SHGetFileInfo
            if (!hIcon) {
                SHFILEINFOW sfi2 = {0};
                PIDLIST_ABSOLUTE pidlIcon = nullptr;
                if (SUCCEEDED(SHGetIDListFromObject(pItem, &pidlIcon))) {
                    if (SHGetFileInfoW((LPCWSTR)pidlIcon, 0, &sfi2, sizeof(sfi2), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_PIDL))
                        hIcon = sfi2.hIcon;
                    ILFree(pidlIcon);
                }
            }

            allApps.push_back({appName, filePath, appId, pItem, hIcon});
        } else {
            pItem->Release();
        }
    }

    pEnum->Release();
    pAppsFolder->Release();
}

void AppLauncher::FilterApps(const std::wstring& search) {
    filteredApps.clear();
    
    if (search.empty()) {
        filteredApps = allApps;
        return;
    }

    std::wstring searchLower = search;
    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::towlower);

    std::vector<App> startsWithApps;
    std::vector<App> containsApps;

    for (const auto& app : allApps) {
        std::wstring nameLower = app.name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::towlower);
        
        if (nameLower.find(searchLower) != std::wstring::npos) {
            if (nameLower.find(searchLower) == 0) {
                startsWithApps.push_back(app);
            } else {
                containsApps.push_back(app);
            }
        }
    }
    
    filteredApps.insert(filteredApps.end(), startsWithApps.begin(), startsWithApps.end());
    filteredApps.insert(filteredApps.end(), containsApps.begin(), containsApps.end());
}

const std::vector<App>& AppLauncher::GetFilteredApps() const {
    return filteredApps;
}

bool AppLauncher::LaunchApp(int index) {
    if (index >= 0 && index < (int)filteredApps.size()) {
        const App& app = filteredApps[index];
        
        // Check if it's a real UWP AppUserModelID (not a file path)
        if (!app.appId.empty() && 
            app.appId.find(L":\\") == std::wstring::npos &&
            app.appId.find(L"\\\\") == std::wstring::npos) {
            
            AllowSetForegroundWindow(ASFW_ANY);
            
            IApplicationActivationManager* pActivator = nullptr;
            HRESULT hr = CoCreateInstance(CLSID_ApplicationActivationManager_Local, nullptr, CLSCTX_LOCAL_SERVER,
                                          IID_PPV_ARGS(&pActivator));
            if (SUCCEEDED(hr)) {
                DWORD pid = 0;
                hr = pActivator->ActivateApplication(app.appId.c_str(), nullptr, AO_NONE, &pid);
                pActivator->Release();
                
                if (SUCCEEDED(hr)) {
                    return true;
                }
            }
        }
        
        // If we have an IShellItem (from AppsFolder), try PIDL method first
        if (app.item != nullptr) {
            PIDLIST_ABSOLUTE pidl = nullptr;
            HRESULT hr = SHGetIDListFromObject(app.item, &pidl);
            
            if (SUCCEEDED(hr) && pidl) {
                SHELLEXECUTEINFOW sei = {};
                sei.cbSize = sizeof(sei);
                sei.fMask = SEE_MASK_IDLIST;
                sei.lpIDList = (LPITEMIDLIST)pidl;
                sei.nShow = SW_SHOWNORMAL;
                
                if (ShellExecuteExW(&sei)) {
                    ILFree(pidl);
                    return true;
                }
                
                ILFree(pidl);
            }
            
            // Fallback: Try getting file path from IShellItem
            PWSTR pszPath = nullptr;
            hr = app.item->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
            
            if (SUCCEEDED(hr) && pszPath) {
                HINSTANCE result = ShellExecuteW(nullptr, nullptr, pszPath, nullptr, nullptr, SW_SHOWNORMAL);
                CoTaskMemFree(pszPath);
                
                if ((INT_PTR)result > 32) {
                    return true;
                }
            }
        }
        
        // For Start Menu items (shortcuts), launch directly
        if (!app.path.empty()) {
            SHELLEXECUTEINFOW sei = { sizeof(sei) };
            sei.fMask = SEE_MASK_NOCLOSEPROCESS;
            sei.lpFile = app.path.c_str();
            sei.nShow = SW_SHOWNORMAL;
            
            if (ShellExecuteExW(&sei)) {
                if (sei.hProcess) {
                    CloseHandle(sei.hProcess);
                }
                return true;
            }
        }
        
        return false;
    }
    return false;
}

int AppLauncher::GetFilteredCount() const {
    return (int)filteredApps.size();
}

void AppLauncher::Reset() {
    // Release all stored IShellItem pointers and icons before clearing
    for (auto& app : allApps) {
        if (app.item) { app.item->Release(); app.item = nullptr; }
        if (app.icon) { DestroyIcon(app.icon); app.icon = nullptr; }
    }
    allApps.clear();
    filteredApps.clear();

    CoInitialize(NULL);

    WCHAR path[MAX_PATH];

    if (SHGetFolderPathW(NULL, CSIDL_COMMON_PROGRAMS, NULL, 0, path) == S_OK)
        ScanStartMenu(path);

    if (SHGetFolderPathW(NULL, CSIDL_PROGRAMS, NULL, 0, path) == S_OK)
        ScanStartMenu(path);

    ScanAppsFolder();

    std::sort(allApps.begin(), allApps.end(), [](const App& a, const App& b) {
        return a.name < b.name;
    });

    auto newEndR = std::unique(allApps.begin(), allApps.end(), [](const App& a, const App& b) {
        return a.name == b.name;
    });
    for (auto it = newEndR; it != allApps.end(); ++it)
        if (it->icon) { DestroyIcon(it->icon); it->icon = nullptr; }
    allApps.erase(newEndR, allApps.end());

    filteredApps = allApps;

    CoUninitialize();
}
