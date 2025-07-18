#include "SceneManager.hpp"
#include "ResourceManager.hpp"
#include "Jenkins.hpp"
#include "Game.hpp"
#include "ModLoader.hpp"

#include <cstdarg>

#include <Windows.h>

bool CTextureObject::SetNewTexture(int hash, SiffLoadSet const* siff)
{
    const static uintptr_t address = ASLR(0x00495f90);
    __asm
    {
        mov esi, hash
        mov edi, siff
        push this
        call [address]
    }
}

bool SceneManager::SetNewTexture(int object, int texture)
{
    SiffLoadSet* set;
    if (gSlMod->OverrideTexture(texture, set))
        return SetNewTexture(object, texture, set);

    CTextureObject* obj = gObjectManager->GetTextureObject(object);
    if (obj == nullptr) return false;
    return obj->SetNewTexture(texture);
}

bool SceneManager::SetNewTexture(int object, int texture, int scene)
{
    SiffLoadSet* set;
    if (gSlMod->OverrideTexture(texture, set))
        return SetNewTexture(object, texture, set);
    
    CTextureObject* obj = gObjectManager->GetTextureObject(object);
    CSceneEntry* entry = m_sceneTree.Find(scene);
    if (obj == nullptr || entry == nullptr) return false;
    return obj->SetNewTexture(texture, entry->m_pLoadSet);
}

bool SceneManager::SetNewTexture(int object, int texture, SiffLoadSet const* siff)
{
    CTextureObject* obj = gObjectManager->GetTextureObject(object);
    if (obj == nullptr) return false;
    return obj->SetNewTexture(texture, siff);
}

void SceneManager::HideObject(const char* format, ...)
{
    va_list args;
    char buf[256];
    va_start(args, format);
    vsnprintf(buf, 256, format, args);
    va_end(args);

    HideObjectInternal(Hash(buf), true);
}

void SceneManager::ShowObject(const char* format, ...)
{
    va_list args;
    char buf[256];
    va_start(args, format);
    vsnprintf(buf, 256, format, args);
    va_end(args);

    ShowObjectInternal(Hash(buf), true);
}

void SceneManager::SetNewTexture(int texture, const char* object, ...)
{
    va_list args;
    char buf[256];
    va_start(args, object);
    vsnprintf(buf, 256, object, args);
    va_end(args);
    SetNewTexture(Hash(buf), texture);
}

void SceneManager::SetNewTexture(const char* texture, const char* object, ...)
{
    va_list args;
    char buf[256];
    va_start(args, object);
    vsnprintf(buf, 256, object, args);
    va_end(args);

    SetNewTexture(Hash(buf), Hash(texture));
}

void SceneManager::SetNewTexture(const char* texture, int hash)
{
    SetNewTexture(hash, Hash(texture));
}

void SceneManager::SetNewTexture(const SlStringT<char>& resource, int texture, int object)
{
    SiffLoadSet** siff = (SiffLoadSet**)gResourceManager->GetResource(resource);
    if (siff == nullptr || *siff == nullptr) return;
    SetNewTexture(object, texture, *siff);
}

void SceneManager::SetNewTexture(const SlStringT<char>& resource, int texture, const char* object, ...)
{
    SiffLoadSet** siff = (SiffLoadSet**)gResourceManager->GetResource(resource);
    if (siff == nullptr || *siff == nullptr) return;

    va_list args;
    char buf[256];
    va_start(args, object);
    vsnprintf(buf, 256, object, args);
    va_end(args);

    SetNewTexture(Hash(buf), texture, *siff);
}