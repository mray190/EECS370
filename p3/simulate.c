#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR will not implemented for Project 3 */
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDStruct {
	int instr;
	int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
	int instr;
	int pcPlus1;
	int readRegA;
	int readRegB;
	int offset;
} IDEXType;

typedef struct EXMEMStruct {
	int instr;
	int branchTarget;
	int aluResult;
	int readRegB;
} EXMEMType;

typedef struct MEMWBStruct {
	int instr;
	int writeData;
} MEMWBType;

typedef struct WBENDStruct {
	int instr;
	int writeData;
} WBENDType;

typedef struct stateStruct {
	int pc;
	int instrMem[NUMMEMORY];
	int dataMem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
	int cycles; /* number of cycles run so far */
} stateType;

void printInstruction(int instr);
void run();

void printState(stateType *statePtr) {
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles);
    printf("\tpc %d\n", statePtr->pc);

    printf("\tdata memory:\n");
	for (i=0; i<statePtr->numMemory; i++) {
	    printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
	}
    printf("\tregisters:\n");
	for (i=0; i<NUMREGS; i++) {
	    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	}
    printf("\tIFID:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->IFID.instr);
	printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);
    printf("\tIDEX:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->IDEX.instr);
	printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1);
	printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
	printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
	printf("\t\toffset %d\n", statePtr->IDEX.offset);
    printf("\tEXMEM:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->EXMEM.instr);
	printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget);
	printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult);
	printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);
    printf("\tMEMWB:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->MEMWB.instr);
	printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);
    printf("\tWBEND:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->WBEND.instr);
	printf("\t\twriteData %d\n", statePtr->WBEND.writeData);
}

int field0(int instruction) {
	return( (instruction>>19) & 0x7);
}

int field1(int instruction) {
	return( (instruction>>16) & 0x7);
}

int field2(int instruction) {
	return(instruction & 0xFFFF);
}

int opcode(int instruction) {
	return(instruction>>22);
}

void printInstruction(int instr) {
	
	char opcodeString[10];
	
	if (opcode(instr) == ADD) {
		strcpy(opcodeString, "add");
	} else if (opcode(instr) == NAND) {
		strcpy(opcodeString, "nand");
	} else if (opcode(instr) == LW) {
		strcpy(opcodeString, "lw");
	} else if (opcode(instr) == SW) {
		strcpy(opcodeString, "sw");
	} else if (opcode(instr) == BEQ) {
		strcpy(opcodeString, "beq");
	} else if (opcode(instr) == JALR) {
		strcpy(opcodeString, "jalr");
	} else if (opcode(instr) == HALT) {
		strcpy(opcodeString, "halt");
	} else if (opcode(instr) == NOOP) {
		strcpy(opcodeString, "noop");
	} else {
		strcpy(opcodeString, "data");
    }
    printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr), field2(instr));
    return;
}

