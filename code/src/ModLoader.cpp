#define STB_RECT_PACK_IMPLEMENTATION

#include <MinHook.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <DirectXTex.h>
#include <stb_rect_pack.h>

#include <filesystem>
#include <cstdio>

#include "Game.hpp"
#include "Jenkins.hpp"
#include "ModLoader.hpp"
#include "Frontend.hpp"
#include "Networking.hpp"

#include <windows.h>
#include <toml.hpp>

#include <iostream>

template <typename T>
class SePtr {
private:
    T* mData0;
    T* mData1;
};

class SeInstanceEntityNode {};
class SeInstanceAnimationStreamNode {};
class SeNodeBase {};

const int kHash_Random = Hash("Random");
const int kHash_Locked = Hash("Locked");

// cCharacterLoader
    // 0x0 - int State
    // 0x4 - SePtr<SeInstanceEntityNode> Character
    // 0xc - SePtr<SeInstanceEntityNode> Base
    // 0x14 - SePtr<SeInstanceAnimationStreamNode> CarBaseIdle01
    // 0x1c - SePtr<SeInstanceAnimationStreamNode> CarBaseIntoIdle01
    // 0x24 - SePtr<SeInstanceAnimationStreamNode> CarBaseOutOfIdle01
    // 0x2c - SlStringT<char> CharacterResourcePath
    // 0x4c - SePtr<>
    // 0x54 - ResourceList CharacterResources
    // 0x58 - ResourceList BaseResources
    // 0x5c - int NameHash


    // se_anim_stream_${CharacterName}car|driveidle
    // ${CharacterMayaFile}.mb:se_entity_${CharacterEntity}.model
    // FeCharacters\\${CharacterName}_fe\\${CharacterName}_fe

uint8_t g_MaxRacers = 255;
void PatchWeaponSetupManager()
{
    uint32_t new_hit_setup_base = 0x6040 + (g_MaxRacers * 0xb0);
    uint32_t cls_size = new_hit_setup_base + 0xd0;

    // Patch the size of the weapon setup manager's allocation to account for the
    // new space we're going to be adding to the fixed array.
    PatchExecutableSection((void*)(gMemoryBase + 0x2CC5B6), &cls_size, sizeof(uint32_t));

    // Patch the vector initializer/destructor functions for the racer all star data
    PatchExecutableSection((void*)(gMemoryBase + 0x2CDBDE), &g_MaxRacers, sizeof(uint8_t));
    PatchExecutableSection((void*)(gMemoryBase + 0x3F3198), &g_MaxRacers, sizeof(uint8_t));

    // Patch the offsets of the weapon hit setup
    PatchExecutableSection((void*)(gMemoryBase + 0x3F31B1), &new_hit_setup_base, sizeof(uint32_t));
    PatchExecutableSection((void*)(gMemoryBase + 0x2C2CC5), &new_hit_setup_base, sizeof(uint32_t));
    PatchExecutableSection((void*)(gMemoryBase + 0x3F2E34), &new_hit_setup_base, sizeof(uint32_t));

    // Patch grid state
    uint8_t num_states = 54;
    PatchExecutableSection((void*)(gMemoryBase + 0x44FDB0), &num_states, sizeof(uint8_t));

    uint8_t analog_state_index = 0;
    PatchExecutableSection((void*)(gMemoryBase - kExecutableBase + 0x88d157), &analog_state_index, sizeof(uint8_t));
    PatchExecutableSection((void*)(gMemoryBase - kExecutableBase + 0x88d198), &analog_state_index, sizeof(uint8_t));
    PatchExecutableSection((void*)(gMemoryBase - kExecutableBase + 0x88d1df), &analog_state_index, sizeof(uint8_t));
    PatchExecutableSection((void*)(gMemoryBase - kExecutableBase + 0x88d1f8), &analog_state_index, sizeof(uint8_t));
    PatchExecutableSection((void*)(gMemoryBase - kExecutableBase + 0x88d286), &analog_state_index, sizeof(uint8_t));

    PatchExecutableSection((void*)(gMemoryBase - kExecutableBase + 0x88d286), &analog_state_index, sizeof(uint8_t));


    // uint8_t shellcode[2] = { 0xeb, 0x0f };
    // uint8_t nop[2] = { 0x66, 0x90 };
    // PatchExecutableSection((void*)0x42b837, &shellcode, sizeof(uint8_t) * 2);
    // PatchExecutableSection((void*)0x42b86d, &nop, sizeof(uint8_t) * 2);
}

class SoftResetManager {
public:
    inline static SlReloc<SoftResetManager*> ms_pSE_RTTI_SINGLETON{0x00e9a3f0};
public:
    DEFINE_MEMBER_FN_0(DisablePolling, void, 0x0069d500);
};


class GameDatabase {
public:
    inline static int m_uNumBaseRacers;
    inline static int m_uNumCustomRacers;

    inline static SlReloc<unsigned int> m_uNumRacers{0x00bd0318};
    inline static SlReloc<RacerInfo*> m_pRacerInfo{0x00bd0268};
    inline static RacerInfoEx* m_pRacerInfoEx;
public:
    inline static void (*GameDatabase_SetupRacerData)();

    static RacerInfo* GetRacer(int hash)
    {
        for (int i = 0; i < m_uNumRacers; ++i)
        {
            RacerInfo* racer = m_pRacerInfo + i;
            if (racer->NameHash == hash)
                return racer;
        }

        return nullptr;
    }

