#pragma once

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

class SlDeviceInput {
public:
    int GetRawAnalogFromChannel(int channel);
    bool GetButtonDown(int channel, int mask);
    bool GetButtonHeld(int channel, int mask);
    bool GetButtonPressed(int channel, int mask);
    bool GetButtonReleased(int channel, int mask);
private:
    void* GetFocusChannelRead(int channel);
};


extern SlDeviceInput*(*GetKeyboard)(int index);
extern SlDeviceInput*(*GetGamepad)(int index);

/// Initializes all function pointers and globals for the input manager on DLL inject.
void SetupInputNatives();