int main(int argc, char *argv[]) {
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++) {

        if (sscanf(line, "%d", state.instrMem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        state.dataMem[state.numMemory] = state.instrMem[state.numMemory];
    }

    /* Initialization */
    state.pc = 0;
    state.cycles = 0;
    int i;
    for (i = 0; i<NUMREGS; i++)
    	state.reg[i] = 0;
    state.IFID.instr = NOOPINSTRUCTION;
    state.IDEX.instr = NOOPINSTRUCTION;
    state.EXMEM.instr = NOOPINSTRUCTION;
    state.MEMWB.instr = NOOPINSTRUCTION;
    state.WBEND.instr = NOOPINSTRUCTION;

    /* Run */
    run(state);

    return 0;
}

int convertNum(int num) {
    /* convert a 16-bit number into a 32-bit integer */
    if (num & (1 << 15) ) {
        num -= (1 << 16);
    }
    return num;
}

void run(stateType state) {
	stateType newState;
	while (1) {

		printState(&state);

		/* check for halt */
		if (opcode(state.MEMWB.instr) == HALT) {
			printf("machine halted\n");
			printf("total of %d cycles executed\n", state.cycles);
			exit(0);
		}

		newState = state;
		newState.cycles++;

		/* --------------------- IF stage --------------------- */
		newState.IFID.pcPlus1 = state.pc + 1;
		newState.IFID.instr = state.instrMem[state.pc];
		newState.pc = state.pc + 1;
		if (opcode(state.EXMEM.instr)==BEQ) {
			newState.IFID.instr = state.instrMem[state.EXMEM.branchTarget];
			newState.pc = state.EXMEM.branchTarget + 1;
		}

		/* --------------------- ID stage --------------------- */
		newState.IDEX.instr = state.IFID.instr;
		newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
		if (opcode(state.IDEX.instr)==LW) {
			if (opcode(state.IFID.instr)==LW) {
				if (field1(state.IDEX.instr)==field0(state.IFID.instr)) {
					newState.IDEX.instr = NOOPINSTRUCTION;
					newState.pc = state.pc - 1;
				}
			} else if (opcode(state.IFID.instr)!=NOOP) {
				if (field1(state.IDEX.instr)==field0(state.IFID.instr) || field1(state.IDEX.instr)==field1(state.IFID.instr)) {
					newState.IDEX.instr = NOOPINSTRUCTION;
					newState.pc = state.pc - 1;
				}
			}
		}
		if (opcode(state.IFID.instr)!=NOOP) {
			newState.IDEX.readRegA = state.reg[field0(state.IFID.instr)];
			newState.IDEX.readRegB = state.reg[field1(state.IFID.instr)];
			if (opcode(state.IFID.instr)!=ADD && opcode(state.IFID.instr)!=NAND)
				newState.IDEX.offset = convertNum(field2(state.IFID.instr));
		}

		/* --------------------- EX stage --------------------- */
		newState.EXMEM.instr = state.IDEX.instr;

		int regA = state.IDEX.readRegA;
		int regB = state.IDEX.readRegB;
		if (opcode(state.WBEND.instr)==LW) {
			if (field0(state.IDEX.instr)==field1(state.WBEND.instr))
				regA = state.WBEND.writeData;
			if (field1(state.IDEX.instr)==field1(state.WBEND.instr))
				regB = state.WBEND.writeData;
		}
		if (opcode(state.MEMWB.instr)==LW) {
			if (field0(state.IDEX.instr)==field1(state.MEMWB.instr))
				regA = state.MEMWB.writeData;
			if (field1(state.IDEX.instr)==field1(state.MEMWB.instr))
				regB = state.MEMWB.writeData;
		}
		if (opcode(state.EXMEM.instr)==LW) {
			if (field0(state.IDEX.instr)==field1(state.EXMEM.instr))
				regA = state.EXMEM.aluResult;
			if (field1(state.IDEX.instr)==field1(state.EXMEM.instr))
				regB = state.EXMEM.aluResult;
		}
		if (opcode(state.WBEND.instr)==ADD || opcode(state.WBEND.instr)==NAND) {
			if (field0(state.IDEX.instr)==field2(state.WBEND.instr))
				regA = state.WBEND.writeData;
			if (field1(state.IDEX.instr)==field2(state.WBEND.instr))
				regB = state.WBEND.writeData;
		}
		if (opcode(state.MEMWB.instr)==ADD || opcode(state.MEMWB.instr)==NAND) {
			if (field0(state.IDEX.instr)==field2(state.MEMWB.instr))
				regA = state.MEMWB.writeData;
			if (field1(state.IDEX.instr)==field2(state.MEMWB.instr))
				regB = state.MEMWB.writeData;
		}
		if (opcode(state.EXMEM.instr)==ADD || opcode(state.EXMEM.instr)==NAND) {
			if (field0(state.IDEX.instr)==field2(state.EXMEM.instr))
				regA = state.EXMEM.aluResult;
			if (field1(state.IDEX.instr)==field2(state.EXMEM.instr))
				regB = state.EXMEM.aluResult;
		}

		newState.EXMEM.readRegB = regB;

		if (opcode(state.IDEX.instr)==ADD)
			newState.EXMEM.aluResult = regA + regB;
		else if (opcode(state.IDEX.instr)==NAND)
			newState.EXMEM.aluResult = ~(regA & regB);
		else if (opcode(state.IDEX.instr)==BEQ) {
			if ((regB-regA)==0) {
				newState.EXMEM.branchTarget = state.IDEX.pcPlus1 + state.IDEX.offset;
				newState.IDEX.instr = NOOPINSTRUCTION;
				newState.IFID.instr = NOOPINSTRUCTION;
			} else
				newState.EXMEM.branchTarget = state.pc + 1;
		} else if (opcode(state.IDEX.instr)!=NOOP)
			newState.EXMEM.aluResult = regA + state.IDEX.offset;

		/* --------------------- MEM stage --------------------- */
		newState.MEMWB.instr = state.EXMEM.instr;
		newState.MEMWB.writeData = state.EXMEM.aluResult;
		if (opcode(state.EXMEM.instr)==SW)
			newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.readRegB;
		if (opcode(state.EXMEM.instr)==LW)
			newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult];

		/* --------------------- WB stage --------------------- */
		newState.WBEND.instr = state.MEMWB.instr;
		newState.WBEND.writeData = state.MEMWB.writeData;
		if (opcode(state.MEMWB.instr)==LW)
			newState.reg[field1(state.MEMWB.instr)] = state.MEMWB.writeData;
		else if (opcode(state.MEMWB.instr)==ADD || opcode(state.MEMWB.instr)==NAND)
			newState.reg[field2(state.MEMWB.instr)] = state.MEMWB.writeData;

		state = newState;
	}	
}