#include <Sl/Plugins/Interfaces.hpp>

#include <Windows.h>
#include <cstdint>

uintptr_t gMemoryBase;

void PatchExecutableSection(uintptr_t addy, void* data, size_t size)
{
    void* address = (void*)(addy + gMemoryBase);

    DWORD old_protect;
    VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &old_protect);
    memcpy(address, data, size);
    VirtualProtect(address, size, old_protect, &old_protect);
}

void RemoveIntroCinematics()
{
    uint8_t NOP2[2] = { 0x66, 0x90 };
    uint8_t NOP6[6] = { 0x66, 0x90, 0x66, 0x90, 0x66, 0x90 };
    uint8_t PUSH_PI[2] = { 0xd9, 0xeb };
    uint8_t AL_ONE[5] = { 0xb0, 0x01, 0x90, 0x90, 0x90 };

    PatchExecutableSection(0x354503, PUSH_PI, sizeof(PUSH_PI));
    PatchExecutableSection(0x3545B7, AL_ONE, sizeof(AL_ONE));
    PatchExecutableSection(0x356C99, NOP2, sizeof(NOP2));
    PatchExecutableSection(0x356Ca6, NOP6, sizeof(NOP6));

    FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
}

extern "C" void SLAPI SlMod_Load(SlModInterface* sl)
{
    gMemoryBase = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
    RemoveIntroCinematics();
}