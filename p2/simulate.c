/* FSM for LC */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000
 
typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int memoryAddress;
    int memoryData;
    int instrReg;
    int aluOperand;
    int aluResult;
    int numMemory;
} stateType;
 
void printState(stateType *, char *);
void run(stateType);
int memoryAccess(stateType *, int);
int convertNum(int);
 
int main(int argc, char *argv[]) {
    int i;
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
 
    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }
 
    /* initialize memories and registers */
    for (i=0; i<NUMMEMORY; i++) {
        state.mem[i] = 0;
    }
    for (i=0; i<NUMREGS; i++) {
        state.reg[i] = 0;
    }
 
    state.pc=0;
 
    /* read machine-code file into instruction/data memory (starting at
        address 0) */
 
    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s\n", argv[1]);
        perror("fopen");
        exit(1);
    }
 
    for (state.numMemory=0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
        state.numMemory++) {
        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }
 
    printf("\n");
 
    /* run never returns */
    run(state);
 
    return 0;
}
 
void printState(stateType *statePtr, char *stateName) {
    int i;
    static int cycle = 0;
    printf("\n@@@\nstate %s (cycle %d)\n", stateName, cycle++);
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
        for (i=0; i<statePtr->numMemory; i++) {
            printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
        }
    printf("\tregisters:\n");
        for (i=0; i<NUMREGS; i++) {
            printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
        }
    printf("\tinternal registers:\n");
    printf("\t\tmemoryAddress %d\n", statePtr->memoryAddress);
    printf("\t\tmemoryData %d\n", statePtr->memoryData);
    printf("\t\tinstrReg %d\n", statePtr->instrReg);
    printf("\t\taluOperand %d\n", statePtr->aluOperand);
    printf("\t\taluResult %d\n", statePtr->aluResult);
}
 
/*
 * Access memory:
 *     readFlag=1 ==> read from memory
 *     readFlag=0 ==> write to memory
 * Return 1 if the memory operation was successful, otherwise return 0
 */
int memoryAccess(stateType *statePtr, int readFlag) {
    static int lastAddress = -1;
    static int lastReadFlag = 0;
    static int lastData = 0;
    static int delay = 0;
 
    if (statePtr->memoryAddress < 0 || statePtr->memoryAddress >= NUMMEMORY) {
        printf("memory address out of range\n");
        exit(1);
    }
 
    /*
     * If this is a new access, reset the delay clock.
     */
    if ( (statePtr->memoryAddress != lastAddress) ||
             (readFlag != lastReadFlag) ||
             (readFlag == 0 && lastData != statePtr->memoryData) ) {
        delay = statePtr->memoryAddress % 3;
        lastAddress = statePtr->memoryAddress;
        lastReadFlag = readFlag;
        lastData = statePtr->memoryData;
    }
 
    if (delay == 0) {
        /* memory is ready */
        if (readFlag) {
            statePtr->memoryData = statePtr->mem[statePtr->memoryAddress];
        } else {
            statePtr->mem[statePtr->memoryAddress] = statePtr->memoryData;
        }
        return 1;
    } else {
        /* memory is not ready */
        delay--;
        return 0;
    }
}
 
int convertNum(int num) {
    /* convert a 16-bit number into a 32-bit integer */
    if (num & (1 << 15) ) {
        num -= (1 << 16);
    }
    return num;
}
 
