#include <stdint.h>
#include "QBDI.h"
#include "ShadowMemory.h"

    ThirdMap::ThirdMap() {
        for (int i = 0; i < 65536; i++) {
            TM[i] = 0;
        }
    }

    ThirdMap::~ThirdMap() {}

    size_t ThirdMap::checkByte(QBDI::rword tm) {
        return TM[tm];
    }

    void ThirdMap::markByte(QBDI::rword tm, size_t mark) {
        TM[tm] = mark;
    }

    void ThirdMap::freeByte(QBDI::rword tm) {
        TM[tm] = 0;
    }

    SecondMap::SecondMap() {
        for (int i = 0; i < 65536; i++) {
            SM[i] = nullptr;
        }
    }

    SecondMap::~SecondMap() {
        for (int i = 0; i < 65536; i++) {
            if (SM[i] != nullptr) delete SM[i];
        }
    }

    size_t SecondMap::checkByte(QBDI::rword sm, QBDI::rword tm) {
        return SM[sm] == nullptr ? 0 : SM[sm]->checkByte(tm);
    }

    void SecondMap::markByte(QBDI::rword sm, QBDI::rword tm, size_t mark) {
        if (SM[sm] == nullptr) SM[sm] = new ThirdMap;
        SM[sm]->markByte(tm, mark);
    }

    void SecondMap::freeByte(QBDI::rword sm, QBDI::rword tm) {
        if (SM[sm] == nullptr) return;
        SM[sm]->freeByte(tm);
    }

    ShadowMemory::ShadowMemory() {
        for (int i = 0; i < 65536; i++) {
            PM[i] = nullptr;
        }
        for (int i = 0; i < QBDI::NUM_GPR; i++) {
            *(&(regs.rax) + i) = 0;
        }
    }

    ShadowMemory::~ShadowMemory() {
        for (int i = 0; i < 65536; i++) {
            if (PM[i] != nullptr) delete PM[i];
        }
    }

    size_t ShadowMemory::checkByte(QBDI::rword address) {
        QBDI::rword pm = (address & ((QBDI::rword)0xffff << 32)) >> 32;
        QBDI::rword sm = (address & ((QBDI::rword)0xffff << 16)) >> 16;
        QBDI::rword tm = address & (QBDI::rword)0xffff;
        return PM[pm] == nullptr ? 0 : PM[pm]->checkByte(sm, tm);
    }

    size_t ShadowMemory::checkByteRange(QBDI::rword address, QBDI::rword size) {
        size_t res;
        for (int i = 0; i < size; i++) {
            res = checkByte(address + i);
            if (res) return res;
        }
        return 0;
    }

    size_t ShadowMemory::checkRegister(int16_t regCtxIdx) {
        return regCtxIdx < QBDI::NUM_GPR ? *(&(regs.rax) + regCtxIdx) : 0;
    }

    void ShadowMemory::taintByte(QBDI::rword address, size_t color) {
        QBDI::rword pm = (address & ((QBDI::rword)0xffff << 32)) >> 32;
        QBDI::rword sm = (address & ((QBDI::rword)0xffff << 16)) >> 16;
        QBDI::rword tm = address & (QBDI::rword)0xffff;
        if (PM[pm] == nullptr) PM[pm] = new SecondMap;
        PM[pm]->markByte(sm, tm, color);
    }

    void ShadowMemory::taintByteRange(QBDI::rword address, QBDI::rword size, size_t color) {
        for (int i = 0; i < size; i++) {
            taintByte(address + i, color);
        }
    }

    void ShadowMemory::taintRegister(int16_t regCtxIdx, size_t color) {
        if (regCtxIdx < QBDI::NUM_GPR) *(&(regs.rax) + regCtxIdx) = color;
    }

    void ShadowMemory::freeByte(QBDI::rword address) {
        QBDI::rword pm = (address & ((QBDI::rword)0xffff << 32)) >> 32;
        QBDI::rword sm = (address & ((QBDI::rword)0xffff << 16)) >> 16;
        QBDI::rword tm = address & (QBDI::rword)0xffff;
        if (PM[pm] == nullptr) return;
        PM[pm]->freeByte(sm, tm);
    }

    void ShadowMemory::freeByteRange(QBDI::rword address, QBDI::rword size) {
        for (int i = 0; i < size; i++) {
            freeByte(address + size);
        }
    }

    void ShadowMemory::freeRegister(int16_t regCtxIdx) {
        if (regCtxIdx < QBDI::NUM_GPR) *(&(regs.rax) + regCtxIdx) = 0;
    }