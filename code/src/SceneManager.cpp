#include "SceneManager.hpp"
#include "ResourceManager.hpp"
#include "Jenkins.hpp"
#include "Game.hpp"
#include "ModLoader.hpp"

#include <cstdarg>

#include <Windows.h>

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

    CTextureObject* obj = gObjectManager->GetTextureObject(object);
    if (obj == nullptr) return;

    // dumb hack to get around the fact im lazy
    // temporarily set the siff load set of the object
    // to our own.
    SiffLoadSet* b = obj->m_pLoadSet;
    obj->m_pLoadSet = *siff;
    SetNewTexture(object, texture);
    obj->m_pLoadSet = b;
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

    int hash = Hash(buf);

    CTextureObject* obj = gObjectManager->GetTextureObject(hash);
    if (obj == nullptr) return;

    // dumb hack to get around the fact im lazy
    // temporarily set the siff load set of the object
    // to our own.
    SiffLoadSet* b = obj->m_pLoadSet;
    obj->m_pLoadSet = *siff;
    SetNewTexture(hash, texture);
    obj->m_pLoadSet = b;
}

void SceneManager::SetRacerIconTexture(int object, int buf, int scene)
{
    // stupid hack
    const int kHash_RandomIcon = Hash("Random_IconSmall.tga");
    if (buf == kHash_RandomIcon)
    {
        SetNewTextureWithScene(object, kHash_RandomIcon, scene);
        return;
    }

    RacerInfo* info = (RacerInfo*)buf;
    if (info->IsMod())
    {
        SlCustomRacer* racer = info->GetCustomRacer();
        SetNewTexture(racer->FrontendResources, info->CharSelectIconHash, object);
    }
    else SetNewTexture(object, info->CharSelectIconHash);
}