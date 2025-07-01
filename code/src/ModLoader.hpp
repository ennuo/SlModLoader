#pragma once

#include <vector>

#include "Core.hpp"
#include "Sl/Plugins/Interfaces.hpp"

class SlCustomRacer {
public:
    inline SlCustomRacer() : InternalId(), BaseId(), DisplayName(), FrontendResources(), GameResources()
    {}
public:
    SlStringT<char> InternalId;
    SlStringT<char> BaseId;
    SlStringT<char> DisplayName;
    SlStringT<char> FrontendResources;
    SlStringT<char> GameResources;
};

class SlModManager {
public:
    std::vector<SlCustomRacer*> Racers;
};

inline SlModManager* gSlMod(nullptr);
inline SlModInterface gSlModApi;