#pragma once

class RacerInfo;

inline const u32 kCustomNetVersion = 1;

enum EPeerDataType
{   
    kPeerDataType_Players = 0x1,
    kPeerDataType_Characters = 0x8,
    kPeerDataType_Vote = 0x9,
    kPeerDataType_Score = 0xa,
    kPeerDataType_Stickers = 0xb,
    kPeerDataType_RankStats = 0xc,
    kPeerDataType_Username = 0xd, // ???
    kPeerDataType_All = 0xe,
    kPeerDataType_Extended = 0xf
};

enum EPeerDataSubType
{
    kPeerDataSubType_GameInfo,
    kPeerDataSubType_RacerAppearance,
    kPeerDataSubType_VanityEffects
};

namespace SumoNet
{
    class NetInPacket {
    public:
        int Peek(int bits);
        inline int Read(int bits)
        {
            int value = Peek(bits);
            mPosition += bits;
            return value;
        }
    private:
        int mDummy0;
        int mDummy1;
        int mPosition;
        unsigned char* mData;
    };

    class NetOutPacket {
    public:
        virtual ~NetOutPacket() = 0;
        virtual void Write(unsigned int data, int bits) = 0;
    public:
        inline void Write(unsigned int data) { Write(data, 32); }
        inline void WriteBool(bool data) { Write(data, 1); }
        inline void WriteBytes(unsigned char const* data, int len)
        {
            for (int i = 0; i < len; ++i)
                Write(data[i], 8);
        }
    };

    class NetAddr {
    public:
        inline NetAddr() { Clear(); }
        inline bool IsSet() const { return mFlags & 1; }
        inline const in_addr& GetInAddr() const { return mAddress; }

        inline void Clear()
        {
            mFlags &= ~1;
        }

        inline void Set(in_addr const& addr)
        {
            mFlags |= 1;
            mAddress = addr;
        }
    private:
        byte mFlags;
        in_addr mAddress;
    };

    class NetId {
    public:
        inline NetId() : mId(0) {}
        inline NetId(unsigned short id) : mId(id) {}
        inline NetId(const NetId& id) : mId(id.mId) {}
    public:
        inline operator unsigned short() const { return mId; }
    public:
        inline bool IsInitialised() const { return mId != 0; }
        inline NetId GetPeerId() const { return mId & ~7; }
        inline int GetPeerData() const { return mId >> 3; }
        inline int GetUserNum() const { return mId & 7; }
    private:
        unsigned short mId;
    };

    class NetOnlineId {
    private:
        int mOnlineId;
        int mSteamId;
        char mPad1[0xc];
        char mDisplayName[0x20];
    };

    class NetCharacter {
    public:
        inline NetCharacter() : mPlayerId() {}
        inline NetCharacter(byte id) : mPlayerId() { SetPlayerId(id); }
    public:
        inline void SetPlayerId(byte id) { mPlayerId = (id << 1) | 1; }
        inline void Reset() { mPlayerId &= ~1; }
        inline bool IsInitialised() const { return mPlayerId & 1; }
        inline byte GetPlayerId() const { return mPlayerId >> 1; }
    public:
        inline bool operator==(const NetCharacter& rhs) const { return mPlayerId == rhs.mPlayerId; }
        inline bool operator!=(const NetCharacter& rhs) const { return !(*this == rhs); }
    private:
        byte mPlayerId;
    };

    class NetMatchPlayer {
    public:
        inline const NetId& GetId() const { return mId; }
        inline bool is_character() const { return mPlayerCharacter.IsInitialised(); }
        inline NetCharacter& get_character() { return mPlayerCharacter; }
    public:
        void set_character(NetCharacter const& pc);
    private:
        char mPad[0x48];
        NetId mId;
        NetCharacter mPlayerCharacter;
    private:
        char mPad2[0x134];
    };

    struct NetMatchPlayerEx {
        bool mInitialised;
        int mNameHash;
    };

