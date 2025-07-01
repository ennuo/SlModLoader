#pragma warning(disable: 4073)
#pragma init_seg(lib)

#include <windows.h>

static SlPreInitGlobals gSlPreInit;
uintptr_t SlPreInitGlobals::kMemoryBase = 0;
SlPreInitGlobals::SlPreInitGlobals()
{
    kMemoryBase = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
}