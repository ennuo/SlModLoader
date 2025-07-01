#ifndef CORE_H
#define CORE_H

#include <string.h>

typedef void* (*SlGlobalMalloc)(unsigned int size, unsigned int align, bool array, bool);
typedef void (*SlGlobalFree)(void* p, bool array);

class SlCoreGlobals {
public:
    SlGlobalMalloc Malloc;
    SlGlobalFree Free;
};

inline SlReloc<SlCoreGlobals*> gSlCoreGlobals(0xBC58C0);

void gSlFree(void* p, bool array);
void* gSlMalloc(unsigned int size);
void* gSlMallocAlign(unsigned int size, unsigned int align);
void* gSlMallocAlign(unsigned int size, unsigned int align, bool array);

template <typename T>
class SlFilePtrBasic {
public:
    T* Data;
};

template <typename T>
class SlStringT {
public:

    SlStringT()
    {
        memset(this, 0, sizeof(SlStringT<T>));
    }

    SlStringT(char const* s) : SlStringT()
    {
        IntAssign(s, IntGetLength(s));
    }

    SlStringT(char const* s, unsigned int len) : SlStringT()
    {
        IntAssign(s, len);
    }
    
    SlStringT(SlStringT<char>& s) : SlStringT()
    {
        IntAssign(s.m_Data, s.m_Length);
    }

    ~SlStringT()
    {
        IntDeleteAll();
    }

    void operator=(char const* s)
    {
        IntAssign(s, IntGetLength(s));
    }

    void operator=(SlStringT<T> const& s)
    {
        if (&s == this) return;
        IntAssign(s.m_Data, s.m_Length);
    }
    
    void IntAssign(char const* s, unsigned int len)
    {
        if (s == nullptr || len == 0)
        {
            IntDeleteAll();
            return;
        }

        IntReserve(len + 1);
        m_Length = len;
        memcpy(m_Data, s, len);
        m_Data[len] = '\0';
        m_Hash = 0;
    }

    void IntReserve(unsigned int len)
    {
        if (len <= m_Capacity) return;
        m_Capacity = len;

        T* dest = (T*)gSlMallocAlign(len * sizeof(T), 0x10);
        if (m_Data != nullptr)
            memcpy(dest, m_Data, m_Length);
        
        if (m_Ref == 0 && m_Data != nullptr)
            gSlFree(m_Data, true);
        
        dest[m_Length] = '\0';
        m_Data = (T*)dest;
        m_Ref = 0;
    }

    void IntDeleteAll()
    {
        if (m_Ref == 0 && m_Data != nullptr)
            gSlFree(m_Data, false);
        m_Data = nullptr;
        m_Hash = 0;
        m_Length = 0;
        m_Capacity = 0;
    }

    int CompareCaseInsensitive(const char* s) const
    {
        return _stricmp(m_Data, s);
    }

    operator T*() const
    {
        return m_Data;
    }
    
public:
    static int IntGetLength(char const* s)
    {
        if (s == nullptr) return 0;
        int len = 0;
        while (s[len] != '\0') len++;
        return len;
    }
public:
    T* m_Data;
    unsigned int _pad0;
    unsigned int m_Hash;
    int m_Length;
    int m_Capacity;
    unsigned int m_Ref;
    unsigned int _pad1;
    unsigned int _pad2;
};

static_assert(sizeof(SlStringT<char>) == 0x20, "SlStringT<char> type size is incorrect!");

#endif // CORE_H