#pragma once

enum
{
    kSumoToolPackageFile_Dat,
    kSumoToolPackageFile_Rel,
    kSumoToolPackageFile_Gpu,
    kSumoToolPackageFile_Lang,
    kSumoToolPackageFile_LangRel,
    kSumoToolPackageFile_LangGpu,
    kSumoToolPackageFile_Max
};

enum
{
    kSiff_EndianMarker = 0x44332211
};

enum
{
    kSiffResource_Info = 0x4F464E49,
    kSiffResource_TexturePack = 0x58455450,
    kSiffResource_KeyFrame = 0x4D52464B,
    kSiffResource_ObjectDef = 0x534A424F,
    kSiffResource_Scene = 0x454E4353,
    kSiffResource_Font = 0x544E4F46,
    kSiffResource_Text = 0x54584554,
    kSiffResource_Relocations = 0x4F4C4552
};

struct SumoToolFileInfo {
    unsigned int Offset;
    unsigned int RealSize;
    unsigned int CompressedSize;
};

struct SumoToolPackage {
    SumoToolFileInfo Files[kSumoToolPackageFile_Max];
};

struct ChunkHeader {
    unsigned int id;
    unsigned int chunk_size;
    unsigned int data_size;
    unsigned int endian_indicator;
};

namespace Siff 
{
    struct Sheet {
        uintptr_t Offset;
        unsigned int Size;
    };

    struct Texture {
        unsigned int Hash;
        float TopLeft[2];
        float TopRight[2];
        float BottomRight[2];
        float BottomLeft[2];
        unsigned int Texture;
    };

    struct TexturePack {
        unsigned int NumSprites;
        unsigned int NumSheets;
        unsigned int SheetList;
    };

    #pragma pack(push, 1)
    struct Relocation {
        unsigned short Flags;
        unsigned int Offset;
    };
    #pragma pack(pop)
};

namespace Siff::Object
{
    struct TableEntry {
        unsigned int Hash;
        unsigned int Type;
        unsigned int* Index;
        void* LoadSet;
    };

    struct Header {
        Header* m_pNext;
        Header* m_pPrev;
        int m_NumObjects;
        unsigned int* m_Hashes;
        TableEntry* m_Objects;
    };
};

template <typename T>
class SumoList {
public:
    T* m_pStart;
    T* m_pEnd;
};

class SiffObjectDefManager {
public:
    Siff::Object::TableEntry* GetObjectDef(unsigned int hash);
private:
    Siff::Object::TableEntry* GetObjectDef_Linear(unsigned int hash);
public:
    SumoList<Siff::Object::Header> m_objectDefBlockList;
};