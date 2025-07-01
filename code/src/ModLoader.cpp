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

#include <windows.h>
#include <toml.hpp>

#include <iostream>
#define LOG(x, ...) { printf(""##x##"\n", __VA_ARGS__); }

class GameDatabase {
public:
    inline static SlReloc<unsigned int> m_uNumRacers{0x00bd0318};
};

SlReloc<bool> m_bOldFrontend(0x00bc6cd7);

static uint8_t g_MaxRacers = 255;
RacerInfo** g_FirstRacer;
RacerInfo** g_LastRacer;

void (__thiscall *GameCamera_SwitchTo)(void*, int);
void (__thiscall *GameCamera_Reset)(void*);
SlDeviceInput* (__thiscall *Racer_GetGamepad)(void*);

// 54 visual racer slots


void PatchExecutableSection(void* address, void* data, int size)
{
    DWORD old_protect;
    VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &old_protect);
    memcpy(address, data, size);
    VirtualProtect(address, size, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), address, size);
}

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
    PatchExecutableSection((void*)(gMemoryBase - gExecutableBase + 0x88d157), &analog_state_index, sizeof(uint8_t));
    PatchExecutableSection((void*)(gMemoryBase - gExecutableBase + 0x88d198), &analog_state_index, sizeof(uint8_t));
    PatchExecutableSection((void*)(gMemoryBase - gExecutableBase + 0x88d1df), &analog_state_index, sizeof(uint8_t));
    PatchExecutableSection((void*)(gMemoryBase - gExecutableBase + 0x88d1f8), &analog_state_index, sizeof(uint8_t));
    PatchExecutableSection((void*)(gMemoryBase - gExecutableBase + 0x88d286), &analog_state_index, sizeof(uint8_t));

    PatchExecutableSection((void*)(gMemoryBase - gExecutableBase + 0x88d286), &analog_state_index, sizeof(uint8_t));


    // uint8_t shellcode[2] = { 0xeb, 0x0f };
    // uint8_t nop[2] = { 0x66, 0x90 };
    // PatchExecutableSection((void*)0x42b837, &shellcode, sizeof(uint8_t) * 2);
    // PatchExecutableSection((void*)0x42b86d, &nop, sizeof(uint8_t) * 2);
}

glm::vec3 g_CameraPosition;
glm::vec3 g_CameraRotation;
glm::mat4 g_InverseCameraRotation;


// void __fastcall cCharacterLoader_PerformLoad(void* self)
// {
//     ResourceList& resource_list = *(ResourceList*)((uintptr_t)self + 0x54);
//     if (!resource_list.IsLoaded() && resource_list.GetSize() != 0) return;

//     resource_list.UnloadResources();

//     SlStringT<char>& path = *(SlStringT<char>*)((uintptr_t)self + 0x2c);
//     path = "FeCharacters\\QuestionMarkHolo\\QuestionMarkHolo";

//     resource_list.AddResource(path, 3, false, false);

    
//     resource_list.StartLoadResources();
// }


void (__thiscall *FreeCamera_OnUpdate)(void* self, float);
void __fastcall OnFreeCameraUpdate(void* self, void* _eax, float f)
{
    void** racers = (void**)LoadPointer(0x7CE920);
    int num_racers = LoadMemory<int>(0x7CE924);
    if (racers == nullptr || num_racers < 1) return;
    void* racer = *racers;
    if (racer == nullptr) return;
    SlDeviceInput* gamepad = Racer_GetGamepad(racer);
    if (gamepad == nullptr) return;

    char* state_data = *(char**)((char*)gamepad + 0x57c);

    float left_x = *(float*)(state_data + 0x94);
    float left_y = *(float*)(state_data + 0x98);

    float right_x = *(float*)(state_data + 0xa0);
    float right_y = *(float*)(state_data + 0xa4);

    float l2 = *(float*)(state_data + 0xcc);
    float r2 = *(float*)(state_data + 0xd4);

    glm::vec3& pos = *(glm::vec3*)((uintptr_t)self + 0x10);
    glm::vec3& front = *(glm::vec3*)((uintptr_t)self + 0x20);
    glm::vec3& up = *(glm::vec3*)((uintptr_t)self + 0x30);

    g_CameraRotation.x += right_y * 0.01f;
    g_CameraRotation.y += right_x * 0.01f;

    g_InverseCameraRotation = 
        glm::rotate(glm::mat4(1.0f), -g_CameraRotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), -g_CameraRotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), -g_CameraRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));

    g_CameraPosition += glm::vec4(-left_x, left_y, -l2 + r2, 1.0f) * g_InverseCameraRotation;

    glm::mat4 rotation =
        glm::rotate(glm::mat4(1.0f), g_CameraRotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), g_CameraRotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), g_CameraRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 translation =
        glm::translate(glm::mat4(1.0f), g_CameraPosition);

    glm::mat4 view = rotation * translation; 
    glm::mat4 inverse = glm::inverse(view);

    pos = g_CameraPosition;
    front = g_CameraPosition + glm::vec3(glm::normalize(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * inverse));
    up = glm::normalize(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * inverse);
}

