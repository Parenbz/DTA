#ifndef SHADOW_MEMORY_H
#define SHADOW_MEMORY_H
#include "QBDI.h"

class ThirdMap {
private:
    size_t TM[65536];

public:
    ThirdMap();
    ~ThirdMap();

    size_t checkByte(QBDI::rword tm);
    void markByte(QBDI::rword tm, size_t mark);
    void freeByte(QBDI::rword tm);
};

class SecondMap {
private:
    ThirdMap* SM[65536];

public:
    SecondMap();
    ~SecondMap();

    size_t checkByte(QBDI::rword sm, QBDI::rword tm);
    void markByte(QBDI::rword sm, QBDI::rword tm, size_t mark);
    void freeByte(QBDI::rword sm, QBDI::rword tm);
};

class ShadowMemory {
private:
    SecondMap* PM[65536];
    QBDI::GPRState regs;

public:
    ShadowMemory();
    ~ShadowMemory();

    size_t checkByte(QBDI::rword address);
    size_t checkByteRange(QBDI::rword address, QBDI::rword size);
    size_t checkRegister(int16_t regCtxIdx);
    void taintByte(QBDI::rword address, size_t color);
    void taintByteRange(QBDI::rword address, QBDI::rword size, size_t color);
    void taintRegister(int16_t regCtxIdx, size_t color);
    void freeByte(QBDI::rword address);
    void freeByteRange(QBDI::rword address, QBDI::rword size);
    void freeRegister(int16_t regCtxIdx);
};

#endif