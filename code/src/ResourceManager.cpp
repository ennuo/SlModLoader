#include "ResourceManager.hpp"

SlReloc<ResourceManager> g_pResourceManager(0x0);

ResourceList::~ResourceList()
{
    UnloadResources();
}

void ResourceList::AddResource(SlStringT<char> const& path)
{
    AddResource(path, kResourceType_Invalid, false, false);
}

void ResourceList::AddResource(SlStringT<char> const& path, eResourceType type, bool a, bool b)
{
    sGameFile file;
    file.Path = path;
    file.Type = type;
    file.A = a;
    file.B = b;

    if (m_GameFiles == nullptr) 
        m_GameFiles = new std::vector<sGameFile>();

    for (const auto& file : *m_GameFiles)
    {
        if (file.Path.CompareCaseInsensitive(path) != 0)
            return;
    }

    m_GameFiles->push_back(file);
}

void ResourceList::UnloadResources()
{
    if (m_GameFiles != nullptr)
    {
        for (const auto& file : *m_GameFiles)
        {
            // g_pResourceManager->UnloadResource(file);
        }
    }

    ClearResourceList();
}

void ResourceList::ClearResourceList()
{
    if (m_GameFiles == nullptr) return;
    delete m_GameFiles;
}