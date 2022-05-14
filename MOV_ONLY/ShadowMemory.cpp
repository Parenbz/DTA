#include <stdint.h>
#include "QBDI.h"
#include "ShadowMemory.h"
#include <set>
#include <iterator>

size_t ShadowMemory::checkByteRange(const size_t &address, const size_t &size) {
    size_t l = address;
    size_t r = address + size - 1;
    auto lb = memory.lower_bound(line(l, r, 0));
    if (lb != memory.begin()) {
        lb--;
        if (lb->r >= l) return lb->color;
        lb++;
    }
    if (lb != memory.end()) {
        if (lb->l <= r) return lb->color;
    }
    return 0;
}

size_t ShadowMemory::checkByte(const size_t &address){
    return checkByteRange(address, 1);
}

size_t ShadowMemory::checkRegister(const size_t &regaddress, const size_t &size) {
    size_t l = regaddress;
    size_t r = regaddress + size - 1;
    auto lb = regs.lower_bound(line(l, r, 0));
    if (lb != regs.begin()) {
        lb--;
        if (lb->r >= l) return lb->color;
        lb++;
    }
    if (lb != regs.end()) {
        if (lb->l <= r) return lb->color;
    }
    return 0;
}

void ShadowMemory::taintByteRange(const size_t &address, const size_t &size, const size_t &color) {
    size_t l = address;
    size_t r = address + size - 1;
    auto lb = memory.lower_bound(line(l, r, 0));
    if (lb != memory.begin()) lb--;
    if (lb == memory.end()) {
        memory.insert(line(l, r, color));
        return;
    }
    auto ub = lb;
    while (ub != memory.end() && ub->r <= r) {
        ub++;
    }
    if (lb->r < l) {
        if (ub == memory.end() || (ub != memory.end() && ub->l > r)) {
            memory.erase(++lb, ub);
            memory.insert(line(l, r, color));
        } else {
            size_t c = ub->color;
            size_t pr = ub->r;
            memory.erase(++lb, ++ub);
            memory.insert(line(l, r, color));
            memory.insert(line(r + 1, pr, c));
        }
    } else if (lb->l < l) {
        if (ub == memory.end() || (ub != memory.end() && ub->l > r)) {
            size_t c = lb->color;
            size_t pl = lb->l;
            memory.erase(lb, ub);
            memory.insert(line(pl, l - 1, c));
            memory.insert(line(l, r, color));
        } else {
            size_t lc = lb->color;
            size_t pl = lb->l;
            size_t rc = ub->color;
            size_t pr = ub->r;
            memory.erase(lb, ++ub);
            memory.insert(line(pl, l - 1, lc));
            memory.insert(line(l, r, color));
            memory.insert(line(r + 1, pr, rc));
        }
    } else {
        if (ub == memory.end() || (ub != memory.end() && ub->l > r)) {
            memory.erase(lb, ub);
            memory.insert(line(l, r, color));
        } else {
            size_t c = ub->color;
            size_t pr = ub->r;
            memory.erase(lb, ++ub);
            memory.insert(line(l, r, color));
            memory.insert(line(r + 1, pr, c));
        }
    }
}

void ShadowMemory::taintByte(const size_t &address, const size_t &color) {
    taintByteRange(address, 1, color);
}

void ShadowMemory::taintRegister(const size_t &regaddress, const size_t &size, const size_t &color) {
    size_t l = regaddress;
    size_t r = regaddress + size - 1;
    auto lb = regs.lower_bound(line(l, r, 0));
    if (lb != regs.begin()) lb--;
    if (lb == regs.end()) {
        regs.insert(line(l, r, color));
        return;
    }
    auto ub = lb;
    while (ub != regs.end() && ub->r <= r) {
        ub++;
    }
    if (lb->r < l) {
        if (ub == regs.end() || (ub != regs.end() && ub->l > r)) {
            regs.erase(++lb, ub);
            regs.insert(line(l, r, color));
        } else {
            size_t c = ub->color;
            size_t pr = ub->r;
            regs.erase(++lb, ++ub);
            regs.insert(line(l, r, color));
            regs.insert(line(r + 1, pr, c));
        }
    } else if (lb->l < l) {
        if (ub == regs.end() || (ub != regs.end() && ub->l > r)) {
            size_t c = lb->color;
            size_t pl = lb->l;
            regs.erase(lb, ub);
            regs.insert(line(pl, l - 1, c));
            regs.insert(line(l, r, color));
        } else {
            size_t lc = lb->color;
            size_t pl = lb->l;
            size_t rc = ub->color;
            size_t pr = ub->r;
            regs.erase(lb, ++ub);
            regs.insert(line(pl, l - 1, lc));
            regs.insert(line(l, r, color));
            regs.insert(line(r + 1, pr, rc));
        }
    } else {
        if (ub == regs.end() || (ub != regs.end() && ub->l > r)) {
            regs.erase(lb, ub);
            regs.insert(line(l, r, color));
        } else {
            size_t c = ub->color;
            size_t pr = ub->r;
            regs.erase(lb, ++ub);
            regs.insert(line(l, r, color));
            regs.insert(line(r + 1, pr, c));
        }
    }
}

