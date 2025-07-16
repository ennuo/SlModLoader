#include <MinHook.h>

#include "Networking.hpp"
#include "Game.hpp"

namespace SumoNet
{
    void NetMatch::SetLocalCharacter(int player, const RacerInfo& info)
    {
        if (!IsRunning()) return;

        int pad = GetLocals().GetPadIndex(player);
        NetCharacter ch(info.Ex->StatId);

        GetLocals().SetCharacter(pad, info);
        GetMyPeer()->SetPlayerCharacter(player, info);
    }

    void NetMatchPlayer::set_character(NetCharacter const& pc)
    {
        if (!gNetManager->GetMatch()->HasRoundStarted())
            mPlayerCharacter = pc;
    }

    void NetMatchPeer::SetPlayerCharacter(int num, const RacerInfo& info)
    {
        NetCharacter pc(info.Ex->StatId);
        NetMatchPlayer& player = mPlayers[num];
        bool change = !player.is_character() || player.get_character() != pc;

        player.set_character(pc);
        GetEx().GetPlayer(num).mInitialised = true;
        GetEx().GetPlayer(num).mNameHash = info.NameHash;

        LOG("Setting character stat_id=%02x, racer_hash=%08x, racer_name=%s", info.Ex->StatId, info.NameHash, info.DisplayName);

        // if (change)
        //     PeerDataChanged(kPeerDataType_Characters);
    }

    bool NetLocals::SetCharacter(int pad, const RacerInfo& ch)
    {
        for (int i = 0; i < mNumLocal; ++i)
        {
            if (GetPadIndex(i) != pad) continue;
            mLocal[i].Character = ch.Ex->StatId;
            mLocalEx[i].NameHash = ch.NameHash;
            if (mLocalEx[i].Info != nullptr)
                delete mLocalEx[i].Info;

            return true;
        }

        return false;
    }

    NetMatchPeerEx& NetMatchPeer::GetEx()
    {
        return mEx[this - gNetManager->GetMatch()->GetPeerBase()];
    }

    void NetMatchPeerEx::Clear()
    {
        memset(this, 0, sizeof(NetMatchPeerEx));
    }
}

using namespace SumoNet;

void (__fastcall *SumoNet_NetMatchPeer_reset)();
void __fastcall Network_OnPeerReset(NetMatchPeer* peer)
{
    __asm { mov peer, edi }

    NetMatch* match = gNetManager->GetMatch();
    if (match != nullptr)
    {
        int index = peer - gNetManager->GetMatch()->GetPeerBase();
        LOG("Resetting peer (%d/%d)!", index, 0xd);

        peer->GetEx().Clear();
    }

    __asm { mov edi, peer }
    SumoNet_NetMatchPeer_reset();
}

int Network_LobbyGetCharacter(int num)
{
    RacerNetworkId id;

    NetMatchPeer* peer = NetworkFEHelpers::GetPeerFromLobbyIndex(num);
    if (peer != nullptr && num < peer->GetNumPlayers())
    {
        NetMatch* match = gNetManager->GetMatch();
        NetMatchSetupState& setup = match->GetSetupState();
        NetMatchPlayer& player = peer->GetPlayer(num);

        int type;
        if (setup.IsPlayer(player.GetId(), num, type))
        {
            id.SetCharacter(setup.GetCharacter(num, type));
            if (setup.IsHumanPlayer(num, type))
                id.SetPeerId(setup.GetPlayerId(num, type));
        }
    }

    return id;
}

int Network_GetCharacter(int num)
{
    RacerNetworkId id;

    NetMatch* match = gNetManager->GetMatch();
    if (match != nullptr && !match->IsError())
    {
        NetMatchSetupState& setup = match->GetSetupState();
        id.SetCharacter(setup.GetCharacter(num, 0));
        if (setup.IsHumanPlayer(num, 0))
            id.SetPeerId(setup.GetPlayerId(num, 0));
    }

    return id;
}



// Let everybody use duplicate characters in online races
// if they want.
bool Network_IsCharacterAvailable(RacerNetworkId id)
{
    return true;
}

void Network_RequestLocalCharacter(int pad, const RacerInfo& info)
{
    auto match = gNetManager->GetMatch();
    if (match == nullptr || match->IsError()) return;

    int player = match->GetLocals().GetPlayerIndexFromPadIndex(pad);

    LOG("[NETWORK] REQUEST %s", info.DisplayName);
    LOG("[NETWORK] Locals - %08x", (uintptr_t)&match->GetLocals());

    if (0 <= player)
        match->SetLocalCharacter(player, info);
}

void Network_InstallHooks()
{
    CREATE_HOOK(0x700520, Network_IsCharacterAvailable, nullptr);
    CREATE_HOOK(0x700340, Network_RequestLocalCharacter, nullptr);
    CREATE_HOOK(0x007004e0, Network_GetCharacter, nullptr);
    CREATE_HOOK(0x00700f30, Network_LobbyGetCharacter, nullptr);
    CREATE_HOOK(0x004760a0, Network_OnPeerReset, &SumoNet_NetMatchPeer_reset);

    // Patch all calls to Network_RequestLocalCharacter to
    // pass the pointer to RacerInfo instead of reading the network id member.

    uint8_t PUSH_EAX = 0x50;
    uint8_t PUSH_EBP = 0x55;
    intptr_t SM_SET_ICON_FN = GetFnAddr(&SceneManager::SetRacerIconTexture) - ASLR(0x008428f6);

    PatchExecutableSection((void*)ASLR(0x0065cf3c), &PUSH_EAX, sizeof(PUSH_EAX));
    PatchExecutableSection((void*)ASLR(0x008503cb), &PUSH_EBP, sizeof(PUSH_EBP));
    PatchExecutableSection((void*)ASLR(0x008428f2), &SM_SET_ICON_FN, sizeof(SM_SET_ICON_FN));
    PatchExecutableSection((void*)ASLR(0x0084272c), &PUSH_EBP, sizeof(PUSH_EBP));
}