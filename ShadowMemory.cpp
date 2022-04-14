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