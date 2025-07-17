#pragma once

#include <vector>
#include <map>

#include "Core.hpp"
#include "ResourceManager.hpp"
#include "Sl/Plugins/Interfaces.hpp"

inline const char* kTextureSet_MiniMapIcons = "Dynamic Minimap Resources";
inline const char* kTextureSet_CharacterSelect = "Character Select Icon Resources";

class SiffLoadSet;
class SlCustomRacer {
public:
    inline SlCustomRacer() { memset(this, 0, sizeof(SlCustomRacer)); }
public:
    SlStringT<char> InternalId;
    SlStringT<char> BaseId;
    SlStringT<char> DisplayName;

    SlStringT<char> FrontendResources;
    SlStringT<char> GameResources;

    SlStringT<char> CharacterFile;
    SlStringT<char> FrontendCharacterFile;
    
    SlStringT<char> CarAnimationFilePrefix;
    SlStringT<char> BoatAnimationFilePrefix;
    SlStringT<char> PlaneAnimationFilePrefix;
    SlStringT<char> TransformAnimationFilePrefix;

    int NameHash;
    int BaseHash;
    int RaceResultsHash;
    int MinimapIconHash;
    int SelectIconHash;
    int VersusPortraitHash;

    bool IsModelSwap;
};

class SlGlobalTextureSet {
public:
    SlGlobalTextureSet(const char* name, const std::vector<SlStringT<char>>& files);
public:
    inline bool IsEnabled() const { return Enabled; }
    inline bool IsDisabled() const { return !Enabled; }
    inline int GetName() const { return Name; }
    void Enable();
    void Disable();
private:
    int Name;
    std::vector<SlStringT<char>> Files;
    ResourceList Resources;
    int Priority;
    bool Enabled;
    bool Linked;
};

class SlModManager {
public:
    void LoadFrontendAssets(ResourceList* resources);
    SlCustomRacer* GetRacerByHash(int hash);

    void SetTextureSetEnabled(const char* name, bool enable);
    void MakeTextureSet(const char* name, const std::vector<SlStringT<char>>& files);
    void DestroyTextureSet(const char* name);
    SlGlobalTextureSet* GetTextureSet(const char* name);
    inline const std::vector<SlGlobalTextureSet*>& GetTextureSets() { return TextureSets; }
    void SetupRacerTextureSets();
public:
    std::vector<SlCustomRacer*> Racers;
private:
    std::vector<SlGlobalTextureSet*> TextureSets;
};

inline SlModManager* gSlMod(nullptr);
inline SlModInterface gSlModApi;