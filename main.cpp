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

//s подаётся через rdi, n через rsi
void sink(uint8_t *s, size_t n) {
    free(s);
}

void func(size_t n) {
    uint8_t *tstr = source(n);

    uint8_t *x = (uint8_t *)malloc(1);

    x[0] = tstr[0];
    
    sink(tstr, n);
    sink(x, 1);
}

QBDI::VMAction showInstruction(QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data) {
    const QBDI::InstAnalysis *instAnalysis = vm->getInstAnalysis((QBDI::AnalysisType)7);

    std::cout << std::setbase(16) << instAnalysis->address << ": "
                << instAnalysis->disassembly << std::endl << std::setbase(10);

    return QBDI::VMAction::CONTINUE;
}

//Помечаем данные, выделенные в source
QBDI::VMAction taintSource(QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data) {
    const QBDI::InstAnalysis *instAnalysis = vm->getInstAnalysis((QBDI::AnalysisType)7);

    if (instAnalysis->isReturn) {
        for (int i = 0; i < gprState->rdi; i++) {
            sm->taintByte(gprState->rax + i, i + 1);
            std::cout << "Tainted byte " << std::setbase(16) << gprState->rax + i << std::setbase(10) 
                        << " with offset " << sm->checkByte(gprState->rax + i) - 1 << std::endl;
        }
    }

    return QBDI::VMAction::CONTINUE;
}

//Обновление Shadow Memory с каждой командой mov
QBDI::VMAction taintPropagation(QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data) {
    const QBDI::InstAnalysis *instAnalysis = vm->getInstAnalysis((QBDI::AnalysisType)7);

    if (instAnalysis->numOperands == 2) {
        if (sm->checkRegister(instAnalysis->operands[1].regCtxIdx)) {
            sm->taintRegister(instAnalysis->operands[0].regCtxIdx, sm->checkRegister(instAnalysis->operands[0].regCtxIdx));
            std::cout << "Taint register " << instAnalysis->operands[0].regName << " with offset " 
                        << sm->checkRegister(instAnalysis->operands[0].regCtxIdx) - 1 << std::endl;
        } else {
            sm->freeRegister(instAnalysis->operands[0].regCtxIdx);
            std::cout << "Freed register " << instAnalysis->operands[0].regName << std::endl;
        }
    } else if (instAnalysis->numOperands == 6) {
        if (instAnalysis->operands[1].type == QBDI::OPERAND_GPR) {
            if (sm->checkByteRange(gprState->rbp + instAnalysis->operands[4].value, instAnalysis->operands[0].size)) {
                sm->taintRegister(instAnalysis->operands[0].regCtxIdx, 
                                    sm->checkByteRange(gprState->rbp + instAnalysis->operands[4].value, instAnalysis->operands[0].size));
                std::cout << "Taint register " << instAnalysis->operands[0].regName << " with offset "
                            << sm->checkRegister(instAnalysis->operands[0].regCtxIdx) - 1<< std::endl;
            } else {
                sm->freeRegister(instAnalysis->operands[0].regCtxIdx);
                std::cout << "Freed resgister " << instAnalysis->operands[0].regName << std::endl;
            }
        } else {
            if (sm->checkRegister(instAnalysis->operands[5].regCtxIdx)) {
                sm->taintByteRange(gprState->rbp + instAnalysis->operands[3].value, instAnalysis->operands[5].size, 
                                    sm->checkRegister(instAnalysis->operands[5].regCtxIdx));
                std::cout << "Taint byte range" << std::setbase(16) << gprState->rbp + instAnalysis->operands[3].value << std::setbase(10) << " size of "
                            << (unsigned)instAnalysis->operands[5].size << " with offset "
                            << sm->checkByteRange(gprState->rbp + instAnalysis->operands[3].value, instAnalysis->operands[5].size) - 1 << std::endl;
            } else {
                sm->freeByteRange(gprState->rbp + instAnalysis->operands[3].value, instAnalysis->operands[5].size);
                std::cout << "Freed byte range " << std::setbase(16) << gprState->rbp + instAnalysis->operands[3].value << std::setbase(10) 
                            << " size of " << (unsigned)instAnalysis->operands[5].size << std::endl;
            }
        }
    }

    return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction taintSink(QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data) {
    QBDI::rword address = gprState->rdi;
    size_t size = gprState->rsi;

    for (size_t i = 0; i < size; i++) {
        if (sm->checkByte(address + i)) {
            std::cout << "Caught byte " << std::setbase(16) << address + i << std::setbase(10) 
                        << " with offset " << sm->checkByte(address + i) - 1 << std::endl;
        }
    }

    return QBDI::VMAction::CONTINUE;
}

const static size_t STACK_SIZE = 0x100000;

int main(int argc, char **argv) {

    size_t n = atoi(argv[1]);

    uint8_t *fakestack;
    QBDI::GPRState *state;

    QBDI::VM *vm = new QBDI::VM;
    state = vm->getGPRState();
    QBDI::allocateVirtualStack(state, STACK_SIZE, &fakestack);
    vm->addCodeCB(QBDI::POSTINST, showInstruction, nullptr);
    vm->addMnemonicCB("mov*", QBDI::PREINST, taintPropagation, nullptr);
    vm->addCodeRangeCB((QBDI::rword)source, (QBDI::rword)sink, QBDI::PREINST, taintSource, nullptr);
    vm->addCodeAddrCB((QBDI::rword)sink, QBDI::PREINST, taintSink, nullptr);
    vm->addInstrumentedModuleFromAddr((QBDI::rword)func);
    vm->call(nullptr, (QBDI::rword)func, {(QBDI::rword)n});
    QBDI::alignedFree(fakestack);
    delete sm;
}