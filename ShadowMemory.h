#ifndef SHADOW_MEMORY_H
#define SHADOW_MEMORY_H
#include "QBDI.h"

class ThirdMap {
private:
    uint8_t TM[8192];

public:
    ThirdMap();
    ~ThirdMap();

    bool checkByte(QBDI::rword tm, QBDI::rword dtm);
    void markByte(QBDI::rword tm, QBDI::rword dtm);
    void freeByte(QBDI::rword tm, QBDI::rword dtm);
};

class SecondMap {
private:
    ThirdMap* SM[65536];

public:
    SecondMap();
    ~SecondMap();

    bool checkByte(QBDI::rword sm, QBDI::rword tm, QBDI::rword dtm);
    void markByte(QBDI::rword sm, QBDI::rword tm, QBDI::rword dtm);
    void freeByte(QBDI::rword sm, QBDI::rword tm, QBDI::rword dtm);
};

class ShadowMemory {
private:
    SecondMap* PM[65536];
    QBDI::GPRState regs;

public:
    ShadowMemory();
    ~ShadowMemory();

    bool checkByte(QBDI::rword address);
    bool checkByteRange(QBDI::rword address, QBDI::rword size);
    bool checkRegister(int16_t regCtxIdx);
    void markByte(QBDI::rword address);
    void markByteRange(QBDI::rword address, QBDI::rword size);
    void markRegister(int16_t regCtxIdx);
    void freeByte(QBDI::rword address);
    void freeByteRange(QBDI::rword address, QBDI::rword size);
    void freeRegister(int16_t regCtxIdx);
};

#endif