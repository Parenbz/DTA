#include <stdint.h>
#include "QBDI.h"
#include "ShadowMemory.h"

    ThirdMap::ThirdMap() {
        for (int i = 0; i < 8192; i++) {
            TM[i] = 0;
        }
    }

    ThirdMap::~ThirdMap() {}

    bool ThirdMap::checkByte(QBDI::rword tm, QBDI::rword dtm) {
        return TM[tm] & (1 << (7 - dtm));
    }

    void ThirdMap::markByte(QBDI::rword tm, QBDI::rword dtm) {
        TM[tm] |= 1 << (7 - dtm);
    }

    void ThirdMap::freeByte(QBDI::rword tm, QBDI::rword dtm) {
        TM[tm] &= ~(1 << (7 - dtm));
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

    bool SecondMap::checkByte(QBDI::rword sm, QBDI::rword tm, QBDI::rword dtm) {
        return SM[sm] == nullptr ? 0 : SM[sm]->checkByte(tm, dtm);
    }

    void SecondMap::markByte(QBDI::rword sm, QBDI::rword tm, QBDI::rword dtm) {
        if (SM[sm] == nullptr) SM[sm] = new ThirdMap;
        SM[sm]->markByte(tm, dtm);
    }

    void SecondMap::freeByte(QBDI::rword sm, QBDI::rword tm, QBDI::rword dtm) {
        if (SM[sm] == nullptr) return;
        SM[sm]->freeByte(tm, dtm);
    }

    ShadowMemory::ShadowMemory() {
        for (int i = 0; i < 65536; i++) {
            PM[i] = nullptr;
        }
        regs.eflags = 0;
        regs.fs = 0;
        regs.gs = 0;
        regs.r10 = 0;
        regs.r11 = 0;
        regs.r12 = 0;
        regs.r13 = 0;
        regs.r14 = 0;
        regs.r15 = 0;
        regs.r8 = 0;
        regs.r9 = 0;
        regs.rax = 0;
        regs.rbp = 0;
        regs.rbx = 0;
        regs.rcx = 0;
        regs.rdi = 0;
        regs.rdx = 0;
        regs.rip = 0;
        regs.rsi = 0;
        regs.rsp = 0;
    }

    ShadowMemory::~ShadowMemory() {
        for (int i = 0; i < 65536; i++) {
            if (PM[i] != nullptr) delete PM[i];
        }
    }

    bool ShadowMemory::checkByte(QBDI::rword address) {
        QBDI::rword pm = (address & ((QBDI::rword)65536 << 32)) >> 32;
        QBDI::rword sm = (address & ((QBDI::rword)65536 << 16)) >> 16;
        QBDI::rword tm = (address & ((QBDI::rword)8192 << 3)) >> 3;
        QBDI::rword dtm = address & (QBDI::rword)8;
        return PM[pm] == nullptr ? 0 : PM[pm]->checkByte(sm, tm, dtm);
    }

    bool ShadowMemory::checkByteRange(QBDI::rword address, QBDI::rword size) {
        bool res;
        for (int i = 0; i < size; i++) {
            res = checkByte(address + i);
            if (res) return 1;
        }
        return 0;
    }

    bool ShadowMemory::checkRegister(int16_t regCtxIdx) {
        switch (regCtxIdx) {
            case 0:
                return regs.rax;
                break;
            case 1:
                return regs.rbx;
                break;
            case 2:
                return regs.rcx;
                break;
            case 3:
                return regs.rdx;
                break;
            case 4:
                return regs.rsi;
                break;
            case 5:
                return regs.rdi;
                break;
            case 6:
                return regs.r8;
                break;
            case 7:
                return regs.r9;
                break;
            case 8:
                return regs.r10;
                break;
            case 9:
                return regs.r11;
                break;
            case 10:
                return regs.r12;
                break;
            case 11:
                return regs.r13;
                break;
            case 12:
                return regs.r14;
                break;
            case 13:
                return regs.r15;
                break;
            case 14:
                return regs.rbp;
                break;
            case 15:
                return regs.rsp;
                break;
            case 16:
                return regs.rip;
                break;
            case 17:
                return regs.eflags;
                break;
            case 18:
                return regs.fs;
                break;
            case 19:
                return regs.gs;
                break;
        }
        return 0;
    }

    void ShadowMemory::markByte(QBDI::rword address) {
        QBDI::rword pm = (address & ((QBDI::rword)65536 << 32)) >> 32;
        QBDI::rword sm = (address & ((QBDI::rword)65536 << 16)) >> 16;
        QBDI::rword tm = (address & ((QBDI::rword)8192 << 3)) >> 3;
        QBDI::rword dtm = address & (QBDI::rword)8;
        if (PM[pm] == nullptr) PM[pm] = new SecondMap;
        PM[pm]->markByte(sm, tm, dtm);
    }

    void ShadowMemory::markByteRange(QBDI::rword address, QBDI::rword size) {
        for (int i = 0; i < size; i++) {
            markByte(address + size);
        }
    }

    void ShadowMemory::markRegister(int16_t regCtxIdx) {
        switch (regCtxIdx) {
            case 0:
                regs.rax = 1;
                break;
            case 1:
                regs.rbx = 1;
                break;
            case 2:
                regs.rcx = 1;
                break;
            case 3:
                regs.rdx = 1;
                break;
            case 4:
                regs.rsi = 1;
                break;
            case 5:
                regs.rdi = 1;
                break;
            case 6:
                regs.r8 = 1;
                break;
            case 7:
                regs.r9 = 1;
                break;
            case 8:
                regs.r10 = 1;
                break;
            case 9:
                regs.r11 = 1;
                break;
            case 10:
                regs.r12 = 1;
                break;
            case 11:
                regs.r13 = 1;
                break;
            case 12:
                regs.r14 = 1;
                break;
            case 13:
                regs.r15 = 1;
                break;
            case 14:
                regs.rbp = 1;
                break;
            case 15:
                regs.rsp = 1;
                break;
            case 16:
                regs.rip = 1;
                break;
            case 17:
                regs.eflags = 1;
                break;
            case 18:
                regs.fs = 1;
                break;
            case 19:
                regs.gs = 1;
                break;
        }
    }

    void ShadowMemory::freeByte(QBDI::rword address) {
        QBDI::rword pm = (address & ((QBDI::rword)65536 << 32)) >> 32;
        QBDI::rword sm = (address & ((QBDI::rword)65536 << 16)) >> 16;
        QBDI::rword tm = (address & ((QBDI::rword)8192 << 3)) >> 3;
        QBDI::rword dtm = address & (QBDI::rword)8;
        if (PM[pm] == nullptr) return;
        PM[pm]->freeByte(sm, tm, dtm);
    }

    void ShadowMemory::freeByteRange(QBDI::rword address, QBDI::rword size) {
        for (int i = 0; i < size; i++) {
            freeByte(address + size);
        }
    }

    void ShadowMemory::freeRegister(int16_t regCtxIdx) {
        switch (regCtxIdx) {
            case 0:
                regs.rax = 0;
                break;
            case 1:
                regs.rbx = 0;
                break;
            case 2:
                regs.rcx = 0;
                break;
            case 3:
                regs.rdx = 0;
                break;
            case 4:
                regs.rsi = 0;
                break;
            case 5:
                regs.rdi = 0;
                break;
            case 6:
                regs.r8 = 0;
                break;
            case 7:
                regs.r9 = 0;
                break;
            case 8:
                regs.r10 = 0;
                break;
            case 9:
                regs.r11 = 0;
                break;
            case 10:
                regs.r12 = 0;
                break;
            case 11:
                regs.r13 = 0;
                break;
            case 12:
                regs.r14 = 0;
                break;
            case 13:
                regs.r15 = 0;
                break;
            case 14:
                regs.rbp = 0;
                break;
            case 15:
                regs.rsp = 0;
                break;
            case 16:
                regs.rip = 0;
                break;
            case 17:
                regs.eflags = 0;
                break;
            case 18:
                regs.fs = 0;
                break;
            case 19:
                regs.gs = 0;
                break;
        }
    }