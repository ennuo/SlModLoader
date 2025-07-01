#include "Input.hpp"
#include "Game.hpp"

SlDeviceInput*(*GetKeyboard)(int);
SlDeviceInput*(*GetGamepad)(int);





int(__thiscall *SlGamepad_GetRawAnalogFromChannel)(SlDeviceInput* pad, int channel);
int(__thiscall *SlGamepad_GetCleanedFocusChannelRead)(SlDeviceInput* pad, int channel);
bool(__thiscall *SlGamepad_GetButtonDown)(SlDeviceInput* pad, int channel, int mask);
bool(__thiscall *SlGamepad_GetButtonReleased)(SlDeviceInput* pad, int channel, int mask);
bool(__thiscall *SlGamepad_GetButtonHeld)(SlDeviceInput* pad, int channel, int mask);
bool(__thiscall *SlGamepad_GetButtonPressed)(SlDeviceInput* pad, int channel, int mask);

int SlDeviceInput::GetRawAnalogFromChannel(int channel)
{
    return SlGamepad_GetRawAnalogFromChannel(this, channel);
}

void* SlDeviceInput::GetFocusChannelRead(int channel)
{
    int read = SlGamepad_GetCleanedFocusChannelRead(this, channel);
    return (void*)(read * 0x19c + *(int*)((uintptr_t)this + 0x57c));
}

bool SlDeviceInput::GetButtonDown(int channel, int mask)
{
    return SlGamepad_GetButtonDown(this, channel, mask);
}

bool SlDeviceInput::GetButtonPressed(int channel, int mask)
{
    return SlGamepad_GetButtonPressed(this, channel, mask);
}

bool SlDeviceInput::GetButtonHeld(int channel, int mask)
{
    return SlGamepad_GetButtonHeld(this, channel, mask);
}

bool SlDeviceInput::GetButtonReleased(int channel, int mask)
{
    return SlGamepad_GetButtonReleased(this, channel, mask);
}

void SetupInputNatives()
{
    *(void**)(&GetKeyboard) = (void*)GetAddress(0x2680);
    *(void**)(&GetGamepad) = (void*)GetAddress(0x15360);
    
    *(void**)(&SlGamepad_GetCleanedFocusChannelRead) = (void*)GetAddress(0x20F830);

    *(void**)(&SlGamepad_GetButtonHeld) = (void*)GetAddress(0x25C230);
    *(void**)(&SlGamepad_GetButtonPressed) = (void*)GetAddress(0x2650);
    *(void**)(&SlGamepad_GetButtonDown) = (void*)GetAddress(0x2620);
    *(void**)(&SlGamepad_GetButtonReleased) = (void*)GetAddress(0x25C260);
    *(void**)(&SlGamepad_GetRawAnalogFromChannel) = (void*)GetAddress(0x210C30);
}
