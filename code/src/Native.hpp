#pragma once

#include <cstdint>
#include <concepts>

#define LOG(x, ...) { printf(""##x##"\n", __VA_ARGS__); }

extern uintptr_t gMemoryBase;
const uintptr_t kExecutableBase = 0x00400000;

#define ASLR(address) (SlPreInitGlobals::kMemoryBase + (uintptr_t)address - kExecutableBase)

#define INLINE_ALWAYS __forceinline

#define CREATE_HOOK(address, hook, original) MH_CreateHook((void*)ASLR(address), (void*)&hook, (void**)original);
#define CREATE_MEMBER_HOOK(address, hook, original) MH_CreateHook((void*)ASLR(address), (void*)GetFnAddr(&hook), (void**)original);

#define READ_EAX(name) __asm mov name, eax
#define READ_ECX(name) __asm mov name, ecx
#define READ_EDI(name) __asm mov name, edi
#define READ_ESI(name) __asm mov name, esi

#define WRITE_EAX(name) __asm mov eax, name
#define WRITE_ECX(name) __asm mov ecx, name
#define WRITE_EDI(name) __asm mov edi, name
#define WRITE_ESI(name) __asm mov esi, name

#define PUSH_REGISTER(name) __asm push name
#define POP_REGISTER(name) __asm pop name

#define RETURN_POP(n) __asm ret n

#define INLINE_ASM_PROLOGUE __asm \
{ \
    __asm push ebp \
    __asm mov ebp, esp \
    __asm sub esp, __LOCAL_SIZE \
    __asm push esi \
    __asm push ebx \
    __asm push edi \
}

#define INLINE_ASM_EPILOGUE __asm \
{ \
    __asm pop edi \
    __asm pop ebx \
    __asm pop esi \
    __asm mov esp, ebp \
    __asm pop ebp \
    __asm ret \
}

#define INLINE_ASM_EPILOGUE_N(x) __asm \
{ \
    __asm pop edi \
    __asm pop ebx \
    __asm pop esi \
    __asm mov esp, ebp \
    __asm pop ebp \
    __asm ret x \
}
    
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

    T& operator++()
    {
        *mData++;
        return *mData;
    }

    T operator++(int)
    {
        T old = *mData;
        operator++();
        return old;
    }

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

#define DEFINE_STATIC_MEMBER_FN_0(fnName, retnType, addr) \
    static INLINE_ALWAYS retnType fnName() { \
    typedef retnType(__cdecl *_##fnName_t)(); \
    const static uintptr_t address = ASLR(addr); \
    _##fnName_t fn = *(_##fnName_t*)&address; \
    return fn(); \
}

#define DEFINE_STATIC_MEMBER_FN_1(fnName, returnType, ADDR, ...) \
    inline static uintptr_t __m_p##fnName{ASLR(ADDR)}; \
    template <typename T1> \
    static INLINE_ALWAYS returnType fnName(T1 && t1) { \
    typedef returnType(__cdecl *delegate)( __VA_ARGS__); \
    delegate fn = *(delegate*)&__m_p##fnName; \
    return fn(t1); \
}

#define DEFINE_MEMBER_FN_0(fnName, retnType, addr) \
    INLINE_ALWAYS retnType fnName() { \
    typedef retnType(__thiscall *_##fnName_t)(uintptr_t); \
    const static uintptr_t address = ASLR(addr); \
    _##fnName_t fn = *(_##fnName_t*)&address; \
    return fn((uintptr_t)this); \
}

#define DEFINE_MEMBER_FN_1(fnName, returnType, ADDR, ...) \
    inline static uintptr_t __m_p##fnName{ASLR(ADDR)}; \
    template <typename T1> \
    INLINE_ALWAYS returnType fnName(T1 && t1) { \
    typedef returnType(__thiscall *delegate)(decltype(this), __VA_ARGS__); \
    delegate fn = *(delegate*)&__m_p##fnName; \
    return fn(this, t1); \
}

#define DEFINE_MEMBER_FN_2(fnName, retnType, addr, ...) \
    template <typename T1, typename T2> \
    INLINE_ALWAYS retnType fnName(T1 && t1, T2 && t2) { \
    typedef retnType(__thiscall *_##fnName_t)(uintptr_t, __VA_ARGS__); \
    const static uintptr_t address = ASLR(addr); \
    _##fnName_t fn = *(_##fnName_t*)&address; \
    return fn((uintptr_t)this, t1, t2); \
}

#define DEFINE_MEMBER_FN_3(fnName, retnType, addr, ...) \
    template <typename T1, typename T2, typename T3> \
    INLINE_ALWAYS retnType fnName(T1 && t1, T2 && t2, T3 && t3) { \
    typedef retnType(__thiscall *_##fnName_t)(uintptr_t, __VA_ARGS__); \
    const static uintptr_t address = ASLR(addr); \
    _##fnName_t fn = *(_##fnName_t*)&address; \
    return fn((uintptr_t)this, t1, t2, t3); \
}

#define DEFINE_MEMBER_FN_4(fnName, retnType, addr, ...) \
    template <typename T1, typename T2, typename T3, typename T4> \
    INLINE_ALWAYS retnType fnName(T1 && t1, T2 && t2, T3 && t3, T4 && t4) { \
    typedef retnType(__thiscall *_##fnName_t)(uintptr_t, __VA_ARGS__); \
    const static uintptr_t address = ASLR(addr); \
    _##fnName_t fn = *(_##fnName_t*)&address; \
    return fn((uintptr_t)this, t1, t2, t3, t4); \
}

#define DEFINE_MEMBER_FN_5(fnName, retnType, addr, ...) \
    template <typename T1, typename T2, typename T3, typename T4, typename T5> \
    INLINE_ALWAYS retnType fnName(T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5) { \
    typedef retnType(__thiscall *_##fnName_t)(uintptr_t, __VA_ARGS__); \
    const static uintptr_t address = ASLR(addr); \
    _##fnName_t fn = *(_##fnName_t*)&address; \
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

void PatchExecutableSection(void* address, void* data, int size);