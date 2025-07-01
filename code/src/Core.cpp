#include "Core.hpp"
#include "Native.hpp"

#include <cstdio>
#include <windows.h>

void gSlFree(void* p, bool array)
{
    (*gSlCoreGlobals).Free(p, array);
}

void* gSlMalloc(unsigned int size)
{
    return gSlMallocAlign(size, 4);
}

void* gSlMallocAlign(unsigned int size, unsigned int align)
{
    return (*gSlCoreGlobals).Malloc(size, align, false, 0);
}

void* gSlMallocAlign(unsigned int size, unsigned int align, bool array)
{
    return (*gSlCoreGlobals).Malloc(size, align, array, 0);
}

void* operator new(size_t n) 
{
    return gSlMalloc(n);
}

void operator delete(void* p) 
{
    gSlFree(p, false);
}

void* operator new[](size_t sz)
{
	return gSlMallocAlign(sz, 4, true);
}

void operator delete[](void* p)
{
    return gSlFree(p, true);
}