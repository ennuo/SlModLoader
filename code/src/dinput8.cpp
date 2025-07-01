#include "dinput8.h"

DirectInput8Create_t oDirectInput8Create = nullptr;
HMODULE DInput8Dll;
extern void ClvMain();
extern void ClvClose();
uintptr_t gMemoryBase;

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            CHAR syspath[MAX_PATH];
            if (!GetSystemDirectoryA(syspath, MAX_PATH)) return FALSE;
            strcat(syspath, "\\dinput8.dll");

            DInput8Dll = LoadLibraryA(syspath);
            if (!DInput8Dll) return FALSE;

            oDirectInput8Create = (DirectInput8Create_t)GetProcAddress(DInput8Dll, "DirectInput8Create");
            gMemoryBase = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));

            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
        {
            ClvClose();
            FreeLibrary(DInput8Dll);
            break;
        }
    }

    return TRUE;
}

DINPUT8_API HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
    #pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)

    ClvMain();
    
    if (oDirectInput8Create)
        return oDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    return S_FALSE;
}