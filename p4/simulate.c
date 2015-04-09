/*
 * Instruction-level simulator for the LC
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000
#define MAXBLOCKSIZE 256
#define MAXBLOCKS 256

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5
#define HALT 6
#define NOOP 7

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

typedef struct setStruct {
	int dirty[MAXBLOCKSIZE];
	int valid[MAXBLOCKSIZE];
	int tag[MAXBLOCKSIZE];
	int data[MAXBLOCKSIZE];
	int LRUbits[MAXBLOCKSIZE];
	int LRU;
} setType;

typedef struct cacheStruct {
	setType sets[MAXBLOCKS];
	int SIZE;
	int blockSize;
    int numSets;
    int blocksPerSet;
} cacheType;

enum actionType {
	cacheToProcessor, processorToCache, memoryToCache, cacheToMemory, cacheToNowhere
};

void printState(stateType *);
void run(cacheType, stateType);
int convertNum(int);
void printAction(int, int, enum actionType);

void adjustLRU(cacheType *, int, int);
int load(cacheType *, int, stateType *);
void store(cacheType *, int, int, stateType *);

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

int convertNum(int num) {
    /* convert a 16-bit number into a 32-bit Sun integer */
    if (num & (1<<15) ) {
	num -= (1<<16);
    }
    return(num);
}

/*
 * Log the specifics of each cache action.
 *
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 * 	cacheToProcessor: reading data from the cache to the processor
 * 	processorToCache: writing data from the processor to the cache
 * 	memoryToCache: reading data from the memory to the cache
 * 	cacheToMemory: evicting cache data by writing it to the memory
 * 	cacheToNowhere: evicting cache data by throwing it away
 */
void printAction(int address, int size, enum actionType type) {
    printf("@@@ transferring word [%d-%d] ", address, address + size - 1);
    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    } else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    } else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    } else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    } else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
}

/*
cacheToProcessor, processorToCache, memoryToCache, cacheToMemory, cacheToNowhere

setStruct
	int dirty[MAXBLOCKSIZE];
	int valid[MAXBLOCKSIZE];
	int tag[MAXBLOCKSIZE];
	int data[MAXBLOCKSIZE];
	int LRUbits[MAXBLOCKSIZE];
	int LRU;

cacheStruct {
	setType sets[MAXBLOCKS];
	int SIZE;
	int blockSize;
    int numSets;
    int blocksPerSet; */

void adjustLRU(cacheType *cache, int set, int i) {
	cache->sets[set].LRUbits[i] = 0;
	int j;
	for (j=0; j<cache->blockSize; j++) cache->sets[set].LRUbits[j]++;
}

/**
 * Properly simulates the cache for a load from
 * memory address “addr”. Returns the loaded value.
 */
int load(cacheType *cache, int addr, stateType *state) {
	int set = (addr % (cache->blockSize * cache->numSets)) / cache->blockSize;
	int tag = addr / (cache->blockSize * cache->numSets);
	int block = ((int)(addr/cache->blockSize))*cache->blockSize;
	int offset = addr % cache->blockSize;

	/*
	printf("DEBUG - set: %d tag: %d block: %d offset: %d\n", set, tag, block, offset);
	*/

	int i;
	for (i=0; i < cache->blocksPerSet; i++) {

		/* Hit */
		if (cache->sets[set].valid[i]==1 && cache->sets[set].tag[i]==tag) {
			adjustLRU(cache, set, i);
			printAction(addr, 1, cacheToProcessor);
			return cache->sets[set].data[i*cache->blockSize + offset];

		/* Compulsory miss */
		} else if (cache->sets[set].valid[i]==0) {
			cache->sets[set].valid[i] = 1;
			cache->sets[set].tag[i] = tag;
			int j;
			for (j=0; j < cache->blockSize; j++)
				cache->sets[set].data[i*cache->blockSize + j] = state->mem[block + j];
			adjustLRU(cache, set, i);
			printAction(block, cache->blockSize, memoryToCache);
			printAction(addr, 1, cacheToProcessor);
			return cache->sets[set].data[i*cache->blockSize + offset];
		}
	}

	/* Determine LRU */
	int max = cache->sets[set].LRUbits[0];
	cache->sets[set].LRU = 0;
	for (i=0; i < cache->blocksPerSet; i++) {
		if (max < cache->sets[set].LRUbits[i]) {
			max = cache->sets[set].LRUbits[i];
			cache->sets[set].LRU = i;
		}
	}

	/* Write back */
	if (cache->sets[set].dirty[cache->sets[set].LRU]==1) {
		printAction(cache->blockSize*set+cache->sets[set].tag[cache->sets[set].LRU]*cache->blockSize*cache->numSets, cache->blockSize, cacheToMemory);
		for (i=0; i < cache->blockSize; i++)
			state->mem[block+i] = cache->sets[set].data[cache->blockSize*set+cache->sets[set].tag[cache->sets[set].LRU]*cache->blockSize*cache->numSets + i];
		cache->sets[set].dirty[cache->sets[set].LRU] = 0;
	} else {
		printAction(cache->blockSize*set+cache->sets[set].tag[cache->sets[set].LRU]*cache->blockSize*cache->numSets, cache->blockSize, cacheToNowhere);
	}

	/* Conflict miss */
	cache->sets[set].valid[cache->sets[set].LRU] = 1;
	cache->sets[set].tag[cache->sets[set].LRU] = tag;
	for (i=0; i < cache->blockSize; i++)
		cache->sets[set].data[cache->sets[set].LRU*cache->blockSize + i] = state->mem[block + i];
	adjustLRU(cache, set, cache->sets[set].LRU);
	printAction(block, cache->blockSize, memoryToCache);
	printAction(addr, 1, cacheToProcessor);
	return cache->sets[set].data[cache->sets[set].LRU*cache->blockSize + offset];
}

