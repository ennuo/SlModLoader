#pragma warning(disable: 4073)
#pragma init_seg(lib)

#include <windows.h>

static SlPreInitGlobals gSlPreInit;
uintptr_t SlPreInitGlobals::kMemoryBase = 0;
SlPreInitGlobals::SlPreInitGlobals()
{
    kMemoryBase = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
}

void PatchExecutableSection(void* address, void* data, int size)
{
    DWORD old_protect;
    VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &old_protect);
    memcpy(address, data, size);
    VirtualProtect(address, size, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), address, size);
}