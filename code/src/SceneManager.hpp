#pragma once

#include "Core.hpp"

class SiffLoadSet;

// sizeof: 0x64
// SmoToolRenderList @ 0x10
// they probably didnt change this at all since the first game,
// looks like all they did was add a single value?

class CTextureObject {
public:
    DEFINE_MEMBER_FN_2(SetNewTexture, bool, 0x00495f00, int hash);
private:
    char Pad[0xec];
public:
    SiffLoadSet* m_pLoadSet;
};

class ObjectManager {
public:
    DEFINE_MEMBER_FN_1(GetTextureObject, CTextureObject*, 0x004972a0, int hash);
};

class SceneManager {
public:
    DEFINE_MEMBER_FN_2(SetNewTexture, void, 0x00493e40, int object, int texture);
    DEFINE_MEMBER_FN_3(SetNewTextureWithScene, void, 0x00493e70, int object, int texture, int scene);
private:
    DEFINE_MEMBER_FN_2(HideObjectInternal, void, 0x00493b60, int object, bool b);
    DEFINE_MEMBER_FN_2(ShowObjectInternal, void, 0x00493b10, int object, bool b);
public:
    void SetRacerIconTexture(int object, int buf, int scene);
public:
    void SetNewTexture(int texture, const char* object, ...);
    void SetNewTexture(const char* texture, int hash);
    void SetNewTexture(const char* texture, const char* object, ...);
    void SetNewTexture(const SlStringT<char>& resource, int texture, const char* object, ...);
    void SetNewTexture(const SlStringT<char>& resource, int texture, int object);
    void HideObject(const char* format, ...);
    void ShowObject(const char* format, ...);

    inline void HideObject(int hash) { HideObjectInternal(hash, true); }
    inline void ShowObject(int hash) { ShowObjectInternal(hash, true); }

};

/// Global scene manager for UI, gets linked to game's pointer at startup.
inline SlReloc<SceneManager> gSceneManager(0xEC1EC8);
inline SlReloc<ObjectManager> gObjectManager(0x00ec1f2c);