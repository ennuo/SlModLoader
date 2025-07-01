#pragma once

// sizeof: 0x64
// SmoToolRenderList @ 0x10
// they probably didnt change this at all since the first game,
// looks like all they did was add a single value?

class SceneManager {
public:
    DEFINE_MEMBER_FN_2(SetNewTexture, void, 0x00493e40, int object, int texture);
    DEFINE_MEMBER_FN_2(HideObject, void, 0x00493b60, int object, bool b);
    DEFINE_MEMBER_FN_2(ShowObject, void, 0x00493b10, int object, bool b);
};

/// Global scene manager for UI, gets linked to game's pointer at startup.
inline SlReloc<SceneManager> gSceneManager(0xEC1EC8);