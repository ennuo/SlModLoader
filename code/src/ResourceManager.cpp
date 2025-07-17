#include "ResourceManager.hpp"

ResourceList::~ResourceList()
{
    UnloadResources();
}

void ResourceList::AddResource(SlStringT<char> const& path)
{
    AddResource(path, kResourceType_Invalid, false);
}

void ResourceList::StartLoadResources()
{
    if (m_GameFiles == nullptr) return;
    for (const auto& file : *m_GameFiles)
        gResourceManager->LoadResource(file.Path, file.Type, file.A);
}

bool ResourceList::IsLoaded() const
{
    if (m_GameFiles == nullptr) return true;
    for (const auto& file : *m_GameFiles)
    {
        if (gResourceManager->IsLoading(file.Path))
            return false;
    }
    
    return true;
}

int ResourceList::GetSize()
{
    if (m_GameFiles == nullptr) return 0;
    return m_GameFiles->size();
}

void ResourceList::AddResource(SlStringT<char> const& path, eResourceType type, bool a)
{
    sGameFile file;
    file.Path = path;
    file.Type = type;
    file.A = a;

    if (m_GameFiles == nullptr) 
        m_GameFiles = new std::vector<sGameFile>();

    for (const auto& file : *m_GameFiles)
    {
        if (file.Path.CompareCaseInsensitive(path) == 0)
            return;
    }

    m_GameFiles->push_back(file);
}

void ResourceList::UnloadResources()
{
    if (m_GameFiles != nullptr)
    {
        for (const auto& file : *m_GameFiles)
            gResourceManager->UnloadResource(file.Path);
    }

    ClearResourceList();
}

void ResourceList::ClearResourceList()
{
    if (m_GameFiles == nullptr) return;
    delete m_GameFiles;
    m_GameFiles = nullptr;
}