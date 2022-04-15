#include <iostream>
#include <stdlib.h>
#include <ctime>
#include <iomanip>
#include <stdio.h>

#include "QBDI.h"
#include "ShadowMemory.h"

ShadowMemory* sm = new ShadowMemory;

uint8_t *source(int n) {
    return (uint8_t *)malloc(n);
}

void sink(uint8_t *s, size_t n) {
    for (int i = 0; i < n; i++) {
        if (sm->checkByte((size_t)s + i)) {
            std::cout << "Caught byte " << std::setbase(16) << (size_t)s + i << std::setbase(10) 
                        << " with offset " << sm->checkByte((size_t)s + i) - 1 << std::endl;
        }
    }
    free(s);
}

void func(size_t n) {
    uint8_t *tstr = source(n);
    uint8_t *str = (uint8_t *)malloc(n);

    for (int i = 0; i < n; i++) {
        if (rand() % 2) {
            std::cout << "Should catch byte with offset " << i << std::endl;
            str[i] = tstr[i];
        }
    }

    sink(str, n);

    free(tstr);
}

//Получить размер массива, выделенного в source
QBDI::VMAction getSourceSize(QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data) {
    *(size_t *)data = gprState->rdi;

    return QBDI::VMAction::CONTINUE;
}

//Пометить данные, выделенные в source
QBDI::VMAction taintSource(QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data) {
    const QBDI::InstAnalysis *instAnalysis = vm->getInstAnalysis((QBDI::AnalysisType)7);

    if (instAnalysis->isReturn) {
        size_t size = *(size_t *)data;
        size_t address = gprState->rax;
        for (int i = 0; i < size; i++) {
            sm->taintByte(address + i, i + 1);
        }
    }

    return QBDI::VMAction::CONTINUE;
}

//Обновление Shadow Memory с каждой командой mov
QBDI::VMAction taintPropagation(QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data) {
    const QBDI::InstAnalysis *instAnalysis = vm->getInstAnalysis((QBDI::AnalysisType)7);

    if (instAnalysis->numOperands == 2) {
        
        //mov регистр, регистр
        if (instAnalysis->operands[1].type == QBDI::OPERAND_GPR) {
            if (sm->checkRegister(instAnalysis->operands[1].regCtxIdx)) {
                sm->taintRegister(instAnalysis->operands[0].regCtxIdx, sm->checkRegister(instAnalysis->operands[0].regCtxIdx));
            } else {
                sm->freeRegister(instAnalysis->operands[0].regCtxIdx);
            }

        //mov регистр, константа
        } else {
            sm->freeRegister(instAnalysis->operands[0].regCtxIdx);
        }
        
    } else if (instAnalysis->numOperands == 6) {

        //mov регистр, адрес
        if (instAnalysis->operands[1].type == QBDI::OPERAND_GPR) {
            size_t address = *(&(gprState->rax) + instAnalysis->operands[1].regCtxIdx) + instAnalysis->operands[4].value;
            if (sm->checkByteRange(address, instAnalysis->operands[0].size)) {
                sm->taintRegister(instAnalysis->operands[0].regCtxIdx, sm->checkByteRange(address, instAnalysis->operands[0].size));
            } else {
                sm->freeRegister(instAnalysis->operands[0].regCtxIdx);
            }

        //mov адрес, регистр
        } else {
            size_t address = *(&(gprState->rax) + instAnalysis->operands[0].regCtxIdx) + instAnalysis->operands[3].value;
            if (sm->checkRegister(instAnalysis->operands[5].regCtxIdx)) {
                sm->taintByteRange(address, instAnalysis->operands[5].size, sm->checkRegister(instAnalysis->operands[5].regCtxIdx));
            } else {
                sm->freeByteRange(address, instAnalysis->operands[5].size);
            }
        }
    }

    return QBDI::VMAction::CONTINUE;
}

const static size_t STACK_SIZE = 0x100000;

int main(int argc, char **argv) {
    srand(time(0));

    size_t n = atoi(argv[1]);

    uint8_t *fakestack;
    QBDI::GPRState *state;

    size_t *sourceSize = new size_t;

    QBDI::VM *vm = new QBDI::VM;
    state = vm->getGPRState();
    QBDI::allocateVirtualStack(state, STACK_SIZE, &fakestack);
    vm->addMnemonicCB("mov*", QBDI::PREINST, taintPropagation, nullptr);
    vm->addCodeAddrCB((QBDI::rword)source, QBDI::PREINST, getSourceSize, sourceSize);
    vm->addCodeRangeCB((QBDI::rword)source, (QBDI::rword)sink, QBDI::PREINST, taintSource, sourceSize);
    vm->addInstrumentedModuleFromAddr((QBDI::rword)func);
    vm->call(nullptr, (QBDI::rword)func, {(QBDI::rword)n});
    QBDI::alignedFree(fakestack);
    delete sm;
    delete sourceSize;
}