#pragma once

#include "Core.hpp"

#include <vector>

enum eResourceType {
    kResourceType_Invalid,
    kResourceType_Binary,
    kResourceType_Text,
    kResourceType_SumoEngine,
    kResourceType_SumoTool,
    kResourceType_SoundBank,
    kResourceType_SoundBank2,
    kResourceType_BinaryEncrypted
};

class sGameFile {
public:
    sGameFile() : Type(), Path(), A(), B() {}
    sGameFile(sGameFile const& file) : Type(file.Type), Path(file.Path), A(file.A), B(file.B) {}
    sGameFile& operator=(const sGameFile& file)
    {
        Type = file.Type;
        Path = file.Path;
        A = file.A; // GPU?
        B = file.B;
        return *this;
    }
public:
    eResourceType Type;
    SlStringT<char> Path;
    bool A;
    bool B;
};

static_assert(sizeof(sGameFile) == 0x28, "sGameFile type size is incorrect!");

class ResourceManager {
public:
    DEFINE_STATIC_MEMBER_FN_1(FileExists, int, 0x006fdbf0, const SlStringT<char>& resource);
    DEFINE_MEMBER_FN_1(UnloadResource, void, 0x006fe2d0, const SlStringT<char>& resource);
    DEFINE_MEMBER_FN_3(LoadResource, void, 0x006feff0, const SlStringT<char>& resource, eResourceType type, bool b);
    DEFINE_MEMBER_FN_1(IsLoading, bool, 0x006fe3a0, const SlStringT<char>& resource);
    DEFINE_MEMBER_FN_1(GetResource, char*, 0x006fe590, const SlStringT<char>& resource);
};

class ResourceList {
public:
    ~ResourceList();
public:
    bool IsLoaded();
    int GetSize();
    void UnloadResources();
    void StartLoadResources();
    DEFINE_MEMBER_FN_0(SortList, void, 0x0072b570);
    void AddResource(SlStringT<char> const& path);
    void AddResource(SlStringT<char> const& path, eResourceType type, bool);
    void ClearResourceList();
public:
    std::vector<sGameFile>* m_GameFiles;
};

static_assert(sizeof(std::vector<sGameFile>) == 0xc, "Vector is padded in your compiler!");

inline SlReloc<ResourceManager*> gResourceManager(0x00bd0cf4);