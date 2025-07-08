#include "Game.hpp"
#include "ModLoader.hpp"

void(*SlPrintf)(const char*, ...);
void(*_SoftResetManager_DisablePolling)(void*);

uintptr_t GetPhysicsManager()
{
    return (uintptr_t)LoadPointer(0xA9A3CC);
}

uintptr_t GetClockManager()
{
    return (uintptr_t)LoadPointer(0xA9A36C);
}

uintptr_t GetSoftResetManager()
{
    return (uintptr_t)LoadPointer(0x5C9D60);
}

void SetupGameNatives()
{
    *(void**)(&SlPrintf) = (void*)GetAddress(0x205820);
    *(void**)(&_SoftResetManager_DisablePolling) = (void*)GetAddress(0x205820);
}

void PauseGame()
{
    *(int*)(*(uintptr_t**)(GetPhysicsManager() + 0xd0) + 0x14) = 0;
    auto clock = GetClockManager();
    *(int*)(clock + 0x16c) = 0;
    *(int*)(clock + 0x168) = 1;
}

void UnPauseGame()
{
    *(int*)(*(uintptr_t**)(GetPhysicsManager() + 0xd0) + 0x14) = 1;
    auto clock = GetClockManager();
    *(int*)(clock + 0x16c) = 1;
    *(int*)(clock + 0x168) = 0;
}

void SoftResetManager_DisablePolling()
{

}

bool RacerInfo::IsMod() const
{
    return CustomRacerIndex >= 0;
}

bool RacerInfo::IsModelSwap() const
{
    if (!IsMod()) return false;
    return GetCustomRacer()->IsModelSwap;
}

SlCustomRacer* RacerInfo::GetCustomRacer() const
{
    if (!IsMod()) return nullptr;
    return gSlMod->Racers[CustomRacerIndex];
}

