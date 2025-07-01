#ifndef DINPUT8_H
#define DINPUT8_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <unknwn.h>

#define DINPUT8_API __declspec(dllexport)

typedef HRESULT(WINAPI *DirectInput8Create_t)(
	HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID * ppvOut,
	LPUNKNOWN punkOuter
);

extern DirectInput8Create_t oDirectInput8Create;
extern HMODULE DInput8Dll;

DINPUT8_API HRESULT WINAPI DirectInput8Create(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID * ppvOut,
    LPUNKNOWN punkOuter
);

#endif // DINPUT8_H