void run(stateType state) {
    int bus;
    int memAddressResult;

    /* FINAL */
    fetch:
        printState(&state, "fetch");
        bus = state.pc;
        state.memoryAddress = bus;
        goto check;

    /* FINAL */
    check:
        printState(&state, "check");
        memAddressResult = memoryAccess(&state, 1);
        if (memAddressResult == 0)
            goto check;
        goto instruction;

    /* FINAL */
    instruction:
        printState(&state, "instruction");
        bus = state.memoryData;
        state.instrReg = bus;
        goto ldRegA;

    /* FINAL */
    ldRegA:
        printState(&state, "ldRegA");
        bus = state.reg[(state.instrReg >> 19) & 0x7];
        state.aluOperand = bus;
        state.pc++;
        if ((state.instrReg >> 22)==0)
            goto ALUadd;
        else if ((state.instrReg >> 22)==1)
            goto ALUnand;
        else if ((state.instrReg >> 22)==2 || (state.instrReg >> 22)==3)
            goto calcOffset;
        else if ((state.instrReg >> 22)==4)
            goto ALUbeq;
        else if ((state.instrReg >> 22)==5)
            goto ALUjalr;
        else if ((state.instrReg >> 22)==6)
            goto ALUhalt;
        else
            goto fetch;

    /* FINAL */
    ALUadd:
        printState(&state, "ALUadd");
        bus = state.reg[(state.instrReg >> 16) & 0x7];
        state.aluResult = state.aluOperand + bus;
        goto ldDest;

    /* FINAL */
    ALUnand:
        printState(&state, "ALUadd");
        bus = state.reg[(state.instrReg >> 16) & 0x7];
        state.aluResult = ~(state.aluOperand & bus);
        goto ldDest;

    /* FINAL */
    ldDest:
        printState(&state, "ldDest");
        bus = state.aluResult;
        state.reg[state.instrReg & 0x7] = bus;
        goto fetch;

    /* FINAL */
    ALUbeq:
        printState(&state, "ALUbeq");
        bus = state.reg[(state.instrReg >> 16) & 0x7];
        state.aluResult = bus - state.aluOperand;
        goto ALUbeq2;

    /* FINAL */
    ALUbeq2:
        printState(&state, "ALUbeq2");
        if (state.aluResult != 0)
            goto fetch;
        bus = state.pc;
        state.aluOperand = bus;
        goto calcOffset;

    /* FINAL */
    ALUbeq3:
        printState(&state, "ALUbeq3");
        bus = state.aluResult;
        state.pc = bus;
        goto fetch;

    /* FINAL */
    calcOffset:
        printState(&state, "calcOffset");
        bus = convertNum(state.instrReg & 0xFFFF);
        state.aluResult = state.aluOperand + bus;
        if ((state.instrReg >> 22)==2)
            goto ALUlw;
        else if ((state.instrReg >> 22)==3)
            goto ALUsw;
        else if ((state.instrReg >> 22)==4)
            goto ALUbeq3;

    /* FINAL */
    ALUlw:
        printState(&state, "ALUlw");
        bus = state.aluResult;
        state.memoryAddress = bus;
        goto ALUlw2;

    /* FINAL */
    ALUlw2:
        printState(&state, "ALUlw2");
        memAddressResult = memoryAccess(&state, 1);
        if (memAddressResult == 0)
            goto ALUlw2;
        goto ALUlw3;

    /* FINAL */
    ALUlw3:
        printState(&state, "ALUlw3");
        bus = state.memoryData;
        state.reg[((state.instrReg << 13) >> 29) & 0x7] = bus;
        goto fetch;

    /* FINAL */
    ALUsw:
        printState(&state, "ALUsw");
        bus = state.aluResult;
        state.memoryAddress = bus;
        goto ALUsw2;

    /* FINAL */
    ALUsw2:
        printState(&state, "ALUsw2");
        bus = state.reg[((state.instrReg << 13) >> 29) & 0x7];
        state.memoryData = bus;
        goto ALUsw3;

    /* FINAL */
    ALUsw3:
        printState(&state, "ALUsw3");
        memAddressResult = memoryAccess(&state, 0);
        if (memAddressResult == 0)
            goto ALUsw3;
        goto fetch;

    /* FINAL */
    ALUjalr:
        printState(&state, "ALUjalr");
        bus = state.pc;
        state.reg[(state.instrReg >> 16) & 0x7] = bus;
        goto ALUjalr2;

    /* FINAL */
    ALUjalr2:
        printState(&state, "ALUjalr2");
        bus = state.reg[(state.instrReg >> 19) & 0x7];
        state.pc = bus;
        goto fetch;

    /* FINAL */
    ALUhalt:
        printState(&state, "ALUhalt");
        exit(0);

}