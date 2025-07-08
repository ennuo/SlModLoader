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

    struct Relocation {
        unsigned int Flags; // Technically a short, but it doesn't really matter
        unsigned int Offset;
    };
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

class SiffTextManager {
    void* m_pStart;
    void* m_pEnd;
};

class KeyFrameManager {
    void* m_pStart;
    void* m_pEnd;
};

class CustomTextureManager {
public:
    virtual ~CustomTextureManager();
    virtual void RegisterTexturePack();
    virtual void UnregisterTexturePack();
    virtual void SetTexture();
    virtual void GetTexture();
    virtual void GetTexture1();
    virtual void UnregisterTextureData();
    virtual void RegisterTextureData();
private:
    void* em_texture_manager;
};

class SiffLoadSet {
public:
    DEFINE_MEMBER_FN_0(__ctor, SiffLoadSet*, 0x004ef600);
    inline SiffLoadSet() { __ctor(); }
private:
    bool m_is_loaded;
    bool m_is_linked;
	unsigned char* p_common_dat_file;
	int common_dat_file_size;
	unsigned int common_dat_file_id;
	unsigned char* p_common_rel_file;
	int common_rel_file_size;
	unsigned int common_rel_file_id;
	unsigned char* p_common_gpu_file;
	int common_gpu_file_size;
	unsigned int common_gpu_file_id;
	unsigned char* p_locale_dat_file;
	int locale_dat_file_size;
	unsigned int locale_dat_file_id;
	unsigned char* p_locale_rel_file;
	int locale_rel_file_size;
	unsigned int locale_rel_file_id;
	unsigned char* p_locale_gpu_file;
	int locale_gpu_file_size;
	unsigned int locale_gpu_file_id;
	char common_gpu_filename[300];
	char locale_gpu_filename[300];
public:
    CustomTextureManager m_TextureManager;
    SiffTextManager m_TextManager;
    KeyFrameManager m_KeyframeManager;
    SiffObjectDefManager m_ObjectDefManager;
};