    static RacerInfo* GetRacerAppearanceFromNetworkID(RacerNetworkId id)
    {
        SumoNet::NetMatch* match = SumoNet::gNetManager->GetMatch();
        if (match == nullptr || match->IsError()) return nullptr;
        
        const SumoNet::NetId& pid = id.GetPeerId();
        int pnum = pid.GetUserNum();

        SumoNet::NetMatchPeer* peer = match->GetPeer(pid);
        if (peer == nullptr || peer->GetNumPlayers() < pnum) return nullptr;

        auto player = peer->GetEx().GetPlayer(pnum);
        if (player.mInitialised)
            return GetRacer(player.mNameHash);
        
        return nullptr;
    }

    static RacerInfo* GetRacerFromNetworkID(RacerNetworkId id)
    {
        // LOG("GetRacerFromNetworkID: char=%02x, peer=%04x, user_num=%d", id.GetCharacterId(), (unsigned short)id.GetPeerId(), id.GetPeerId().GetUserNum());

        if (id.IsRandom()) return nullptr;

        if (id.IsHuman())
        {
            RacerInfo* appearance = GetRacerAppearanceFromNetworkID(id);
            if (appearance != nullptr)
                return appearance;
        }

        for (int i = 0; i < m_uNumRacers; ++i)
        {
            RacerInfo* racer = m_pRacerInfo + i;
            if (racer->Ex->StatId == id.GetCharacterId())
                return racer;
        }

        return nullptr;
    }

    void SetupRacerData()
    {
        GameDatabase_SetupRacerData();


        m_uNumBaseRacers = m_uNumRacers;
        m_uNumCustomRacers = gSlMod->Racers.size();

        int size = m_uNumBaseRacers * sizeof(RacerInfo) + 4;

        int* old = ((int*)(RacerInfo*)m_pRacerInfo) - 1;
        int* data = (int*)gSlMallocAlign(size + (m_uNumCustomRacers * sizeof(RacerInfo)), 4, true);
        memcpy(data, old, size);
        delete[] old;

        m_pRacerInfo = (RacerInfo*)(data + 1);
        RacerInfo* custom = (RacerInfo*)(data + 1) + m_uNumBaseRacers;
        m_uNumRacers = m_uNumBaseRacers + m_uNumCustomRacers;
        *data = m_uNumRacers;

        m_pRacerInfoEx = new RacerInfoEx[m_uNumRacers];
        memset(m_pRacerInfoEx, 0, sizeof(RacerInfoEx) * m_uNumRacers);

        for (int i = 0; i < m_uNumRacers; ++i)
        {
            m_pRacerInfo[i].Ex = m_pRacerInfoEx + i;
            m_pRacerInfo[i].Ex->Info = m_pRacerInfo + i;
        }

        int custom_racer_index = 0;
        for (const auto& racer : gSlMod->Racers)
        {
            RacerInfo* base_racer = nullptr;
            for (int i = 0; i < m_uNumBaseRacers; ++i)
            {
                if (m_pRacerInfo[i].NameHash == racer->BaseHash)
                {
                    base_racer = &m_pRacerInfo[i];
                    break;
                }
            }

            if (base_racer == nullptr)
            {
                LOG("Could not find base racer %s (%08x) for %s (%08x)", racer->BaseId.m_Data, racer->BaseHash, racer->InternalId.m_Data, racer->NameHash);
                continue;
            }

            LOG("Adding %s (%s) to grid (base=%s)", racer->InternalId.m_Data, racer->DisplayName.m_Data, racer->BaseId.m_Data);

            memcpy(custom, base_racer, sizeof(RacerInfo));

            custom->GridOrder = 512 + custom_racer_index;

            custom->Ex = m_pRacerInfoEx + m_uNumBaseRacers + custom_racer_index;
            custom->Ex->Mod = gSlMod->Racers[custom_racer_index++];
            custom->NameHash = racer->NameHash;
            custom->DisplayName = racer->DisplayName;
            custom->CharacterName = racer->InternalId;
            if (!racer->IsModelSwap)
            {
                custom->CharacterMayaFile = racer->InternalId;
                custom->CharacterEntity = racer->InternalId;
            }
            custom->CharSelectIconHash = racer->SelectIconHash;
            custom->CharSelectBigIconHash = racer->VersusPortraitHash;
            custom->Unlocked = true;
            custom->InitiallyUnlocked = true;
            // custom->Parameter = racer->InternalId;
            custom->AvailableInFrontEnd = true;
            custom->AvailableOnPc = true;

            custom->CharacterFlags |= 1; // DefaultCharacter
            custom->CharacterFlags |= 2; // DefaultAICharacter

            if (!racer->IsModelSwap)
            {
                custom->CarAnimationFilePrefix = racer->CarAnimationFilePrefix;
                custom->BoatAnimationFilePrefix = racer->BoatAnimationFilePrefix;
                custom->PlaneAnimationFilePrefix = racer->PlaneAnimationFilePrefix;
                custom->TransformAnimationFilePrefix = racer->TransformAnimationFilePrefix;
            }

            custom->MiniMapIcon_Car = racer->MinimapIconHash;
            custom->MiniMapIcon_Boat = racer->MinimapIconHash;
            custom->MiniMapIcon_Plane = racer->MinimapIconHash;

            custom->ImageUnlockHash = racer->RaceResultsHash;
            
            custom++;
        }

        for (int i = 0; i < m_uNumRacers; ++i)
        {
            RacerInfo& info = m_pRacerInfo[i];
            info.Ex->StatId = info.NetworkId;
            memcpy(&info.Ex->BaseStats, &info.DefaultMod, sizeof(CachedRacerStats));
        }
    }
};

void RacerInfo::RestoreStats()
{
    NetworkId = Ex->StatId;
    memcpy(&DefaultMod, &Ex->BaseStats, sizeof(CachedRacerStats));
}

