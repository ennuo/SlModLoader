#include <MinHook.h>

#include "Networking.hpp"
#include "Game.hpp"

namespace SumoNet
{
    int NetInPacket::Peek(int bits)
    {
        unsigned int value;
        typedef void (*Delegate)(unsigned int*, int, unsigned char*);
        const static uintptr_t address = ASLR(0x00474e70);

        int v = mPosition + mDummy0;
        __asm mov ecx, v
        (*(Delegate*)&address)(&value, bits, mData);
    }

    void NetMatch::SetLocalCharacter(int player, const RacerInfo& info)
    {
        if (!IsRunning()) return;

        int pad = GetLocals().GetPadIndex(player);
        GetLocals().SetCharacter(pad, info);
        GetMyPeer()->SetPlayerCharacter(player, info);
    }

    void NetMatchPlayer::set_character(NetCharacter const& pc)
    {
        if (!gNetManager->GetMatch()->HasRoundStarted())
            mPlayerCharacter = pc;
    }

    // in debug mode runtime stack checks store the parameter in esi
    // so thats just going to break this function
    #pragma runtime_checks("s", off)
    void NetMatchPeer::PeerDataChanged(EPeerDataType type)
    {
        typedef void (__fastcall *Delegate)(NetMatchPeer*, EPeerDataType);
        const static uintptr_t address = ASLR(0x00476a60);
        uintptr_t self = (uintptr_t)this;
        __asm mov esi, self
        (*(Delegate*)&address)(nullptr, type);
    }
    #pragma runtime_checks("s", restore)

    void NetMatchPeer::SetPlayerCharacter(int num, const RacerInfo& info)
    {
        NetCharacter pc(info.Ex->StatId);
        NetMatchPlayer& player = mPlayers[num];

        player.set_character(pc);
        GetEx().GetPlayer(num).mInitialised = true;
        GetEx().GetPlayer(num).mNameHash = info.NameHash;

        LOG("Setting character peer_id=%04x, stat_id=%02x, racer_hash=%08x, racer_name=%s", (unsigned short)GetId(), info.Ex->StatId, info.NameHash, info.DisplayName);

        PeerDataChanged(kPeerDataType_Characters);
        PeerDataChanged((EPeerDataType)(kPeerDataType_Extended | kPeerDataSubType_RacerAppearance << 8));
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

// Stack check will fail on this function because
// the stack pointer gets cached after I manually push the arguments
// for the functions, oops!
#pragma runtime_checks("s", off)
void (*SumoNet_NetMatchPeer_WritePeerData)();
void (*SumoNet_NetMatchPeer_ReadPeerData)();

__declspec(naked) void Network_OnReadPeerData(NetMatchPeer* peer)
{
    INLINE_ASM_PROLOGUE
    NetInPacket* packet;
    READ_EAX(packet);

    if (packet != nullptr)
    {
        if (packet->Peek(4) == kPeerDataType_Extended && packet->Read(4))
        {
            LOG("Reading extended packet data...");

            switch (packet->Read(24))
            {
                case kPeerDataSubType_RacerAppearance:
                {
                    LOG(" - Reading racer appearance packet...");
                    for (int i = 0; i < peer->GetNumPlayers(); ++i)
                    {
                        NetMatchPlayerEx& ex = peer->GetEx().GetPlayer(i);
                        ex.mInitialised = packet->Read(1);
                        if (ex.mInitialised)
                            ex.mNameHash = packet->Read(32);
                    }

                    break;
                }
            }
        }
        else
        {
            LOG("Reading base packet data...");
            
            __asm mov eax, peer
            __asm push eax
            __asm mov eax, packet

            SumoNet_NetMatchPeer_ReadPeerData();
        }
    }

    INLINE_ASM_EPILOGUE_N(4)
}

__declspec(naked) void Network_OnWritePeerData(NetMatchPeer* peer, int type)
{
    INLINE_ASM_PROLOGUE
    NetOutPacket* packet;
    READ_EAX(packet);

    if (packet != nullptr)
    {
        EPeerDataType main = (EPeerDataType)(type & 0xff);
        EPeerDataSubType secondary = (EPeerDataSubType)(type >> 8);

        LOG("Writing peer data of type %08x", main);
        if (main == kPeerDataType_Extended)
        {
            LOG("- Extended packet data of type: %06x", secondary);
            packet->Write(main, 4);
            packet->Write(secondary, 24);

            switch (secondary)
            {
                case kPeerDataSubType_RacerAppearance:
                {
                    for (int i = 0; i < peer->GetNumPlayers(); ++i)
                    {
                        NetMatchPlayerEx ex = peer->GetEx().GetPlayer(i);
                        packet->WriteBool(ex.mInitialised);
                        if (ex.mInitialised)
                            packet->Write(ex.mNameHash);
                    }

                    break;
                }
            }
        }
        else
        {

            __asm mov eax, main
            __asm push eax
            __asm mov eax, peer
            __asm push eax
            __asm mov eax, packet
            
            SumoNet_NetMatchPeer_WritePeerData();
        }
    }

    INLINE_ASM_EPILOGUE_N(8);
}
#pragma runtime_checks("s", restore)

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
    CREATE_HOOK(0x00476af0, Network_OnWritePeerData, &SumoNet_NetMatchPeer_WritePeerData);
    CREATE_HOOK(0x00476df0, Network_OnReadPeerData, &SumoNet_NetMatchPeer_ReadPeerData);

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