void ShadowMemory::freeByteRange(const size_t &address, const size_t &size) {
    size_t l = address;
    size_t r = address + size - 1;
    auto lb = memory.lower_bound(line(l, r, 0));
    if (lb != memory.begin()) lb--;
    if (lb == memory.end()) return;
    auto ub = lb;
    while (ub != memory.end() && ub->r <= r) {
        ub++;
    }
    if (lb->r < l) {
        if (ub == memory.end() || (ub != memory.end() && ub->l > r)) {
            memory.erase(++lb, ub);
        } else {
            size_t c = ub->color;
            size_t pr = ub->r;
            memory.erase(++lb, ++ub);
            memory.insert(line(r + 1, pr, c));
        }
    } else if (lb->l < l) {
        if (ub == memory.end() || (ub != memory.end() && ub->l > r)) {
            size_t c = lb->color;
            size_t pl = lb->l;
            memory.erase(lb, ub);
            memory.insert(line(pl, l - 1, c));
        } else {
            size_t lc = lb->color;
            size_t pl = lb->l;
            size_t rc = ub->color;
            size_t pr = ub->r;
            memory.erase(lb, ++ub);
            memory.insert(line(pl, l - 1, lc));
            memory.insert(line(r + 1, pr, rc));
        }
    } else {
        if (ub == memory.end() || (ub != memory.end() && ub->l > r)) {
            memory.erase(lb, ub);
        } else {
            size_t c = ub->color;
            size_t pr = ub->r;
            memory.erase(lb, ++ub);
            memory.insert(line(r + 1, pr, c));
        }
    }
}

void ShadowMemory::freeByte(const size_t &address) {
    freeByteRange(address, 1);
}

void ShadowMemory::freeRegister(const size_t &regaddress, const size_t &size) {
    size_t l = regaddress;
    size_t r = regaddress + size - 1;
    auto lb = regs.lower_bound(line(l, r, 0));
    if (lb != regs.begin()) lb--;
    if (lb == regs.end()) return;
    auto ub = lb;
    while (ub != regs.end() && ub->r <= r) {
        ub++;
    }
    if (lb->r < l) {
        if (ub == regs.end() || (ub != regs.end() && ub->l > r)) {
            regs.erase(++lb, ub);
        } else {
            size_t c = ub->color;
            size_t pr = ub->r;
            regs.erase(++lb, ++ub);
            regs.insert(line(r + 1, pr, c));
        }
    } else if (lb->l < l) {
        if (ub == regs.end() || (ub != regs.end() && ub->l > r)) {
            size_t c = lb->color;
            size_t pl = lb->l;
            regs.erase(lb, ub);
            regs.insert(line(pl, l - 1, c));
        } else {
            size_t lc = lb->color;
            size_t pl = lb->l;
            size_t rc = ub->color;
            size_t pr = ub->r;
            regs.erase(lb, ++ub);
            regs.insert(line(pl, l - 1, lc));
            regs.insert(line(r + 1, pr, rc));
        }
    } else {
        if (ub == regs.end() || (ub != regs.end() && ub->l > r)) {
            regs.erase(lb, ub);
        } else {
            size_t c = ub->color;
            size_t pr = ub->r;
            regs.erase(lb, ++ub);
            regs.insert(line(r + 1, pr, c));
        }
    }
}