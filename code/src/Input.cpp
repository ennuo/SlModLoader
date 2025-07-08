#include "Input.hpp"
#include "Game.hpp"

void* SlGamepad::GetFocusChannelRead(int channel)
{
    int read = GetCleanedFocusChannelRead(channel);
    return (void*)(read * 0x19c + *(int*)((uintptr_t)this + 0x57c));
}

bool SlKeyboard::GetKeyPressed(int channel, int key)
{
    int read = GetCleanedFocusChannelRead(channel);
    return *(int*)(read * 0x104c + *(int*)((uintptr_t)this + 0x57c) + 0x80c + key * 4);
}