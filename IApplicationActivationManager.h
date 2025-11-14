#pragma once
#include <windows.h>
#include <unknwn.h>

// Define IApplicationActivationManager for MinGW
enum ACTIVATEOPTIONS {
    AO_NONE = 0x00000000,
    AO_DESIGNMODE = 0x00000001,
    AO_NOERRORUI = 0x00000002,
    AO_NOSPLASHSCREEN = 0x00000004
};

#ifndef __IApplicationActivationManager_INTERFACE_DEFINED__
#define __IApplicationActivationManager_INTERFACE_DEFINED__

MIDL_INTERFACE("2e941141-7f97-4756-ba1d-9decde894a3d")
IApplicationActivationManager : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE ActivateApplication(
        LPCWSTR appUserModelId,
        LPCWSTR arguments,
        ACTIVATEOPTIONS options,
        DWORD *processId) = 0;

    virtual HRESULT STDMETHODCALLTYPE ActivateForFile(
        LPCWSTR appUserModelId,
        IShellItemArray *itemArray,
        LPCWSTR verb,
        DWORD *processId) = 0;

    virtual HRESULT STDMETHODCALLTYPE ActivateForProtocol(
        LPCWSTR appUserModelId,
        IShellItemArray *itemArray,
        DWORD *processId) = 0;
};

#endif

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(IApplicationActivationManager, 0x2e941141, 0x7f97, 0x4756, 0xba, 0x1d, 0x9d, 0xec, 0xde, 0x89, 0x4a, 0x3d)
#endif

static const GUID CLSID_ApplicationActivationManager_Local = 
    {0x45BA127D, 0x10A8, 0x46EA, {0x8A, 0xB7, 0x56, 0xEA, 0x90, 0x78, 0x94, 0x3C}};
