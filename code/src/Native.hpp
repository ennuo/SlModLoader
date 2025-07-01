#pragma once

#include <cstdint>
#include <concepts>

extern uintptr_t gMemoryBase;
const uintptr_t gExecutableBase = 0x00400000;

#define ASLR(address) (SlPreInitGlobals::kMemoryBase + (uintptr_t)address - gExecutableBase)

#define INLINE_ALWAYS __forceinline

#define CREATE_HOOK(address, hook, original) MH_CreateHook((void*)((uintptr_t)gMemoryBase + address), (void*)&hook, (void**)original);
#define CREATE_MEMBER_HOOK(address, hook, original) MH_CreateHook((void*)((uintptr_t)gMemoryBase + address), (void*)GetFnAddr(&hook), (void**)original);

class SlPreInitGlobals {
public:
    SlPreInitGlobals();
    static uintptr_t kMemoryBase;
};

template<typename T>
class SlReloc {
public:
    SlReloc(uintptr_t offset) : mData((T*)ASLR(offset)) {}
    
    operator T() const { return *mData; }

    T operator->() const requires (std::is_pointer_v<T>) { return *mData; }
    T* operator->() const requires (!std::is_pointer_v<T>) { return mData; }

    T* operator&() const { return mData; }
    T& operator=(const T& rhs) { *mData = rhs; return *mData; }
private:
    T* mData;
private:
    SlReloc();
    SlReloc(SlReloc& rhs);
    SlReloc& operator=(SlReloc& rhs);
};

template <typename T>
inline T LoadMemory(int address)
{
    return *(T*)((uintptr_t)gMemoryBase + address);
}

inline char* LoadPointer(int address)
{
    return *(char**)((uintptr_t)gMemoryBase + address);
}

inline char* GetAddress(int address)
{
    return (char*)((uintptr_t)gMemoryBase + address);
}

#define DEFINE_MEMBER_FN_0(fnName, retnType, addr) \
    INLINE_ALWAYS retnType fnName() { \
    typedef retnType(__thiscall *_##fnName##_type)(uintptr_t); \
    const static uintptr_t address = ASLR(addr); \
    _##fnName##_type fn = *(_##fnName##_type*)&address; \
    return fn((uintptr_t)this); \
}

#define DEFINE_MEMBER_FN_1(fnName, retnType, addr, ...) \
    template <typename T1> \
    INLINE_ALWAYS retnType fnName(T1 && t1) { \
    typedef retnType(__thiscall *_##fnName##_type)(uintptr_t, __VA_ARGS__); \
    const static uintptr_t address = ASLR(addr); \
    _##fnName##_type fn = *(_##fnName##_type*)&address; \
    return fn((uintptr_t)this, t1); \
}

#define DEFINE_MEMBER_FN_2(fnName, retnType, addr, ...) \
    template <typename T1, typename T2> \
    INLINE_ALWAYS retnType fnName(T1 && t1, T2 && t2) { \
    typedef retnType(__thiscall *_##fnName##_type)(uintptr_t, __VA_ARGS__); \
    const static uintptr_t address = ASLR(addr); \
    _##fnName##_type fn = *(_##fnName##_type*)&address; \
    return fn((uintptr_t)this, t1, t2); \
}

#define DEFINE_MEMBER_FN_3(fnName, retnType, addr, ...) \
    template <typename T1, typename T2, typename T3> \
    INLINE_ALWAYS retnType fnName(T1 && t1, T2 && t2, T3 && t3) { \
    typedef retnType(__thiscall *_##fnName##_type)(uintptr_t, __VA_ARGS__); \
    const static uintptr_t address = ASLR(addr); \
    _##fnName##_type fn = *(_##fnName##_type*)&address; \
    return fn((uintptr_t)this, t1, t2, t3); \
}

#define DEFINE_MEMBER_FN_4(fnName, retnType, addr, ...) \
    template <typename T1, typename T2, typename T3, typename T4> \
    INLINE_ALWAYS retnType fnName(T1 && t1, T2 && t2, T3 && t3, T4 && t4) { \
    typedef retnType(__thiscall *_##fnName##_type)(uintptr_t, __VA_ARGS__); \
    const static uintptr_t address = ASLR(addr); \
    _##fnName##_type fn = *(_##fnName##_type*)&address; \
    return fn((uintptr_t)this, t1, t2, t3, t4); \
}

#define DEFINE_MEMBER_FN_5(fnName, retnType, addr, ...) \
    template <typename T1, typename T2, typename T3, typename T4, typename T5> \
    INLINE_ALWAYS retnType fnName(T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5) { \
    typedef retnType(__thiscall *_##fnName##_type)(uintptr_t, __VA_ARGS__); \
    const static uintptr_t address = ASLR(addr); \
    _##fnName##_type fn = *(_##fnName##_type*)&address; \
    return fn((uintptr_t)this, t1, t2, t3, t4, t5); \
}

template <typename T>
uintptr_t GetFnAddr(T src)
{
	union
	{
		uintptr_t	u;
		T			t;
	} data;

	data.t = src;

	return data.u;
}