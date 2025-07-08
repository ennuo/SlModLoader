#pragma once

#include <vector>

#include "Siff.hpp"
#include "ResourceManager.hpp"

class SiffObjectVec {
public:
    SiffObjectVec()
    {
        memset(this, 0, sizeof(SiffObjectVec));
    }
public:
    Siff::Object::Header* mNext;
    Siff::Object::Header* mPrev;
    int mNumObjects;
    unsigned int* mHashes;

    // because the first member of an std::vector is a pointer
    // to the first element, we can get away with just using this type here.
    std::vector<Siff::Object::TableEntry> mObjects;
};

class SlModUI : public SiffLoadSet {
public:
    inline SlModUI() : SiffLoadSet(), m_ObjectRoot() {}
private:
    SiffObjectVec m_ObjectRoot;
};

class SlEnsureVTableAtZeroOffset {
public:
    virtual ~SlEnsureVTableAtZeroOffset() {}
};

class ITouchMenu {
public:
    virtual ~ITouchMenu() {}
    virtual void TouchMenu_AreaPressed(unsigned int) {}
    virtual void TouchMenu_AreaReleased(unsigned int) {}
    virtual void TouchMenu_AreaReleasedOutside(unsigned int) {}
    virtual int TouchMenu_AreaRollOver(unsigned int) { return 0; }
    virtual int TouchMenu_AreaRollOff(unsigned int) { return 0; }
    virtual void TouchMenu_SelectionChanged(unsigned int) {}
    virtual void TouchMenu_ValueChanged(unsigned int, unsigned int) {}
    virtual void TouchMenu_Progress() {}
};

class cFrontendScene : ITouchMenu, SlEnsureVTableAtZeroOffset {
public:
    inline cFrontendScene() : mState(0), mPreUpdateState(0), mUpdateState(0), mPostUpdateState(0), mDummy2(0) {}
public:
    virtual ~cFrontendScene() {}
    virtual void PreUpdate() {}
    virtual void Update() {}
    virtual void PostUpdate() {}
private:
    virtual void Dummy8() {}
public:
    virtual void Finish() {}
private:
    virtual void Dummy9() {}
    virtual void Dummy10() {}
public:
    void NextState();
public:
    int mState;
    int mPreUpdateState;
    int mUpdateState;
    int mPostUpdateState;
private:
    bool mDummy2;
};

// sizeof = 0x18

// 0x0 = cFrontEndScene vtable
// 0x4 = ITouchMenu vtable
// 0x8 = int State (whether scene is exiting, starting, updating, etc)
// 0xc = int
// 0x10 = int Stage (custom tracker for scene state)
// 0x14 = int
// 0x18 = byte


class cStartup : public cFrontendScene {
public:
    cStartup();
    void PreUpdate();
    void Update();
    void PostUpdate();
    DEFINE_MEMBER_FN_0(Finish, void, 0x00756530);
private:
    float mTime;
};

class c3dManager {
public:
    DEFINE_MEMBER_FN_1(LoadFrontendAssets, void, 0x00741b80, ResourceList* list);
};

class cFrontEndManager {
public:
    inline static SlReloc<SlStringT<char>> IslandsSoundBank{0x00bc7bd8};
    inline static SlReloc<int> m_MasterPlayerDevice{0x00bc7b68};
    inline static SlReloc<cFrontEndManager*> ms_pSE_RTTI_SINGLETON{0x00e9a410};
public:
    static void StartLoadingFESoundBank();
public:
    DEFINE_STATIC_MEMBER_FN_1(SetMasterPlayer, void, 0x0068ec20, int player);
    DEFINE_MEMBER_FN_1(SetMenuKeyboardMasterPlayer, void, 0x007542a0, int player);
    DEFINE_MEMBER_FN_3(AddScene, cFrontendScene*, 0x00695690, const char* name, cFrontendScene* scene, int x);
    DEFINE_MEMBER_FN_1(GetControllerState, int, 0x0068cf30, int);
private:
    char Pad[0x1e8c];
public:
    c3dManager* fe3d;
private:
    char Pad1[0x574];
public:
    int FocusChannel;
};