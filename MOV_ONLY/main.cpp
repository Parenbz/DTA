#include <iostream>
#include <stdlib.h>
#include <ctime>
#include <iomanip>
#include <stdio.h>

#include "QBDI.h"
#include "ShadowMemory.h"

namespace{
    ShadowMemory sm;
}

uint8_t *source(int size) {
    return (uint8_t *)calloc(size, sizeof(uint8_t));
}

void sink(uint8_t b) {
}

char globalbuf[10];

void testSink() {
    unsigned int tmp;
    uint8_t *buf = source(10);
    uint8_t localbuf[10];

    sink(buf[0]);
    sink(localbuf[0]);

    localbuf[1] = buf[1];
    sink(localbuf[1]);

    localbuf[2] = buf[2];
    localbuf[3] = localbuf[2];
    localbuf[2] = 'a';
    sink(localbuf[3]);
    sink(localbuf[2]);

    globalbuf[0] = buf[3];
    sink(globalbuf[0]);

    globalbuf[1] = buf[4];
    localbuf[0] = globalbuf[1];
    tmp = localbuf[0];
    sink(globalbuf[2]);
    sink(tmp & 0xff);
    sink(tmp >> 8 & 0xff);

    free(buf);
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
            sm.taintByte(address + i, i + 1);
        }
    }

    return QBDI::VMAction::CONTINUE;
}

//Проверка байта, попавшего в sink
QBDI::VMAction lookSink(QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data) {
    if (sm.checkRegister(40, 4)) {
        std::cout << "Caught byte with offset " << sm.checkRegister(40, 4) - 1 << std::endl; 
    }

    return QBDI::VMAction::CONTINUE;
}

//Обновление Shadow Memory с каждой командой mov
QBDI::VMAction taintPropagation(QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data) {
    const QBDI::InstAnalysis *instAnalysis = vm->getInstAnalysis((QBDI::AnalysisType)7);

    char op1, op2;
    int szl, szr;
    int i = 0;
    while (instAnalysis->mnemonic[i] != '8' && instAnalysis->mnemonic[i] != '1' &&
           instAnalysis->mnemonic[i] != '3' && instAnalysis->mnemonic[i] != '6') {
        i++;
    }
    if (instAnalysis->mnemonic[i] == '8') {
        szl = 1;
        szr = 1;
        i++;
    } else if (instAnalysis->mnemonic[i] == '1') {
        szl = 2;
        szr = 2;
        i += 2;
    } else if (instAnalysis->mnemonic[i] == '3') {
        szl = 4;
        szr = 4;
        i += 2;
    } else if (instAnalysis->mnemonic[i] == '6') {
        szl = 8;
        szr = 8;
        i += 2;
    }
    op1 = instAnalysis->mnemonic[i++];
    op2 = instAnalysis->mnemonic[i++];
    if (instAnalysis->mnemonic[i] == '8') szr = 1;
    else if (instAnalysis->mnemonic[i] == '1') szr = 2;
    else if (instAnalysis->mnemonic[i] == '3') szr = 4;
    else if (instAnalysis->mnemonic[i] == '6') szr = 8;
        
    //mov регистр, регистр
    if (op1 == 'r' && op2 == 'r') {
        if (sm.checkRegister(instAnalysis->operands[1].regCtxIdx * 8 + instAnalysis->operands[1].regOff, szr)) {
            sm.taintRegister(instAnalysis->operands[0].regCtxIdx * 8 + instAnalysis->operands[0].regOff, szl, 
                             sm.checkRegister(instAnalysis->operands[1].regCtxIdx * 8 + instAnalysis->operands[1].regOff, szr));
        } else {
            sm.freeRegister(instAnalysis->operands[0].regCtxIdx * 8 + instAnalysis->operands[0].regOff, szl);
        }
    }

    //mov регистр, константа
    if (op1 == 'r' && op2 == 'i') {
        sm.freeRegister(instAnalysis->operands[0].regCtxIdx * 8 + instAnalysis->operands[0].regOff, szl);
    }

    //mov регистр, адрес
    if (op1 == 'r' && op2 == 'm') {
        size_t address = *(&(gprState->rax) + instAnalysis->operands[1].regCtxIdx) + instAnalysis->operands[4].value;
        if (instAnalysis->operands[1].regCtxIdx == 16) address++;
        if (sm.checkByteRange(address, szr)) {
            sm.taintRegister(instAnalysis->operands[0].regCtxIdx * 8 + instAnalysis->operands[0].regOff, szl, 
                             sm.checkByteRange(address, szr));
        } else {
            sm.freeRegister(instAnalysis->operands[0].regCtxIdx * 8 + instAnalysis->operands[0].regOff, szl);
        }
    }

    //mov адрес, регистр
    if (op1 == 'm' && op2 == 'r') {
        size_t address = *(&(gprState->rax) + instAnalysis->operands[0].regCtxIdx) + instAnalysis->operands[3].value;
        if (sm.checkRegister(instAnalysis->operands[5].regCtxIdx * 8 + instAnalysis->operands[5].regOff, szr)) {
            sm.taintByteRange(address, szl, sm.checkRegister(instAnalysis->operands[5].regCtxIdx * 8 + instAnalysis->operands[5].regOff, szr));
        } else {
            sm.freeByteRange(address, szl);
        }
    }

    //mov адрес, константа
    if (op1 == 'm' && op2 == 'i') {
        size_t address = *(&(gprState->rax) + instAnalysis->operands[0].regCtxIdx) + instAnalysis->operands[3].value;
        sm.freeByteRange(address, szl);
    }

    return QBDI::VMAction::CONTINUE;
}

const static size_t STACK_SIZE = 0x100000;

int main(int argc, char **argv) {
    srand(time(0));

    size_t n = argc > 1 ? atoi(argv[1]) : 1;

    uint8_t *fakestack;
    QBDI::GPRState *state;

    size_t sourceSize;

    QBDI::VM *vm = new QBDI::VM;
    state = vm->getGPRState();
    QBDI::allocateVirtualStack(state, STACK_SIZE, &fakestack);
    vm->addCodeAddrCB((QBDI::rword)sink, QBDI::PREINST, lookSink, nullptr);
    vm->addMnemonicCB("mov*", QBDI::PREINST, taintPropagation, nullptr);
    vm->addCodeAddrCB((QBDI::rword)source, QBDI::PREINST, getSourceSize, &sourceSize);
    vm->addCodeRangeCB((QBDI::rword)source, (QBDI::rword)sink, QBDI::PREINST, taintSource, &sourceSize);
    vm->addInstrumentedModuleFromAddr((QBDI::rword)testSink);
    vm->call(nullptr, (QBDI::rword)testSink);
    QBDI::alignedFree(fakestack);
    delete vm;
}