void RacerInfo::CopyStats(int id)
{
    RacerInfo* racer = GameDatabase::GetRacer(id);
    if (racer != nullptr)
    {
        // NetworkId = racer->Ex->StatId;
        memcpy(&DefaultMod, &racer->Ex->BaseStats, sizeof(CachedRacerStats));
    }
}

class cCharacterSelectMulti;
void(__thiscall *cCharacterSelectMulti_SetUpGrid)(void*);
void (__thiscall *cCharacterSelectMulti_Update)(cCharacterSelectMulti*);
void (__thiscall *cCharacterSelectMulti_PreUpdate)(cCharacterSelectMulti*);
void (__thiscall *cCharacterSelectMulti_TouchMenu_AreaPressed)(cCharacterSelectMulti*, int);

bool DoesSumoResourceExist(const SlStringT<char>& path)
{
    SlStringT<char> filename = path;
    filename += ".cpu.spc";
    return gResourceManager->FileExists(filename);
}

__declspec(naked) int MakeCharacterFilename(SlStringT<char> path, RacerInfo* info, SlStringT<char>& filename) noexcept
{
    INLINE_ASM_PROLOGUE
    READ_ECX(filename);

    int status;
    if (info->IsMod())
    {
        filename = info->GetCustomRacer()->CharacterFile;
    }
    else
    {
        filename = path;
        filename += info->CharacterName;
        filename += "\\";
        filename += info->CharacterName;
    }

    LOG("- %s", filename.m_Data);
    {
        SlStringT<char> resource = filename;
        resource += ".cpu.spc";
        status = gResourceManager->FileExists(resource);
    }

    WRITE_EAX(status);
    INLINE_ASM_EPILOGUE
}

class cCharacterSelectMulti : public cFrontendScene {
public:
    inline static std::vector<RacerInfo*> m_Racers;
    inline static SlReloc<std::vector<RacerInfo*>> m_Page{0x00c51c08};
    inline static SlReloc<int> m_PageSize{0x00c51bc4};
    inline static SlReloc<uintptr_t*> __vtbl{0x00a2eec4};
    inline static SlReloc<int> m_uiSelectedChar{0x00c51ba4};
    inline static SlReloc<bool> m_uiReloadChar{0x00c51bb8};
    inline static int m_PageIndex;
    static const int kMaxPageSize = 30;
public:
    DEFINE_MEMBER_FN_1(TriggerFlash, void, 0x00848400, int player);
    DEFINE_MEMBER_FN_4(SetHighlighters, void, 0x0084e0c0, int a, bool b, bool c, bool d);
public:
    void TouchMenu_AreaPressedImpl(int e)
    {
        cCharacterSelectMulti_TouchMenu_AreaPressed(this, e);
    }

    void PreUpdateImpl()
    {
        // When this class is constructed, the page variable is the original
        // racers array, but it's more convenient to use that as a page variable
        if (m_Racers.size() == 0) 
            m_Racers = m_Page;

        for (int i = 0; i < m_Racers.size(); ++i)
        {
            m_Racers[i]->RestoreStats();
            m_Racers[i]->Ex->CachedStatGridIndex = i;
        }

        cCharacterSelectMulti_PreUpdate(this);
    }

    void UpdateImpl()
    {
        for (int i = 0; i < (*SlInput::m_Gamepad).size(); ++i)
        {
            int state = cFrontEndManager::ms_pSE_RTTI_SINGLETON->GetControllerState(i);

            int old_page = m_PageIndex;

            if (i == 0)
            {
                SlKeyboard* keyboard = (*SlInput::m_Keyboard)[0];
                if (keyboard->GetKeyPressed(kFocusChannel_Frontend, KEY_Q)) state |= 0x1000;
                if (keyboard->GetKeyPressed(kFocusChannel_Frontend, KEY_E)) state |= 0x2000;
                if (keyboard->GetKeyPressed(kFocusChannel_Frontend, KEY_V)) state |= 0x8000;
            }


            if (state & 0x1000 || state & 0x400) m_PageIndex--;
            if (state & 0x2000 || state & 0x800) m_PageIndex++;

            if (m_PageIndex < 0) m_PageIndex = 0;
            if (m_PageIndex * kMaxPageSize >= m_Racers.size())
                m_PageIndex--;

            if (old_page != m_PageIndex)
            {
                SetUpGrid();

                int& ch = (&m_uiSelectedChar)[cFrontEndManager::m_MasterPlayerDevice];
                if (ch >= m_Page->size()) ch = m_Page->size() - 1;
                (&m_uiReloadChar)[cFrontEndManager::m_MasterPlayerDevice] = true;
                SetHighlighters(cFrontEndManager::m_MasterPlayerDevice, false, true, false);
                TriggerFlash(cFrontEndManager::m_MasterPlayerDevice);
            }

            if (state & 0x8000)
            {
                int& ch = (&m_uiSelectedChar)[cFrontEndManager::m_MasterPlayerDevice];

                RacerInfo* racer = m_Racers[(m_PageIndex * kMaxPageSize) + ch];

                racer->Ex->CachedStatGridIndex++;
                racer->Ex->CachedStatGridIndex %= m_Racers.size();

                racer->CopyStats(m_Racers[racer->Ex->CachedStatGridIndex]->NameHash);

                SetUpGrid();
                SetHighlighters(cFrontEndManager::m_MasterPlayerDevice, false, true, false);
                TriggerFlash(cFrontEndManager::m_MasterPlayerDevice);
            }
        }

        // frontend controller state
            // & 0x20 = CROSS
            // & 0x40 = CIRCLE
            // & 0x80 = SQUARE
            // & 0x100 = TRIANGLE
            // & 0x200 = START
            // & 0x400 = L2
            // & 0x800 = R2
            // & 0x1000 = L1
            // & 0x2000 = R1
            // & 0x4000 = SELECT

        cCharacterSelectMulti_Update(this);
    }