    class NetMatchPeerEx {
    public:
        inline NetMatchPeerEx() { Clear(); }
    public:
        inline NetMatchPlayerEx& GetPlayer(int num) { return mPlayers[num]; }
        void Clear();
    private:
        NetMatchPlayerEx mPlayers[4];
    };

    class NetMatchPeer {
    private:
        inline static NetMatchPeerEx mEx[13];
    public:
        inline NetId& GetId() const { return *(NetId*)(((uintptr_t)this) + 0x6a); }
        inline int GetNumPlayers() const { return mNumPlayers; }
        inline NetMatchPlayer& GetPlayer(int i) { return mPlayers[i]; }
        NetMatchPeerEx& GetEx();
        void PeerDataChanged(EPeerDataType type);
    public:
        void SetPlayerCharacter(int num, RacerInfo const& pc);
        void reset();
    private:
        char mPad0[0x25d0];
    private:
        int mNumPlayers;
        int mUnknown;
        NetMatchPlayer mPlayers[4];
    private:
        char mPad1[0x28];
    };

    class NetMatchRosterGenerator {
    public:
        
    };

    class NetLocals {
    struct LocalEx {
        unsigned int NameHash;
        RacerInfo* Info;
    };
    
    struct Local {
        int mPad; // 0 ???
        int PadIndex; // 4
        NetOnlineId OnlineId;
        NetCharacter Character;
    };
    private:
        inline static LocalEx mLocalEx[5];
    public:
        inline int GetNumLocal() const { return mNumLocal; }
        inline NetCharacter& GetCharacter(int player) { return mLocal[player].Character; }
        inline bool IsSpace() const { return mNumLocal < 5; }
        inline int GetPlayerIndexFromPadIndex(int pad) const
        {
            for (int i = 0; i < mNumLocal; ++i)
            {
                const Local& local = mLocal[i];
                if (local.PadIndex == pad)
                    return i;
            }

            return -1;
        }

        inline int GetPadIndex(int player) const
        {
            return mLocal[player].PadIndex;
        }

        bool SetCharacter(int pad, const RacerInfo& info);
        inline bool SetCharacter(int pad, NetCharacter const& ch)
        {
            for (int i = 0; i < mNumLocal; ++i)
            {
                if (GetPadIndex(i) != pad) continue;
                mLocal[i].Character = ch;
                return true;
            }

            return false;
        }
    private:
        int mNumLocal;
        Local mLocal[4];
    };

    class NetMatchState {
    public:
        virtual void Dummy() = 0;
    public:
        inline bool IsNewRound() const
        {
            return !((mRounds != -1 || mRoundNum != 0) && mRoundNum <= mRounds);
        }
    private:
        void* mHostManager;
        sbyte mRoundNum;
        sbyte mRounds;
        bool mLenientTimeouts;
    };

    class NetworkFEHelpers {
    public:
        DEFINE_STATIC_MEMBER_FN_1(GetPeerFromLobbyIndex, NetMatchPeer*, 0x00700da0, int& index);
    };

    class NetMatchSetupState {
    public:
        DEFINE_MEMBER_FN_2(GetPlayerId, NetId&, 0x0047e0e0, int player, int i);
        DEFINE_MEMBER_FN_2(IsHumanPlayer, bool, 0x0047e080, int player, int i);
        DEFINE_MEMBER_FN_2(GetCharacter, NetCharacter&, 0x0047e100, int player, int i);
        DEFINE_MEMBER_FN_3(IsPlayer, bool, 0x0047e020, const NetId& id, int& player, int& i);
    };
    
    class NetMatch {
    public:
        // NetMatchPeer[6] @ 0x55c
        inline NetLocals& GetLocals() const { return *(NetLocals*)(((uintptr_t)this) + 0x3e0); }
        inline NetMatchSetupState& GetSetupState() const { return *(NetMatchSetupState*)(((uintptr_t)this) + 0x101d34); }
        inline NetMatchPeer* GetMyPeer() const { return *(NetMatchPeer**)(((uintptr_t)this) + 0x55c); }
        inline NetId& GetMyPeerId() const { return GetMyPeer()->GetId(); }
        inline NetMatchState& GetState() const { return *(NetMatchState*)(((uintptr_t)this) + 0x101d28); }

