/* EECS 370 LC-2K Instruction-level simulator */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

typedef struct commandStruct {
    char* opcode;
    int regA;
    int regB;
    int dest;
    int offset;
} command;

void printState(stateType *);
char* convertOpcode(int a, int b, int c);
void convertDec(int* mem_bin, int sum);
int convertNum(int num);

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

    /*/////////////////////////////////////////////////////////////////////////
    // MY CODE BELOW                                                         //
    /////////////////////////////////////////////////////////////////////////*/

    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
            state.numMemory++) {

        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    /* Initialize program counter and registerse to 0 */
    state.pc = 0;
    int i;
    for (i=0; i<NUMREGS; i++)
        state.reg[i] = 0;

    /* Initialize and convert the input memory into the 32 bit binary string */
    int mem_bin[32];
    convertDec(mem_bin, state.mem[state.pc]);

    int execution = 0;

    char* opcode = convertOpcode(mem_bin[24], mem_bin[23], mem_bin[22]);
    while (strcmp(opcode, "halt")!=0) {

        execution++;

        /* Print out the original state */
        printState(&state);

        /* Insert the correct opcode into the command struct */
        command cc;
        cc.opcode = opcode;

        /* Store regA and regB register values */
        cc.regA = mem_bin[21]*4 + mem_bin[20]*2 + mem_bin[19];
        cc.regB = mem_bin[18]*4 + mem_bin[17]*2 + mem_bin[16];

        /* Execute differing commands based on the opcode
            R-type  */
        if (strcmp(cc.opcode, "add")==0 || strcmp(cc.opcode, "nand")==0) {

            /* Store regA, regB and dest register values */
            cc.dest = mem_bin[2]*4 + mem_bin[1]*2 + mem_bin[0];

            if (strcmp(cc.opcode, "add")==0)
                state.reg[cc.dest] = state.reg[cc.regA] + state.reg[cc.regB];
            else
                state.reg[cc.dest] = ~(state.reg[cc.regA] & state.reg[cc.regB]);

        /* I-type */
        } else if (strcmp(cc.opcode, "lw")==0 || strcmp(cc.opcode, "sw")==0 || strcmp(cc.opcode, "beq")==0) {
            
            int bit_c;
            int sum = pow(2,15)*mem_bin[15]*-1;
            for (bit_c=14; bit_c>=0; bit_c--)
                sum += mem_bin[bit_c]*pow(2,bit_c);
            cc.offset = sum;

            if (strcmp(cc.opcode, "lw")==0)
                state.reg[cc.regB] = state.mem[state.reg[cc.regA] + cc.offset];
            else if (strcmp(cc.opcode, "sw")==0)
                state.mem[state.reg[cc.regA] + cc.offset] = state.reg[cc.regB];
            else {
                if (state.reg[cc.regA]==state.reg[cc.regB])
                    state.pc += cc.offset;
            }

        /* J-type */
        } else if (strcmp(cc.opcode, "jalr")==0) {

            state.reg[cc.regB] = state.pc + 1;
            state.pc = state.reg[cc.regA] - 1;

        }

        state.pc++;

        convertDec(mem_bin, state.mem[state.pc]);
        opcode = convertOpcode(mem_bin[24], mem_bin[23], mem_bin[22]);
    }
 
    /* Print out the new state */
    printState(&state);
    state.pc++;

    printf("machine halted\ntotal of %d instructions executed\nfinal state of machine:\n", execution+1);

    /* Print out the new state */
    printState(&state);

    return(0);
}

void printState(stateType *statePtr) {
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

char* convertOpcode(int a, int b, int c) {
    if (!a && !b && !c)
        return "add";
    else if (!a && !b && c)
        return "nand";
    else if (!a && b && !c)
        return "lw";
    else if (!a && b && c)
        return "sw";
    else if (a && !b && !c)
        return "beq";
    else if (a && !b && c)
        return "jalr";
    else if (a && b && !c)
        return "halt";
    else if (a && b && c)
        return "noop";
    else {
        printf("Invalid opcode!");
        exit(1);
        return NULL;
    }
}

void convertDec(int* mem_bin, int sum) {
    /* Iterate through each bit */
    int i;
    for (i=31; i>=0; i--) {
        if (sum>=pow(2,i)) {
            mem_bin[i] = 1;
            sum -= pow(2,i);
        } else
            mem_bin[i] = 0;
    }
}

int convertNum(int num) {
    /* convert a 16-bit number into a 32-bit Linux integer */
    if (num & (1<<15) )
        num -= (1<<16);
    return(num);
}