/**
 * Properly simulates the cache for a store 
 * to memory address “addr”. Returns nothing.
 */
void store(cacheType *cache, int addr, int data, stateType *state) {
	int set = (addr % (cache->blockSize * cache->numSets)) / cache->blockSize;
	int tag = addr / (cache->blockSize * cache->numSets);
	int block = ((int)(addr/cache->blockSize))*cache->blockSize;
	int offset = addr % cache->blockSize;

	int i;
	for (i=0; i < cache->blocksPerSet; i++) {

		/* Hit */
		if (cache->sets[set].valid[i]==1 && cache->sets[set].tag[i]==tag) {
			cache->sets[set].dirty[i] = 1;
			cache->sets[set].data[i*cache->blockSize + offset] = data;
			adjustLRU(cache, set, i);
			printAction(addr, 1, processorToCache);
			return;

		/* Compulsory miss */
		} else if (cache->sets[set].valid[i]==0) {
			cache->sets[set].valid[i] = 1;
			cache->sets[set].dirty[i] = 1;
			cache->sets[set].tag[i] = tag;
			int j;
			for (j=0; j < cache->blockSize; j++)
				cache->sets[set].data[i*cache->blockSize + j] = state->mem[block + j];
			cache->sets[set].data[i*cache->blockSize + offset] = data;
			adjustLRU(cache, set, i);
			printAction(block, cache->blockSize, memoryToCache);
			printAction(addr, 1, processorToCache);
			return;
		}
	}

	/* Determine LRU */
	int max = cache->sets[set].LRUbits[0];
	cache->sets[set].LRU = 0;
	for (i=0; i < cache->blocksPerSet; i++) {
		if (max<cache->sets[set].LRUbits[i]) {
			max = cache->sets[set].LRUbits[i];
			cache->sets[set].LRU = i;
		}
	}

	printf("DEBUG - store LRU: %d\n",cache->sets[set].LRU);

	/* Write back */
	if (cache->sets[set].dirty[cache->sets[set].LRU]==1) {
		printAction(cache->blockSize*set+cache->sets[set].tag[cache->sets[set].LRU]*cache->blockSize*cache->numSets, cache->blockSize, cacheToMemory);
		for (i=0; i < cache->blockSize; i++)
			state->mem[block+i] = cache->sets[set].data[cache->blockSize*set+cache->sets[set].tag[cache->sets[set].LRU]*cache->blockSize*cache->numSets + i];
		cache->sets[set].dirty[cache->sets[set].LRU] = 0;
	} else {
		printAction(cache->blockSize*set+cache->sets[set].tag[cache->sets[set].LRU]*cache->blockSize*cache->numSets, cache->blockSize, cacheToNowhere);
	}

	/* Conflict miss */
	cache->sets[set].valid[cache->sets[set].LRU] = 1;
	cache->sets[set].tag[cache->sets[set].LRU] = tag;
	cache->sets[set].dirty[cache->sets[set].LRU] = 1;
	for (i=0; i < cache->blockSize; i++)
		cache->sets[set].data[cache->sets[set].LRU*cache->blockSize + i] = state->mem[block + i];
	cache->sets[set].data[cache->sets[set].LRU*cache->blockSize + offset] = data;
	adjustLRU(cache, set, cache->sets[set].LRU);
	printAction(block, cache->blockSize, memoryToCache);
	printAction(addr, 1, processorToCache);
	return;
}