    void SetUpGrid()
    {
        m_Page->clear();

        int start = m_PageIndex * kMaxPageSize;

        m_PageSize = m_Racers.size() - start;
        if ((int)m_PageSize > kMaxPageSize) m_PageSize = kMaxPageSize;

        // fix issue with random slots on startup
        for (int i = 0; i < 4; ++i)
        {
            int& ch = (&m_uiSelectedChar)[i];
            if (ch >= m_Page->size()) ch = m_Page->size();
        }


        for (int i = start, j = 0; j < (int)m_PageSize; ++i, ++j)
        {
            RacerInfo* info = m_Racers[i];
            bool isMaxLevel = info->ModsBits == kModType_All;

            gSceneManager->ShowObject("CHARACTER_SELECT\\CHAR_%d_MOVE", j);
            gSceneManager->ShowObject("CHARACTER_SELECT\\CHAR_%d", j);
            

            const int kHash_Notification = HashF("CHARACTER_SELECT\\MAXED_ICON_%d", j);
            if (isMaxLevel)
            {
                gSceneManager->ShowObject(kHash_Notification);
                gSceneManager->SetNewTexture("Notification_Maxed.tga", kHash_Notification);

            }
            else
            {
                gSceneManager->HideObject(kHash_Notification);
            }

            if (info->Unlocked && info->New)
            {
                gSceneManager->SetNewTexture("Notification_ExclamationMark.tga", kHash_Notification);
                gSceneManager->ShowObject(kHash_Notification);
            }

            RacerInfo* donor = m_Racers[info->Ex->CachedStatGridIndex];
            if (donor != info)
            {
                gSceneManager->ShowObject(kHash_Notification);
                if (donor->IsMod())
                {
                    SlCustomRacer* racer = donor->GetCustomRacer();
                    gSceneManager->SetNewTexture(racer->FrontendResources, racer->SelectIconHash, "CHARACTER_SELECT\\MAXED_ICON_%d", j);
                }
                else 
                {
                    gSceneManager->SetNewTexture(kHash_Notification, donor->CharSelectIconHash);
                }
            }


            if (info->IsMod())
            {
                SlCustomRacer* racer = info->GetCustomRacer();
                gSceneManager->SetNewTexture(racer->FrontendResources, racer->SelectIconHash, "CHARACTER_SELECT\\CHAR_STATE_%d", j);
                gSceneManager->HideObject("CHARACTER_SELECT\\CHAR_GLOW_%d", j);
            }
            else
            {
                gSceneManager->SetNewTexture(info->CharSelectIconGlowHash, "CHARACTER_SELECT\\CHAR_GLOW_%d", j);
                gSceneManager->SetNewTexture(info->CharSelectIconHash, "CHARACTER_SELECT\\CHAR_STATE_%d", j);
            }
        


            m_Page->push_back(info);
        }

        for (int i = m_PageSize; i < 64; ++i)
        {
            gSceneManager->HideObject("CHARACTER_SELECT\\CHAR_%d_MOVE", i);
            gSceneManager->HideObject("CHARACTER_SELECT\\CHAR_%d", i);
        }

        gSceneManager->HideObject("CHARACTER_SELECT\\CHAR_TOP_2_COL_MOVE");
        gSceneManager->HideObject("CHARACTER_SELECT\\CHAR_TOP_3_COL_MOVE");
        gSceneManager->HideObject("CHARACTER_SELECT\\CHAR_BOTTOM_2_COL_MOVE");
        gSceneManager->HideObject("CHARACTER_SELECT\\CHAR_BOTTOM_3_COL_MOVE");
        gSceneManager->HideObject("CHARACTER_SELECT\\CHAR_BOTTOM_4_COL_MOVE");
        gSceneManager->HideObject("CHARACTER_SELECT\\CHAR_BOTTOM_5_COL_MOVE");
        gSceneManager->HideObject("CHARACTER_SELECT\\CHAR_EXTRA_4_COL_MOVE");
        gSceneManager->HideObject("CHARACTER_SELECT\\CHAR_EXTRA_5_COL_MOVE");


    }
};

std::vector<RacerInfo*> Hack_GetPlayerUsableUnlockedCharacters_UpdateStats(bool)
{
    return cCharacterSelectMulti::m_Page;
}

class cCharacterLoader {
public:
    inline static SlReloc<unsigned int> mLoadingCounter{0x00c56950};
public:
    void PerformLoad()
    {
        CharacterHash = RequestedCharacterHash;
        if (!CharacterResources.IsLoaded() && CharacterResources.GetSize() != 0) return;
        CharacterResources.UnloadResources();

        if (CharacterHash == kHash_Random)
            CharacterResourcePath = "FeCharacters\\QuestionMarkWhite\\QuestionMarkWhite"; 
        else if (CharacterHash == kHash_Locked)
            CharacterResourcePath = "FeCharacters\\QuestionMarkHolo\\QuestionMarkHolo";
        else
        {
            RacerInfo* racer = GameDatabase::GetRacer(CharacterHash);
            if (racer->IsMod())
            {
                SlCustomRacer* custom = racer->GetCustomRacer();

                LOG("%s, %s, %s", custom->FrontendCharacterFile.m_Data, custom->CharacterFile.m_Data, custom->InternalId.m_Data);

                if (DoesSumoResourceExist(custom->FrontendCharacterFile))
                    CharacterResourcePath = custom->FrontendCharacterFile;
                else if (DoesSumoResourceExist(custom->CharacterFile))
                    CharacterResourcePath = custom->CharacterFile;
                else
                    CharacterResourcePath = "FeCharacters\\QuestionMarkHolo\\QuestionMarkHolo"; 
            }
            else
            {
                char buf[512];
                sprintf(buf, "FeCharacters\\%s_fe\\%s_fe", racer->CharacterName, racer->CharacterName);
                CharacterResourcePath = buf;
            }
        }

        LOG(" - Loading frontend character: %s", CharacterResourcePath.m_Data);

        CharacterResources.AddResource(CharacterResourcePath, kResourceType_SumoEngine, false);

        if (mLoadingCounter == 0)
            SoftResetManager::ms_pSE_RTTI_SINGLETON->DisablePolling();
        mLoadingCounter++;

        CharacterResources.StartLoadResources();
    }
public:
    int State;
    SePtr<SeInstanceEntityNode> Character;
    SePtr<SeInstanceEntityNode> Base;
    SePtr<SeInstanceAnimationStreamNode> CarBaseIdle01;
    SePtr<SeInstanceAnimationStreamNode> CarBaseIntoIdle01;
    SePtr<SeInstanceAnimationStreamNode> CarBaseOutOfIdle01;
    SlStringT<char> CharacterResourcePath;
    SePtr<SeNodeBase> unk;
    ResourceList CharacterResources;
    ResourceList BaseResources;
    int CharacterHash;
    int RequestedCharacterHash;
};