// free camera is 5
void(*RaceHandler_Update)(float);
void OnRaceHandlerUpdate(float f)
{
    RaceHandler_Update(f);

    void** racers = (void**)LoadPointer(0x7CE920);
    int num_racers = LoadMemory<int>(0x7CE924);
    if (racers == nullptr || num_racers < 1) return;
    void* racer = *racers;
    if (racer == nullptr) return;

    SlDeviceInput* gamepad = Racer_GetGamepad(racer);
    if (gamepad == nullptr) return;

    void* camera = *(void**)((uintptr_t)racer + 0xc1b4);
    if (camera == nullptr) return;

    int camera_type = *(int*)((uintptr_t)camera + 0x484);
    // if (camera_type == 5)
    //     OnFreeCameraUpdate(*(void**)((uintptr_t)camera + 0x464), nullptr, 0.0f);

    if (gamepad->GetButtonPressed(10, L3))
    {
        if (camera_type != 5)
        {
            GameCamera_SwitchTo(camera, 5);
        }
        else
        {
            GameCamera_SwitchTo(camera, 0);
        }

        GameCamera_Reset(camera);
    }
}

void(__thiscall *cCharacterSelectMulti_SetUpGrid)(void*);
void __fastcall OnSetUpGrid(void* self)
{
    char buf[255];
    for (int i = 40; i < 54; ++i)
    {
        sprintf(buf, "CHARACTER_SELECT\\CHAR_MOVE_%d", i);
        gSceneManager->HideObject(sumohash(buf), true);
        sprintf(buf, "CHARACTER_SELECT\\CHAR_STATE_%d", i);
        gSceneManager->HideObject(sumohash(buf), true);
        sprintf(buf, "CHARACTER_SELECT\\CHAR_%d", i);
        gSceneManager->HideObject(sumohash(buf), true);
        sprintf(buf, "CHARACTER_SELECT\\MAXED_ICON_%d", i);
        gSceneManager->HideObject(sumohash(buf), true);
        sprintf(buf, "CHARACTER_SELECT\\CHAR_%d_BACKGROUND_A", i);
        gSceneManager->HideObject(sumohash(buf), true);
        sprintf(buf, "CHARACTER_SELECT\\CHAR_%d_BACKGROUND_B", i);
        gSceneManager->HideObject(sumohash(buf), true);
        sprintf(buf, "CHARACTER_SELECT\\CHAR_%d_FOREGROUND", i);
        gSceneManager->HideObject(sumohash(buf), true);
    }

    cCharacterSelectMulti_SetUpGrid(self);
}

// This function call seems to have been optimized, so the second argument gets passed into EDI, I don't think
// there's a standard calling convention for this? So we'll have to use a naked function definition.
__declspec(naked) Siff::Object::TableEntry* __fastcall OnGetObjectDef(SiffObjectDefManager* manager, unsigned int hash)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, __LOCAL_SIZE

        mov manager, ecx
        mov hash, edi
    }
    
    {
        auto entry = manager->GetObjectDef(hash);
        __asm {
            mov eax, entry
        }
    }

    __asm {
        mov esp, ebp
        pop ebp
        ret
    }
}

