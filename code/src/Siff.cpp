#include "Siff.hpp"

Siff::Object::TableEntry* SiffObjectDefManager::GetObjectDef(unsigned int hash)
{
    auto header = m_objectDefBlockList.m_pStart;
    while (header != nullptr)
    {
        int index = (header->m_Hashes[hash % header->m_NumObjects] ^ hash) % header->m_NumObjects;
        auto entry = &header->m_Objects[index];
        if (entry->Hash == hash)
            return entry;

        header = header->m_pNext;
    }

    // Fallback to linear search if we were unable to find the entry,
    // this is really just a temporary hack for me not wanting to figure out
    // what hashing system they use.
    // I could technically just replace it entirely, but that's dumb.
    return GetObjectDef_Linear(hash);
}

Siff::Object::TableEntry* SiffObjectDefManager::GetObjectDef_Linear(unsigned int hash)
{
    auto header = m_objectDefBlockList.m_pStart;
    while (header != nullptr)
    {
        for (int i = 0; i < header->m_NumObjects; ++i)
        {
            auto entry = &header->m_Objects[i];
            if (entry->Hash == hash)
                return entry;
        }

        header = header->m_pNext;
    }

    return nullptr;
}