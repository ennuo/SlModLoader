#pragma once

#include "Core.hpp"

template <typename TKey, typename TValue, typename THook = TValue::TreeHook, bool T = false>
class SumoAVLTree {
public:
    TValue* Find(TKey const& key) const
    {
        TValue* it = m_pHead;
        while (it != nullptr)
        {
            const THook* h = (const THook*)it;
            const TKey& hk = h->GetKey();

            if (hk == key) return it;
            it = (key <= hk) ? h->GetLeft() : h->GetRight();
        }

        return nullptr;
    }
private:
    TValue* m_pHead;
};

template <typename T>
class SumoAVLTree_TreeHook {
public:
    inline int GetDepth() const { return m_depth; }
    inline T* GetLeft() const { return m_pLeft; }
    inline T* GetRight() const { return m_pRight; }
    inline T* GetParent() const { return m_pParent; }
private:
    int m_depth;
    T* m_pLeft;
    T* m_pRight;
    T* m_pParent;
};

class SiffLoadSet;
class CSceneEntry {
public:
class TreeHook : public SumoAVLTree_TreeHook<CSceneEntry> {
public:
    inline const unsigned int& GetKey() const { return ((CSceneEntry*)this)->m_SceneHash; }
};
public:
    TreeHook m_SumoAVLTreeHook_TreeHook;
    unsigned char* m_prgSceneBuffer;
    unsigned char* m_pCurrentPosition;
    unsigned int m_iSceneBufferSize;
    void* m_pSceneData;
    unsigned int m_SceneHash;
private:
    char mPad[16];
public:
    SiffLoadSet* m_pLoadSet;
    /// so on, but doesnt matter
};

// sizeof: 0x64
// SmoToolRenderList @ 0x10
// they probably didnt change this at all since the first game,
// looks like all they did was add a single value?

class CTextureObject {
public:
    DEFINE_MEMBER_FN_1(SetNewTexture, bool, 0x00495f00, int hash);
    bool SetNewTexture(int hash, SiffLoadSet const* siff);
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
    inline bool __hook_SetNewTexture(int object, int texture) { return SetNewTexture(object, texture); }
    inline bool __hook_scene_SetNewTexture(int object, int texture, int scene) { return SetNewTexture(object, texture, scene); }
public:
    bool SetNewTexture(int object, int texture);
    bool SetNewTexture(int object, int texture, int scene);
    bool SetNewTexture(int object, int texture, SiffLoadSet const* siff);

    // DEFINE_MEMBER_FN_2(SetNewTexture, void, 0x00493e40, int object, int texture);
    // DEFINE_MEMBER_FN_3(SetNewTextureWithScene, void, 0x00493e70, int object, int texture, int scene);
private:
    DEFINE_MEMBER_FN_2(HideObjectInternal, void, 0x00493b60, int object, bool b);
    DEFINE_MEMBER_FN_2(ShowObjectInternal, void, 0x00493b10, int object, bool b);
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
private:
    SumoAVLTree<unsigned int, CSceneEntry> m_sceneTree;
};

/// Global scene manager for UI, gets linked to game's pointer at startup.
inline SlReloc<SceneManager> gSceneManager(0xEC1EC8);
inline SlReloc<ObjectManager> gObjectManager(0x00ec1f2c);