namespace GameDataLoader
{
    static void (*LoadGameResources)();
    static void (*UnloadGameResources)();
}

void (__thiscall *c3dManager_LoadFrontendAssets)(c3dManager*, ResourceList*);
void c3dManager::LoadFrontendAssets(ResourceList* list)
{
    c3dManager_LoadFrontendAssets(this, list);
}

void __fastcall OnLoadFrontendAssets(c3dManager* manager, int, ResourceList* list)
{
    gSlMod->SetTextureSetEnabled(kTextureSet_CharacterSelect, true);
    manager->LoadFrontendAssets(list);
}

void OnLoadGameResources()
{
    gSlMod->SetTextureSetEnabled(kTextureSet_CharacterSelect, false);

    std::vector<SlStringT<char>> files;
    for (const auto& racer : gSlMod->Racers)
    {
        files.push_back(racer->GameResources);
        files.push_back(racer->FrontendResources);
    }

    gSlMod->MakeTextureSet(kTextureSet_Race, files);
    gSlMod->SetTextureSetEnabled(kTextureSet_Race, true);

    GameDataLoader::LoadGameResources();
}

void OnUnloadGameResources()
{
    gSlMod->DestroyTextureSet(kTextureSet_Race);
    GameDataLoader::UnloadGameResources();
}

// This function call seems to have been optimized, so the second argument gets passed into EDI, I don't think
// there's a standard calling convention for this? So we'll have to use a naked function definition.
__declspec(naked) Siff::Object::TableEntry* __fastcall OnGetObjectDef(SiffObjectDefManager* manager, unsigned int hash)
{
    INLINE_ASM_PROLOGUE
    READ_ECX(manager);
    READ_EDI(hash);
    {
        auto entry = manager->GetObjectDef(hash);
        WRITE_EAX(entry);
    }
    INLINE_ASM_EPILOGUE
}

void (__thiscall *VehicleModel_Create)(void*, void*, int, void*, void*, void*, RacerInfo*);
class VehicleModelProxy {
public:
    void Create(void* racer, int type, void* file, void* anim_state_info, void* anim_params, RacerInfo* info)
    {
        if (info->IsModelSwap())
            info->CharacterName = info->GetCustomRacer()->BaseId;
        VehicleModel_Create((void*)this, racer, type, file, anim_state_info, anim_params, info);
        if (info->IsModelSwap())
            info->CharacterName = info->GetCustomRacer()->InternalId;
    }
};

void __fastcall InitStartup(cStartup* startup)
{
    new (startup) cStartup();
}

void InitHooks()
{
    MH_Initialize();


    CREATE_MEMBER_HOOK(0x006d94f0, GameDatabase::GetRacerFromNetworkID, nullptr);
    CREATE_MEMBER_HOOK(0x007bad80, VehicleModelProxy::Create, &VehicleModel_Create);
    CREATE_MEMBER_HOOK(0x00850d50, cCharacterSelectMulti::TouchMenu_AreaPressedImpl, &cCharacterSelectMulti_TouchMenu_AreaPressed);
    CREATE_MEMBER_HOOK(0x0084f9e0, cCharacterSelectMulti::PreUpdateImpl, &cCharacterSelectMulti_PreUpdate);
    CREATE_MEMBER_HOOK(0x00851390, cCharacterSelectMulti::UpdateImpl, &cCharacterSelectMulti_Update);
    CREATE_MEMBER_HOOK(0x00848b90, cCharacterSelectMulti::SetUpGrid, &cCharacterSelectMulti_SetUpGrid);
    CREATE_MEMBER_HOOK(0x006dec10, GameDatabase::SetupRacerData, &GameDatabase::GameDatabase_SetupRacerData);
    CREATE_MEMBER_HOOK(0x008c7710, cCharacterLoader::PerformLoad, nullptr);
    // CREATE_HOOK(0x496700, OnGetObjectDef, nullptr);
    CREATE_HOOK(0x0079a3c0, MakeCharacterFilename, nullptr);

    CREATE_HOOK(0x0072d090, OnLoadGameResources, &GameDataLoader::LoadGameResources);
    CREATE_HOOK(0x00741b80, OnLoadFrontendAssets, &c3dManager_LoadFrontendAssets);

    CREATE_MEMBER_HOOK(0x00493e40, SceneManager::__hook_SetNewTexture, nullptr);
    CREATE_MEMBER_HOOK(0x00493e70, SceneManager::__hook_scene_SetNewTexture, nullptr);

    Network_InstallHooks();

    // CREATE_MEMBER_HOOK(0x007564c0, InitStartup, nullptr);

    MH_EnableHook(MH_ALL_HOOKS);
}

