#include <stdint.h>

class ThirdMap {
private:
    uint8_t TM[8192];

public:
    ThirdMap() {
        for (int i = 0; i < 8192; i++) {
            TM[i] = 0;
        }
    }

    ~ThirdMap() {}

    bool checkByte(uint64_t tm, uint64_t dtm) {
        return TM[tm] & (1 << (7 - dtm));
    }

    void markByte(uint64_t tm, uint64_t dtm) {
        TM[tm] |= 1 << (7 - dtm);
    }

    void freeByte(uint64_t tm, uint64_t dtm) {
        TM[tm] &= ~(1 << (7 - dtm));
    }
};

class SecondMap {
private:
    ThirdMap* SM[65536];

public:
    SecondMap() {
        for (int i = 0; i < 65536; i++) {
            SM[i] = nullptr;
        }
    }

    ~SecondMap() {
        for (int i = 0; i < 65536; i++) {
            if (SM[i] != nullptr) delete SM[i];
        }
    }

    bool checkByte(uint64_t sm, uint64_t tm, uint64_t dtm) {
        return SM[sm] == nullptr ? 0 : SM[sm]->checkByte(tm, dtm);
    }

    void markByte(uint64_t sm, uint64_t tm, uint64_t dtm) {
        if (SM[sm] == nullptr) SM[sm] = new ThirdMap;
        SM[sm]->markByte(tm, dtm);
    }

    void freeByte(uint64_t sm, uint64_t tm, uint64_t dtm) {
        if (SM[sm] == nullptr) return;
        SM[sm]->freeByte(tm, dtm);
    }
};

class ShadowMemory {
private:
    SecondMap* PM[65536];

public:
    ShadowMemory() {
        for (int i = 0; i < 65536; i++) {
            PM[i] = nullptr;
        }
    }

    ~ShadowMemory() {
        for (int i = 0; i < 65536; i++) {
            if (PM[i] != nullptr) delete PM[i];
        }
    }

    bool checkByte(uint64_t address) {
        uint64_t pm = (address & ((uint64_t)65536 << 32)) >> 32;
        uint64_t sm = (address & ((uint64_t)65536 << 16)) >> 16;
        uint64_t tm = (address & ((uint64_t)8192 << 3)) >> 3;
        uint64_t dtm = address & (uint64_t)8;
        return PM[pm] == nullptr ? 0 : PM[pm]->checkByte(sm, tm, dtm);
    }

    bool checkByteRange(uint64_t address, uint32_t size) {
        bool res;
        for (int i = 0; i < size; i++) {
            res = checkByte(address + i);
            if (res) return 1;
        }
        return 0;
    }

    void markByte(uint64_t address) {
        uint64_t pm = (address & ((uint64_t)65536 << 32)) >> 32;
        uint64_t sm = (address & ((uint64_t)65536 << 16)) >> 16;
        uint64_t tm = (address & ((uint64_t)8192 << 3)) >> 3;
        uint64_t dtm = address & (uint64_t)8;
        if (PM[pm] == nullptr) PM[pm] = new SecondMap;
        PM[pm]->markByte(sm, tm, dtm);
    }

    void markByteRange(uint64_t address, uint32_t size) {
        for (int i = 0; i < size; i++) {
            markByte(address + size);
        }
    }

    void freeByte(uint64_t address) {
        uint64_t pm = (address & ((uint64_t)65536 << 32)) >> 32;
        uint64_t sm = (address & ((uint64_t)65536 << 16)) >> 16;
        uint64_t tm = (address & ((uint64_t)8192 << 3)) >> 3;
        uint64_t dtm = address & (uint64_t)8;
        if (PM[pm] == nullptr) return;
        PM[pm]->freeByte(sm, tm, dtm);
    }

    void freeByteRange(uint64_t address, uint32_t size) {
        for (int i = 0; i < size; i++) {
            freeByte(address + size);
        }
    }
};