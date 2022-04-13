#include <iostream>
#include <iomanip>
#include <assert.h>

#include "QBDI.h"

QBDI::VMAction showInstruction(QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data) {
    const QBDI::InstAnalysis *instAnalysis = vm->getInstAnalysis();

    std::cout << std::setbase(16) << instAnalysis->address << ":" 
              << instAnalysis->disassembly << std::endl << std::setbase(10);

    return QBDI::VMAction::CONTINUE;
}

int prod(int p, int x) {
    return p * x;
}

int sum(int a, int b) {
    return prod(3, a) + prod(5, b);
}

static const size_t STACK_SIZE = 0x100000;

int main(int argc, char **argv) {
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);

    bool res;
    uint32_t cid;

    uint8_t *fakestack;
    QBDI::GPRState *state;
    QBDI::rword retvalue;

    QBDI::VM *vm = new QBDI::VM;

    state = vm->getGPRState();
    assert(state != nullptr);

    res = QBDI::allocateVirtualStack(state, STACK_SIZE, &fakestack);
    assert(res);

    cid = vm->addCodeCB(QBDI::PREINST, showInstruction, nullptr);
    assert(cid != QBDI::INVALID_EVENTID);

    res = vm->addInstrumentedModuleFromAddr((QBDI::rword)sum);
    assert(res);

    std::cout << "Running function..." << std::endl;

    res = vm->call(&retvalue, (QBDI::rword)sum, {(QBDI::rword)a, (QBDI::rword)b});
    assert(res);

    std::cout << "result = " << retvalue << std::endl;

    QBDI::alignedFree(fakestack);
}