int main(int argc, char *argv[]) {
    int i;
    char line[MAXLINELENGTH];
    stateType state;
    cacheType cache;
    FILE *filePtr;

    if (argc != 5) {
		printf("error: usage: %s <machine-code file> <blockSizeInWords> <numberOfSets> <blocksPerSet>\n", argv[0]);
		exit(1);
    }

    cache.blockSize = atoi(argv[2]); /* Maximum 256 */
    cache.numSets = atoi(argv[3]);
    cache.blocksPerSet = atoi(argv[4]); /* 1 to blockSize */
    cache.SIZE = cache.blockSize * cache.numSets * cache.blocksPerSet;

    int j;
    for (i=0; i<MAXBLOCKS; i++) {
    	for (j=0; j<MAXBLOCKSIZE; j++) {
    		cache.sets[i].dirty[j] = 0;
    		cache.sets[i].valid[j] = 0;
    	}
    }

    /* initialize memories and registers */
    for (i=0; i<NUMMEMORY; i++)
		state.mem[i] = 0;
    for (i=0; i<NUMREGS; i++)
		state.reg[i] = 0;
    state.pc=0;

    /* read machine-code file into instruction/data memory (starting at
	address 0) */

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
		printf("error: can't open file %s\n", argv[1]);
		perror("fopen");
		exit(1);
    }

    for (state.numMemory=0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++) {
		if (state.numMemory >= NUMMEMORY) {
		    printf("exceeded memory size\n");
		    exit(1);
		}
		if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
		    printf("error in reading address %d\n", state.numMemory);
		    exit(1);
		}
    }
    
    /* run never returns */
    run(cache, state);

    return(0);
}

void run(cacheType cache, stateType state) {
    int arg0, arg1, arg2, addressField;
    int instructions=0;
    int opcode;
    int maxMem=-1;	/* highest memory address touched during run */

    for (; 1; instructions++) { /* infinite loop, exits when it executes halt */

		if (state.pc < 0 || state.pc >= NUMMEMORY) {
		    printf("pc went out of the memory range\n");
		    exit(1);
		}

		maxMem = (state.pc > maxMem)?state.pc:maxMem;

		int instruction = load(&cache, state.pc, &state);

		/* this is to make the following code easier to read */
		opcode = instruction >> 22;
		arg0 = (instruction >> 19) & 0x7;
		arg1 = (instruction >> 16) & 0x7;
		arg2 = instruction & 0x7; /* only for add, nand */

		addressField = convertNum(state.mem[state.pc] & 0xFFFF); /* for beq, lw, sw */
		state.pc++;

		if (opcode == ADD) {
		    state.reg[arg2] = state.reg[arg0] + state.reg[arg1];

		} else if (opcode == NAND) {
		    state.reg[arg2] = ~(state.reg[arg0] & state.reg[arg1]);

		} else if (opcode == LW) {
		    if (state.reg[arg0] + addressField < 0 || state.reg[arg0] + addressField >= NUMMEMORY) {
				printf("address out of bounds\n");
				exit(1);
		    }
		    state.reg[arg1] = load(&cache, state.reg[arg0] + addressField, &state);
		    if (state.reg[arg0] + addressField > maxMem)
				maxMem = state.reg[arg0] + addressField;

		} else if (opcode == SW) {
		    if (state.reg[arg0] + addressField < 0 || state.reg[arg0] + addressField >= NUMMEMORY) {
				printf("address out of bounds\n");
				exit(1);
		    }
		    store(&cache, state.reg[arg0] + addressField, state.reg[arg1], &state);
		    if (state.reg[arg0] + addressField > maxMem)
				maxMem = state.reg[arg0] + addressField;

		} else if (opcode == BEQ) {
		    if (state.reg[arg0] == state.reg[arg1])
				state.pc += addressField;

		} else if (opcode == JALR) {
		    state.reg[arg1] = state.pc;
            if(arg0 != 0)
	 			state.pc = state.reg[arg0];
		    else
				state.pc = 0;

		} else if (opcode == NOOP) {

		} else if (opcode == HALT) {
		    exit(0);

		} else {
		    printf("error: illegal opcode 0x%x\n", opcode);
		    exit(1);

		}
        state.reg[0] = 0;
    }
}