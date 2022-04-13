#include <iostream>
#include <stdlib.h>
#include <ctime>
#include <iomanip>

#include "QBDI.h"

uint8_t *source(int n) {
    return (uint8_t *)malloc(n);
}

void sink(uint8_t *s, int n) {
    free(s);
}

void func(int n) {
    uint8_t *tstr = source(n);
    uint8_t *str = (uint8_t *)malloc(n);
    uint8_t *rd = (uint8_t *)malloc(n);

    for (int i = 0; i < n; i++) {
        if (rand() % 2) rd[i] = tstr[i]; else rd[i] = str[i];
    }

    sink(rd, n);

    free(tstr);
    free(str);
}

QBDI::VMAction showInstruction(QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data) {
    const QBDI::InstAnalysis *instAnalysis = vm->getInstAnalysis((QBDI::AnalysisType)7);

    std::cout << std::setbase(16) << instAnalysis->address << ": "
                << instAnalysis->disassembly << std::endl << std::setbase(10);
    
    if (instAnalysis->isCall) {
        std::cout << std::setbase(16) << instAnalysis->operands[0].value + instAnalysis->address << std::endl << std::setbase(10);
    }

    return QBDI::VMAction::CONTINUE;
}

const static size_t STACK_SIZE = 0x100000;

int main(int argc, char **argv) {
    srand(time(0));

    int n = atoi(argv[1]);

    uint8_t *fakestack;
    QBDI::GPRState *state;

    QBDI::VM *vm = new QBDI::VM;
    state = vm->getGPRState();
    QBDI::allocateVirtualStack(state, STACK_SIZE, &fakestack);
    vm->addCodeCB(QBDI::PREINST, showInstruction, nullptr);
    vm->addInstrumentedModuleFromAddr((QBDI::rword)func);
    vm->call(nullptr, (QBDI::rword)func, {(QBDI::rword)n});
    QBDI::alignedFree(fakestack);
}