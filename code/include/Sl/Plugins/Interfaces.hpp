#pragma once

#include <string>

#define SLAPI __declspec(dllexport)

enum eModType {
    kModType_File,
    kModType_Racer,
    kModType_Track,
    kModType_GameMode
};

typedef unsigned int SlModHandle;

struct SlModInfo
{
    enum { kInfoVersion = 1 };
    unsigned int InfoVersion;
        
    std::string Name;
    eModType Type;
    unsigned int Version;
};

enum
{
    kInterface_Invalid = 0,
    kInterface_Detour,
    kInterface_GameMode,
    kInterface_Max
};

struct SlModInterface
{
    unsigned int Version;
    void* (*GetInterface)(unsigned int id);
    void* (*GetModInfo)(const char* name);
    void* (*GetAllocator)();
};

struct SlHookInterface
{
    enum { kInterfaceVersion = 1 };
    unsigned int InterfaceVersion;

    void* (*Create)(SlModHandle mod, void* target, void* detour, void** original);
};

typedef bool(*_SlMod_Load)(const SlModInterface* sl);