bool WantRebuildSumoToolCache(const std::filesystem::path& cachedir, const std::string& version)
{
    auto version_path = cachedir / "version";

    // We only want to rebuild the sumo tool cache if it either
    // doesn't exist, or if the version doesn't match the current mod version.
    if (std::filesystem::exists(version_path))
    {
        std::ifstream f(version_path, std::ios::in);
        std::ostringstream sstr;
        sstr << f.rdbuf();

        return sstr.str() != version;
    }

    return true;
}

class SlTextureEntry {
public:
    SlTextureEntry() : mHash(), mPath() {}
    SlTextureEntry(int hash, const std::filesystem::path& path) : mHash(hash), mPath(path) {}
    SlTextureEntry(const std::string& name, const std::filesystem::path& path)
    {
        char local[MAX_PATH];
        sprintf(local, "%s\\%s", name.c_str(), path.filename().string().c_str());

        mHash = Hash(local);
        mPath = path;
    }
public:
    int mHash;
    std::filesystem::path mPath;
};

void BuildSpriteAtlas(const std::vector<SlTextureEntry>& sprites, std::filesystem::path output)
{
    const int size = sprites.size();

    std::vector<stbrp_rect> rects(size);
    std::vector<DirectX::TexMetadata> infos(size);
    infos.resize(size);


    int w = 0, h = 0;
    int num_packed_sprites = 0;
    for (int i = 0; i < size; ++i)
    {
        auto& info = infos[i];
        HRESULT res = DirectX::GetMetadataFromWICFile(sprites[i].mPath.wstring().c_str(), DirectX::WIC_FLAGS_NONE, info);

        stbrp_rect rect;
        rect.id = i;

        if (res != NOERROR)
        {
            LOG(" - Failed to get metadata for %s", sprites[i].mPath.string().c_str());

            rect.w = 0;
            rect.h = 0;
        }
        else
        {
            LOG(" - %s (%dx%d)", sprites[i].mPath.string().c_str(), info.width, info.height);

            rect.w = info.width;
            rect.h = info.height;

            if (rect.h > h) h = rect.h;
            if (rect.w > w) w = rect.w;

            num_packed_sprites++;
        }

        rects.push_back(rect);
    }

    stbrp_context context;
    stbrp_node* nodes = new stbrp_node[2048];

    // Start with the smallest power of 2
    h = 1 << (32 - _lzcnt_u32(h - 1));
    w = 1 << (32 - _lzcnt_u32(w - 1));

    int it = 0;
    while (w < 2048 || h < 2048)
    {
        stbrp_init_target(&context, w, h, nodes, w);
        if (stbrp_pack_rects(&context, rects.data(), rects.size()))
            break;

        if (it % 2) h <<= 1;
        else w <<= 1;

        it++;
    }

    delete[] nodes;

    DirectX::ScratchImage atlas;
    atlas.Initialize2D(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, w, h, 1, 1);
    auto atlas_image = atlas.GetImage(0, 0, 0);

    for (const auto& rect : rects)
    {
        const auto& sprite = sprites[rect.id];



        DirectX::ScratchImage scratch_image;
        HRESULT res = DirectX::LoadFromWICFile(sprite.mPath.wstring().c_str(), DirectX::WIC_FLAGS_NONE, nullptr, scratch_image);
        if (res != NOERROR) continue;

        auto image = scratch_image.GetImage(0, 0, 0);
        for (int sy = 0; sy < rect.h; ++sy)
        {
            uint8_t* src = image->pixels + (sy * image->rowPitch);
            uint8_t* dest = atlas_image->pixels + ((sy + rect.y) * atlas_image->rowPitch) + (rect.x * 4);
            memcpy(dest, src, image->rowPitch);
        }
    }

    DirectX::ScratchImage atlas_bc2;
    DirectX::Compress(*atlas_image, DXGI_FORMAT_BC2_UNORM, DirectX::TEX_COMPRESS_SRGB, 0.5f, atlas_bc2);

    DirectX::Blob atlas_blob;
    DirectX::SaveToDDSMemory(*atlas_bc2.GetImage(0, 0, 0), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, atlas_blob);

    atlas_bc2.Release();

    std::ofstream dat(output.string() + ".dat", std::ios::binary | std::ios::trunc);
    std::ofstream rel(output.string() + ".rel", std::ios::binary | std::ios::trunc);

#define WRITE_DUMMY_HEADER(f) { \
    ChunkHeader header; \
    f.write((const char*)&header, sizeof(ChunkHeader)); \
}
#define WRITE_HEADER(f, m, s) { \
    ChunkHeader header; \
    header.id = m; \
    header.chunk_size = s + sizeof(ChunkHeader); \
    header.data_size = s; \
    header.endian_indicator = kSiff_EndianMarker; \
    f.seekp(0); \
    f.write((const char*)&header, sizeof(ChunkHeader)); \
}
#define WRITE_MAGIC(f, m) { \
    u32 magic = m; \
    f.write((const char*)&magic, sizeof(u32)); \
}
#define WRITE_RELOCATION(f, o) { \
    f = ((uintptr_t)o) - (uintptr_t)buf; \
    Siff::Relocation r; \
    r.Flags = 1; \
    r.Offset = ((uintptr_t)&f - (uintptr_t)buf); \
    rel.write((const char*)&r, sizeof(Siff::Relocation)); \
}

#define FINISH_RELOCATIONS() { \
    Siff::Relocation r; \
    r.Flags = 0; r.Offset = 0; \
    rel.write((const char*)&r, sizeof(Siff::Relocation)); \
}

