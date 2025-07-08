#pragma once

#include <vector>
#include <map>

#include "Core.hpp"
#include "ResourceManager.hpp"
#include "Sl/Plugins/Interfaces.hpp"


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

    SlStringT<char> RootFEEntity;
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

class SlModManager {
public:
    void LoadFrontendAssets(ResourceList* resources);
    SlCustomRacer* GetRacerByHash(int hash);
public:
    std::vector<SlCustomRacer*> Racers;

};

inline SlModManager* gSlMod(nullptr);
inline SlModInterface gSlModApi;