        inline byte GetNumPeers() const { return *(byte*)(((uintptr_t)this) + 0x525); }
        inline NetMatchPeer* GetPeer(int i) const { return *(((NetMatchPeer**)(((uintptr_t)this) + 0x528)) + i); }
        inline NetMatchPeer* GetPeerBase() const { return (NetMatchPeer*)(((uintptr_t)this) + 0x568); }

        inline NetMatchPeer* GetPeer(NetId id) const
        {
            for (int i = 0; i < GetNumPeers(); ++i)
            {
                NetMatchPeer* peer = GetPeer(i);
                if (peer->GetId() == id.GetPeerId())
                    return peer;
            }

            return nullptr;
        }

        inline NetMatchPlayer* GetPlayer(NetId id) const
        {
            NetMatchPeer* peer = GetPeer(id);
            if (peer == nullptr || peer->GetNumPlayers() <= id.GetUserNum()) return nullptr;
            return &peer->GetPlayer(id.GetUserNum());
        }

        inline bool IsError() const
        {
            return mState == 4 || mState == 5;
        }

        inline bool IsRunning() const
        {
            return !IsError() && mState == 3;
        }

        inline bool HasRoundStarted() const
        {
            if (!IsRunning()) return false;
            return GetState().IsNewRound();
        }

        void SetLocalCharacter(int player, RacerInfo const& ch);

        // DEFINE_MEMBER_FN_2(SetLocalCharacter, void, 0x00473140, int player, const NetCharacter& ch);
    public:
        int mState;
    };

    class NetManager {
    public:
        inline NetMatch* GetMatch() { return *(NetMatch**)(((uintptr_t)this) + 0x8); }

    };

    inline static SlReloc<NetManager> gNetManager{0x00ec1a80};

    static_assert(sizeof(NetMatchPlayer) == 0x180);
    static_assert(sizeof(NetMatchState) == 0xC);
    static_assert(sizeof(NetMatchPeer) == 0x2c00);
};

class RacerNetworkId {
private:
    enum { kNetworkId_Random = 0x80 };
public:
    inline RacerNetworkId() : Bits()  { Flags |= kNetworkId_Random; }
    inline RacerNetworkId(int bits) : Bits(bits) {}
    inline RacerNetworkId(const RacerNetworkId& rhs) : Bits(rhs.Bits) {}
public:
    inline void Clear() { Bits = 0; }

    inline bool IsHuman() const { return PeerId.IsInitialised(); }
    inline bool IsCOM() const { return !IsHuman(); }
    inline bool IsRandom() const { return (Flags & kNetworkId_Random) == kNetworkId_Random; }

    inline const SumoNet::NetId& GetPeerId() const { return PeerId; }
    inline SumoNet::NetCharacter GetCharacter() const { return CharacterId; }
    inline byte GetCharacterId() const { return CharacterId; }

    inline void SetCharacter(const SumoNet::NetCharacter& pc) 
    { 
        CharacterId = pc.GetPlayerId(); 
        if (pc.IsInitialised()) Flags &= ~kNetworkId_Random;
        else Flags |= kNetworkId_Random;
    
    }
    inline void SetPeerId(const SumoNet::NetId& id) { PeerId = id; }
public:
    inline operator int() const { return Bits; }
private:
    union
    {
        #pragma pack(push, 1)
        struct
        {
            byte CharacterId;
            SumoNet::NetId PeerId;
            byte Flags;
        };
        #pragma pack(pop)

        int Bits;
    };
};

static_assert(sizeof(RacerNetworkId) == 0x4);

void Network_InstallHooks();