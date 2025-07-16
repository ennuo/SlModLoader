#pragma once

enum
{
    kVersion_Initial,
    kVersion_LatestPlusOne
};

class ClvRacerData
{
    int NameHash;
    int GridOrder;
    int StatId;
    bool Unlocked;
    bool Played;
    bool New;
};

class ClvTrackData
{
    int NameHash;
};

class ClvSavedataHeader
{
    int Magic;
    int Version;
    int TrackCount;
    int RacerCount;
};