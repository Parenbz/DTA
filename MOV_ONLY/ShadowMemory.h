#ifndef SHADOW_MEMORY_H
#define SHADOW_MEMORY_H
#include "QBDI.h"
#include <set>

class line {
public:
    size_t l, r, color;

    line(size_t L, size_t R, size_t COLOR)
        : l(L), r(R), color(COLOR)
    {}
};

struct cmp {
    bool operator() (const line &l1, const line &l2) {
        return l1.l < l2.l;
    }
};

class ShadowMemory {
private:

    std::set<line, cmp> regs, memory;

public:

    size_t checkByte(const size_t &address);
    size_t checkByteRange(const size_t &address, const size_t &size);
    size_t checkRegister(const size_t &regaddress, const size_t &size);
    void taintByte(const size_t &address, const size_t &color);
    void taintByteRange(const size_t &address, const size_t &size, const size_t &color);
    void taintRegister(const size_t &regaddress, const size_t &size, const size_t &color);
    void freeByte(const size_t &address);
    void freeByteRange(const size_t &address, const size_t &size);
    void freeRegister(const size_t &regaddress, const size_t &size);
};

#endif