class FrontEndManager {
public:
    inline static SlReloc<ResourceList> m_FrontEndResources{0xFC7870};
public:
    void AddFrontEndResources(sGameFile* files)
    {
        // The texture manager seems to work by only loading the first instance of
        // a texture with a specific UID.
        // So if we load any of our custom frontend resource packs first, we should
        // have priority over the default ones.


        MessageBoxA(NULL, "Loading our resources!", "DEBUG", MB_OK);


        // Load the standard pre-requisites for the frontend.
        while (files->Type != kResourceType_Invalid)
        {
            const auto& file = *files;
            m_FrontEndResources->AddResource(file.Path, file.Type, false, false);
            files++;
        }

    }
};

void InitHooks()
{
    MH_Initialize();

    // CREATE_HOOK(0x448B90, OnSetUpGrid, &cCharacterSelectMulti_SetUpGrid);
    CREATE_HOOK(0x96700, OnGetObjectDef, nullptr);
    CREATE_MEMBER_HOOK(0x2828B0, FrontEndManager::AddFrontEndResources, nullptr);
    // CREATE_HOOK((0x00722cc0 - gExecutableBase), NullFn, nullptr);
    // CREATE_HOOK(0x2AAE40, OnRaceHandlerUpdate, &RaceHandler_Update);
    // CREATE_HOOK(0x48CFA0, OnFreeCameraUpdate, &FreeCamera_OnUpdate);

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
    SlTextureEntry() : Hash(), Path() {}
    SlTextureEntry(int hash, const std::filesystem::path& path) : Hash(hash), Path(path) {}
    SlTextureEntry(const std::string& name, const std::filesystem::path& path)
    {
        char local[MAX_PATH];
        sprintf(local, "%s/%s", name.c_str(), path.filename().string().c_str());

        Hash = sumohash(local);
        Path = path;
    }
public:
    int Hash;
    std::filesystem::path Path;
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
        HRESULT res = DirectX::GetMetadataFromWICFile(sprites[i].Path.wstring().c_str(), DirectX::WIC_FLAGS_NONE, info);

        stbrp_rect rect;
        rect.id = i;

        if (res != NOERROR)
        {
            LOG(" - Failed to get metadata for %s", sprites[i].Path.string().c_str());

            rect.w = 0;
            rect.h = 0;
        }
        else
        {
            LOG(" - %s (%dx%d)", sprites[i].Path.string().c_str(), info.width, info.height);

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
        HRESULT res = DirectX::LoadFromWICFile(sprite.Path.wstring().c_str(), DirectX::WIC_FLAGS_NONE, nullptr, scratch_image);
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
        t.Hash = sprites[rect.id].Hash;
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
        std::filesystem::create_directory(cachedir);

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
    auto cache_path = moddir / ".sumotool_build";

    toml::table config;
    try { config = toml::parse_file(config_path.string()); }
    catch (std::exception& ex)
    {
        LOG(" - Failed to load \"%s\": %s", config_path.string().c_str(), ex.what());
        return;
    }

    if (!config["enabled"].value_or(true)) return;

    std::string version = config["version"].value_or("1.0.0");
    std::string name = config["name"].value_or(moddir.filename().string().c_str());
    LOG("%s", name.c_str());

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
        racer->BaseId = config["racer"]["echo"].value_or("amd");
        racer->InternalId = config["racer"]["id"].value_or("sonic");
        racer->DisplayName = config["racer"]["name"].value_or("Unnamed Racer");
        racer->FrontendResources = SlStringT<char>((name + "/.sumotool_build/frontend").c_str());
        racer->GameResources = SlStringT<char>((name + "/.sumotool_build/race").c_str());

        gSlMod->Racers.push_back(racer);



    }
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

    SetupGameNatives();
    PatchWeaponSetupManager();
    InitHooks();

    *(void**)(&GameCamera_SwitchTo) = (void*)GetAddress(0x364840);
    *(void**)(&Racer_GetGamepad) = (void*)GetAddress(0x2BC280);
    *(void**)(&GameCamera_Reset) = (void*)GetAddress(0x364420);

    g_FirstRacer = (RacerInfo**)(gMemoryBase + 0x851C08);
    g_LastRacer = (RacerInfo**)(gMemoryBase + 0x851C0c);
}

void ClvClose()
{
    MH_Uninitialize();
}