#define GET_ATLAS_POSITION(coord, x, y) \
    coord[0] = ((float)(x)) / (float)w; \
    coord[1] = ((float)(y)) / (float)h;



    byte* buf = new byte[sizeof(Siff::TexturePack) + num_packed_sprites * sizeof(Siff::Texture) + sizeof(Siff::Sheet)];

    WRITE_DUMMY_HEADER(rel);
    WRITE_MAGIC(rel, kSiffResource_TexturePack);



    Siff::TexturePack* pack = (Siff::TexturePack*)buf;
    Siff::Texture* texture = (Siff::Texture*)(buf + sizeof(Siff::TexturePack));
    Siff::Sheet* sheet = (Siff::Sheet*)(texture + num_packed_sprites);
    byte* eof = (byte*)(sheet + 1);
    size_t data_size = (size_t)(eof - buf);

    pack->NumSprites = num_packed_sprites;
    pack->NumSheets = 1;
    WRITE_RELOCATION(pack->SheetList, sheet);

    for (const auto& rect : rects)
    {
        // Sprites that weren't able to be loaded will just have zero dimensions,
        // so we can skip them.
        if (rect.w == 0 || rect.h == 0) continue;

        Siff::Texture& t = texture[rect.id];
        t.Hash = sprites[rect.id].mHash;
        GET_ATLAS_POSITION(t.TopLeft, rect.x, rect.y)
        GET_ATLAS_POSITION(t.TopRight, rect.x + rect.w, rect.y);
        GET_ATLAS_POSITION(t.BottomRight, rect.x + rect.w, rect.y + rect.h);
        GET_ATLAS_POSITION(t.BottomLeft, rect.x, rect.y + rect.h);
        t.Texture = 0;
    }

    WRITE_RELOCATION(sheet->Offset, eof);
    sheet->Size = atlas_blob.GetBufferSize();

    FINISH_RELOCATIONS();
    WRITE_HEADER(rel, kSiffResource_Relocations, sizeof(Siff::Relocation) * 3 + sizeof(u32));

    WRITE_HEADER(dat, kSiffResource_TexturePack, data_size + atlas_blob.GetBufferSize());
    dat.write((const char*)buf, data_size);
    dat.write((const char*)atlas_blob.GetConstBufferPointer(), atlas_blob.GetBufferSize());

#undef WRITE_HEADER
#undef WRITE_DUMMY_HEADER
#undef WRITE_MAGIC
#undef WRITE_RELOCATION
#undef FINISH_RELOCATIONS





    delete[] buf;
}

void BuildSumoToolCache(const std::string& name, const std::filesystem::path& metadir, const std::filesystem::path& cachedir, const std::string& version, eModType type)
{
    if (!std::filesystem::exists(cachedir))
        std::filesystem::create_directories(cachedir);

    if (!WantRebuildSumoToolCache(cachedir, version)) return;
    LOG(" - Rebuilding Sumo Tool Cache");

    switch (type)
    {
        case kModType_Racer:
        {
            std::vector<SlTextureEntry> frontend_resources =
            {
                SlTextureEntry(name, metadir / "icon.png"),
                SlTextureEntry(name, metadir / "versus.png")
            };

            std::vector<SlTextureEntry> race_resources =
            {
                SlTextureEntry(name, metadir / "minimap.png"),
                SlTextureEntry(name, metadir / "results.png")
            };

            BuildSpriteAtlas(race_resources, cachedir / "race");
            BuildSpriteAtlas(frontend_resources, cachedir / "frontend");

            break;
        }
    }

    std::ofstream version_file(cachedir / "version");
    version_file << version;
    version_file.close();
}

void InstallMod(const std::filesystem::path& moddir)
{
    auto config_path = moddir / "config.toml";
    auto meta_path = moddir / "meta";
    auto sound_path = moddir / "sounds";

    toml::table config;
    try { config = toml::parse_file(config_path.string()); }
    catch (std::exception& ex)
    {
        LOG(" - Failed to load \"%s\": %s", config_path.string().c_str(), ex.what());
        return;
    }

    if (!config["enabled"].value_or(true)) return;

    std::string version = config["version"].value_or("1.0.0");

    std::string dirname = moddir.filename().string();
    std::string name = config["name"].value_or(dirname.c_str());
    LOG("%s", name.c_str());

    auto cache_path = std::filesystem::relative("data/modcache") / name; 


    if (toml::array* arr = config["dll"].as_array())
    for (auto& elem : *arr)
    {
        std::string dll = elem.value_or("");
        if (dll.empty()) continue;
        dll = moddir.string() + "\\" + dll;

        HMODULE handle = LoadLibraryA(dll.c_str());
        if (handle)
        {
            _SlMod_Load load = (_SlMod_Load)GetProcAddress(handle, "SlMod_Load");
            if (load)
            {
                load(&gSlModApi);
                LOG(" - Loaded mod!");
            }
            else
            {
                LOG(" - Does not appear to be a valid mod DLL!");
            }
        }
        else
        {
            LOG(" - Failed to load mod DLL!");
        }

    }



    std::string file_type = config["type"].value_or("file");
    if (file_type == "racer")
    {
        LOG(" - Loading Racer!");
        BuildSumoToolCache(name, meta_path, cache_path, version, kModType_Racer);

        SlCustomRacer* racer = new SlCustomRacer();
        racer->BaseId = config["racer"]["echo"].value_or("charmybee");
        racer->InternalId = config["racer"]["id"].value_or("charmybee");
        racer->DisplayName = config["racer"]["name"].value_or("Charmy Bee");
        racer->FrontendResources = SlStringT<char>(("modcache\\" + name + "\\frontend").c_str());
        racer->GameResources = SlStringT<char>(("modcache\\" + name + "\\race").c_str());

        racer->IsModelSwap = racer->BaseId.CompareCaseInsensitive("charmybee") != 0;

        racer->CharacterFile = SlStringT<char>(("mods\\" + dirname + "\\char\\" + racer->InternalId.m_Data).c_str());
        racer->FrontendCharacterFile = SlStringT<char>(("mods\\" + dirname + "\\char\\" + racer->InternalId.m_Data + "_fe").c_str());

        racer->CarAnimationFilePrefix = racer->InternalId + "_car_anims";
        racer->BoatAnimationFilePrefix = racer->InternalId + "_boat_anims";
        racer->PlaneAnimationFilePrefix = racer->InternalId + "_plane_anims";
        racer->TransformAnimationFilePrefix = racer->InternalId + "_transform_anims";

        racer->NameHash = Hash(racer->InternalId);
        racer->BaseHash = Hash(racer->BaseId);
        racer->SelectIconHash = Hash((name + "\\icon.png").c_str());
        racer->VersusPortraitHash = Hash((name + "\\versus.png").c_str());
        racer->MinimapIconHash = Hash((name + "\\minimap.png").c_str());
        racer->RaceResultsHash = Hash((name + "\\results.png").c_str());

        gSlMod->Racers.push_back(racer);
    }
}

