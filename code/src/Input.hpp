#pragma once

#include <vector>
#include "Core.hpp"

#include <usb_hid_keys.h>

#ifdef GetClassName
    #undef GetClassName
#endif

enum
{
    kFocusChannel_Default = 0,
    kFocusChannel_Frontend = 1,
    kFocusChannel_PopUp = 3,
    kFocusChannel_Debug = 10
};

enum SlButtonMasks
{
    DPAD_UP = (1 << 0),
    DPAD_DOWN = (1 << 1),
    DPAD_LEFT = (1 << 2),
    DPAD_RIGHT = (1 << 3),
    CROSS = (1 << 4),
    SQUARE = (1 << 5),
    TRIANGLE = (1 << 6),
    CIRCLE = (1 << 7),
    L3 = (1 << 8),
    R3 = (1 << 9),
    START = (1 << 10),
    SELECT = (1 << 11),
    L1 = (1 << 12),
    L2 = (1 << 13),
    R1 = (1 << 14),
    R2 = (1 << 15)
};

class SlInputDevice {
public:
    virtual ~SlInputDevice();
    virtual void Add();
    virtual void Remove(bool);
    virtual void Open();
    virtual void Update();
    virtual void Close();
    virtual void RefreshPower();
    virtual void ResetStates();
    virtual int GetType();
    virtual const char* GetClassName();
    virtual void SetDefaultRemapTable();
    virtual void SetDefaultAnalogConfig();
    virtual void SetFocusChannel(unsigned int);
public:
    DEFINE_MEMBER_FN_1(GetCleanedFocusChannelRead, int, 0x0060f830, int channel);
};

class SlKeyboard : public SlInputDevice {
public:
    bool GetKeyPressed(int channel, int key);
};

class SlGamepad : public SlInputDevice {
public:
    virtual void SetRumble(int);
    virtual void SetRumbleEX(float, float);
    virtual SlStringT<char> GetDeviceFriendlyName() const;
    virtual SlStringT<char> GetButtonFriendlyName() const;
    virtual SlStringT<char> GetAnalogFriendlyName() const;
    virtual void ProcessState();
    virtual bool IsViewport();
    virtual bool IsAudioCapable();
    virtual void GetPadType(unsigned int&);
    virtual bool UseAlternativePrimaryUIButton();
    virtual int GetRawAnalogFromChannel(unsigned int) const;
public:
    DEFINE_MEMBER_FN_2(GetButtonDown, bool, 0x00402620, int channel, int mask);
    DEFINE_MEMBER_FN_2(GetButtonHeld, bool, 0x0065c230, int channel, int mask);
    DEFINE_MEMBER_FN_2(GetButtonPressed, bool, 0x00402650, int channel, int mask);
    DEFINE_MEMBER_FN_2(GetButtonReleased, bool, 0x0065c260, int channel, int mask);
private:
    void* GetFocusChannelRead(int channel);
};

class SlInput {
public:
    inline static SlReloc<std::vector<SlInputDevice*>*> m_Device{0x00bc5e6c};
    inline static SlReloc<std::vector<SlGamepad*>*> m_Gamepad{0x00bc5e64};
    inline static SlReloc<std::vector<SlKeyboard*>*> m_Keyboard{0x00bc5e68};
};