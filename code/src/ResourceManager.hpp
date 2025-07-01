#pragma once

#include "Core.hpp"

#include <vector>

enum eResourceType {
    kResourceType_Invalid,
    kResourceType_Binary,
    kResourceType_Text,
    kResourceType_SumoEngine,
    kResourceType_SumoEngineSh2,
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
        A = file.A;
        B = file.B;
        return *this;
    }
public:
    eResourceType Type;
    SlStringT<char> Path;
    bool A;
    bool B;
};

class ResourceManager {
public:
    DEFINE_MEMBER_FN_1(UnloadResource, void, 0x006fe2d0, const SlStringT<char>& resource);
};

class ResourceList {
public:
    ~ResourceList();
public:
    bool IsLoaded();
    int GetSize();
    void UnloadResources();
    void StartLoadResources();
    void AddResource(SlStringT<char> const& path);
    void AddResource(SlStringT<char> const& path, eResourceType type, bool, bool);
    void ClearResourceList();
private:
    std::vector<sGameFile>* m_GameFiles;
};

void SetupResourceManagerNatives();