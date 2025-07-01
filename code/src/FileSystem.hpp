#pragma once

class SlFileSystem {
public:
    virtual void DummyMethod() = 0;
};

class SlFileSystemDefault : public SlFileSystem {
public:
    inline SlFileSystemDefault(int param0) : SlFileSystem()
    {
        unsigned int* _init = (unsigned int*)this;
        _init[0] = 0x009eec9c;
        _init[1] = 0;
        _init[2] = param0;
        _init[3] = (unsigned int)_init + 3;
        _init[4] = ~0ul;
        _init[5] = ~0ul;
        _init[6] = ~0ul;
        _init[7] = 0;
        _init[8] = 0;
        _init[9] = 0;
    }
private:
    char _Pad[0x24];
};