SlCustomRacer* SlModManager::GetRacerByHash(int hash)
{
    for (const auto& racer : Racers)
    {
        if (racer->NameHash == hash)
            return racer;
    }

    return nullptr;
}

void SlModManager::LoadFrontendAssets(ResourceList* resources)
{
    for (const auto& racer : Racers)
    {
        LOG(" - Loading %s", racer->FrontendResources.m_Data);
        if (resources == nullptr)
            gResourceManager->LoadResource(racer->FrontendResources, kResourceType_SumoTool, false);
        else
            resources->AddResource(racer->FrontendResources, kResourceType_SumoTool, false);
    }
}

void SlModManager::SetupRacerTextureSets()
{
    std::vector<SlStringT<char>> files;
    for (const auto& racer : Racers)
        files.push_back(racer->FrontendResources);
    MakeTextureSet(kTextureSet_CharacterSelect, files);
}

void SlModManager::SetTextureSetEnabled(const char* name, bool enable)
{
    SlGlobalTextureSet* set = GetTextureSet(name);
    if (set == nullptr || set->IsEnabled() == enable) return;
    if (enable) set->Enable();
    else set->Disable();

    LOG("Texture Set %s has been %s", name, enable ? "enabled" : "disabled");

}

SlGlobalTextureSet* SlModManager::GetTextureSet(const char* name)
{
    int hash = Hash(name);
    for (const auto& set : TextureSets)
    {
        if (set->GetName() == hash)
            return set;
    }

    return nullptr;
}

void SlModManager::DestroyTextureSet(const char* name)
{
    SlGlobalTextureSet* set = GetTextureSet(name);
    if (set != nullptr)
    {
        auto it = std::find(TextureSets.begin(), TextureSets.end(), set);
        TextureSets.erase(it);
        delete set;
        
        LOG("Texture set %s has been destroyed", name);
    }
}

void SlModManager::MakeTextureSet(const char* name, const std::vector<SlStringT<char>>& files)
{
    DestroyTextureSet(name);
    SlGlobalTextureSet* set = new SlGlobalTextureSet(name, files);
    TextureSets.push_back(set);
    LOG("Texture set %s has been created", name);
}

bool SlModManager::OverrideTexture(int& texture, SiffLoadSet*& siff)
{
    for (const auto& set : TextureSets)
    {
        if (set->IsDisabled()) continue;
        for (const auto& file : set->GetFiles())
        {
            SiffLoadSet** handle = (SiffLoadSet**)gResourceManager->GetResource(file);
            if (siff == nullptr) continue;
            SiffLoadSet* ssiff = *handle;
            if (ssiff == nullptr) continue;

            if (ssiff->m_TextureManager.GetTexture1(texture) != nullptr)
            {
                // LOG("- Overwriting %08x with load set %s", texture, file.m_Data);
                siff = ssiff;
                return true;
            }
        }
    }

    return false;
}

SlGlobalTextureSet::SlGlobalTextureSet(const char* name, const std::vector<SlStringT<char>>& files) : Name(Hash(name)), Files(files), Resources(), Priority(), Enabled() {}

void SlGlobalTextureSet::Disable()
{
    if (!Enabled) return;

    Resources.UnloadResources();

    Enabled = false;
    Linked = false;
}

void SlGlobalTextureSet::Enable()
{
    if (Enabled) return;

    for (const auto& file : Files)
        Resources.AddResource(file, kResourceType_SumoTool, false);
    Resources.StartLoadResources();

    Linked = false;
    Enabled = true;
}

void ClvMain()
{
    gSlMod = new SlModManager();
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (!GetConsoleWindow())
    {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
    }

    LOG("Loading mods from %s", std::filesystem::relative("data/mods").string().c_str());
    for (const auto& moddir : std::filesystem::directory_iterator("data/mods"))
    {
        if (!std::filesystem::is_directory(moddir)) continue;
        InstallMod(moddir);
    }

    gSlMod->SetupRacerTextureSets();

    SetupGameNatives();
    PatchWeaponSetupManager();
    InitHooks();


    uintptr_t fnd = (uintptr_t)&Hack_GetPlayerUsableUnlockedCharacters_UpdateStats - 0x00849e4f;
    PatchExecutableSection((void*)0x00849e4b, &fnd, sizeof(uintptr_t));

}

void ClvClose()
{
    MH_Uninitialize();
}