#ifndef SHADOW_MEMORY_H
#define SHADOW_MEMORY_H

#include <stdint.h>

class ThirdMap {
private:
    uint8_t TM[8192];

public:
    ThirdMap();
    ~ThirdMap();

    bool checkByte(uint64_t tm, uint64_t dtm);
    void markByte(uint64_t tm, uint64_t dtm);
    void freeByte(uint64_t tm, uint64_t dtm);
};

class SecondMap {
private:
    ThirdMap* SM[65536];

public:
    SecondMap();
    ~SecondMap();

    bool checkByte(uint64_t sm, uint64_t tm, uint64_t dtm);
    void markByte(uint64_t sm, uint64_t tm, uint64_t dtm);
    void freeByte(uint64_t sm, uint64_t tm, uint64_t dtm);
};

class ShadowMemory {
private:
    SecondMap* PM[65536];

public:
    ShadowMemory();
    ~ShadowMemory();

    bool checkByte(uint64_t address);
    bool checkByteRange(uint64_t address, uint32_t size);
    void markByte(uint64_t address);
    void markByteRange(uint64_t address, uint32_t size);
    void freeByte(uint64_t address);
    void freeByteRange(uint64_t address, uint32_t size);
    
};

#endif