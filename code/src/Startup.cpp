#include <Windows.h>

#include "Core.hpp"
#include "Game.hpp"
#include "Frontend.hpp"
#include "ModLoader.hpp"

SlReloc<ResourceList> gFrontendToolResources{0x00be2828};

void cFrontEndManager::StartLoadingFESoundBank()
{
    gResourceManager->LoadResource(IslandsSoundBank, kResourceType_SumoTool, false);
}

class GameManager {
public:
    inline static SlReloc<GameManager*> ms_pSE_RTTI_SINGLETON{0x00e9a408};
public:
    DEFINE_MEMBER_FN_0(IsPreLoadComplete, bool, 0x00675710);
    DEFINE_MEMBER_FN_0(IsResidentDataLoaded, bool, 0x00675740);
    DEFINE_MEMBER_FN_0(StartLoadResidentData, void, 0x00675730);
};

cStartup::cStartup() : cFrontendScene(), mTime(0.0f) {}

enum
{
    kSceneState_PreUpdate,
    kSceneState_Update,
    kSceneState_PostUpdate,
    kSceneState_Finish
};

void cStartup::PreUpdate()
{
    enum
    {
        kInitial,
        kLoadSoundbank,
        kPlayLogos,
    };

    switch (mPreUpdateState)
    {
        case kInitial:
        {
            if (!GameManager::ms_pSE_RTTI_SINGLETON->IsPreLoadComplete())
                break;

            cFrontEndManager::StartLoadingFESoundBank();
            gResourceManager->LoadResource("ui\\frontend\\LicenceSplash", kResourceType_SumoTool, false);

            mPreUpdateState = kLoadSoundbank;

            break;
        }
        case kLoadSoundbank:
        {
            if (gResourceManager->IsLoading("ui\\frontend\\LicenceSplash"))
                break;

            mPreUpdateState = kPlayLogos;
            break;
        }
        case kPlayLogos:
        {
            NextState();
            break;
        }
    }
}

void cStartup::Update()
{
    enum { kInitial, kWaitForResidentData };
    switch (mUpdateState)
    {
        case kInitial:
        {
            if (!GameManager::ms_pSE_RTTI_SINGLETON->IsResidentDataLoaded())
                GameManager::ms_pSE_RTTI_SINGLETON->StartLoadResidentData();

            gFrontendToolResources->ClearResourceList();

            gSlMod->LoadFrontendAssets(&gFrontendToolResources);

            gFrontendToolResources->AddResource("ui\\frontend\\NewFE\\MainFE", kResourceType_SumoTool, false);
            gFrontendToolResources->AddResource("ui\\frontend\\Stickers\\Stickers", kResourceType_SumoTool, false);
            gFrontendToolResources->AddResource("ui\\frontend\\Licence\\Licence", kResourceType_SumoTool, false);
            gFrontendToolResources->AddResource("ui\\frontend\\fruitmachine\\fruitmachine", kResourceType_SumoTool, false);

            cFrontEndManager::ms_pSE_RTTI_SINGLETON->fe3d->LoadFrontendAssets(&gFrontendToolResources);
            
            gFrontendToolResources->SortList();
            gFrontendToolResources->StartLoadResources();

            mUpdateState = kWaitForResidentData;

            break;
        }

        case kWaitForResidentData:
        {
            if (!GameManager::ms_pSE_RTTI_SINGLETON->IsResidentDataLoaded()) break;

            if (!gFrontendToolResources->IsLoaded())
                break;

            int len = SlInput::m_Device->size();
            LOG("Devices : %d", len);
            for (const auto& device : *SlInput::m_Device)
            {
                const char* cls = device->GetClassName();
                if (strstr(cls, "SlGamepad") != NULL)
                {
                    SlStringT<char> device_name = ((SlGamepad*)device)->GetDeviceFriendlyName();
                    LOG("- %s::%s", cls, device_name.m_Data);
                }
                else LOG("- %s", cls);
            }

            LOG("Gamepads: %d", SlInput::m_Gamepad->size());
            for (const auto& device : *SlInput::m_Gamepad)
            {
                SlStringT<char> device_name = device->GetDeviceFriendlyName();
                LOG("- %s::%s", device->GetClassName(), device_name.m_Data);
            }

            SlGamepad* mouse = (*SlInput::m_Gamepad)[0];
            SlGamepad* keyboard = (*SlInput::m_Gamepad)[1];

            (*SlInput::m_Gamepad)[0] = keyboard;
            (*SlInput::m_Gamepad)[1] = mouse;


            cFrontEndManager::SetMasterPlayer(0);
            cFrontEndManager::ms_pSE_RTTI_SINGLETON->SetMenuKeyboardMasterPlayer(0);

            NextState();
        }
    }
}

void cStartup::PostUpdate()
{
    NextState();
}