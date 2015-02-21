#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
void getBinary(int* output, int in, int start, int end);
void getOpCode(int* output, char* in);
int toDecimal(int* input);

int main(int argc, char *argv[]) {
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    char labels[50][MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    /*////////////////////////////////////////////////////////////////////////
    //  My code below                                                       //
    ////////////////////////////////////////////////////////////////////////*/

    /* Go through and store all of the labels */
    int counter = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)==1) {

        /* Check for duplicate labels */
        int i;
        for (i=0; i<counter; i++) {
            if (strcmp(label, "") && strcmp(labels[i],label)==0) {
                printf("Error! Duplicate label!\n");
                exit(1);
            }
        }

        strcpy(labels[counter],label);
        counter++;
    }
    int total_labels = counter;
    
    /* Iterate through again to actually convert to binary/decimal
        Change the labels to actual address this time through */
    rewind(inFilePtr);
    int pc = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)==1) {

        /* Initialize the output array and set all elements to 0 to start */
        int output[32];
        int i;
        for (i=0; i<31; i++)
            output[i] = 0;

        /* Get the opcode and place into bits 24-22 (inclusive) */
        getOpCode(output, opcode);

        /* Check for the different types of instruction formats */
        /* R type */
        if (output[24]==0 & output[23]==0) {

            /* Convert the registers and offsets to binary and place them into the array */
            getBinary(output, atoi(arg0), 19, 21);
            getBinary(output, atoi(arg1), 16, 18);
            getBinary(output, atoi(arg2), 0 ,2);

        /* I type */
        } else if ((output[24]==0 & output[23]==1 & output[22]==0) ||
                    (output[24]==0 & output[23]==1 & output[22]==1) ||
                    (output[24]==1 & output[23]==0 & output[22]==0))   {

            /* Replace a label with corresponding address. If not, grab the offset */
            int offset;
            if (isNumber(arg2)!=1) {
                counter = 0;
                while (counter<total_labels && strcmp(labels[counter],arg2)!=0)
                    counter++;
                if (counter>=total_labels) {
                    printf("Error! Undefined label!\n");
                    exit(1);
                }
                if (output[24]==1 & output[23]==0 & output[22]==0)
                    offset = counter - pc - 1;
                else
                    offset = counter;
            } else {

                /* Check if the offset is greater/less than 2^16 */
                offset = atoi(arg2);
                if (offset<-32768 || offset>32767) {
                    printf("Error! Large offset!\n");
                    exit(1);
                }

            }

            /* Convert the registers and offsets to binary and place them into the array */
            int regA = atoi(arg0), regB = atoi(arg1);
            getBinary(output, regA, 19, 21);
            getBinary(output, regB, 16, 18);
            getBinary(output, offset, 0 ,15);

        /* J type */
        } else if (output[24]==1 & output[23]==0 & output[22]==1) {

            /* Convert the registers and offsets to binary and place them into the array */
            getBinary(output, atoi(arg0), 19, 21);
            getBinary(output, atoi(arg1), 16, 18);

        } 
        /* Do nothing for O and .fill commands */

        /* Converting the binary number back to decimal 
            For .fill commands, just use the original number */
        if (strcmp(opcode,".fill")==0) {

            /* Figure out if the value for .fill is a label or not */
            int offset = 0;
            if (isNumber(arg0)!=1) {
                int counter = 0;
                while (strcmp(labels[counter],arg0)!=0)
                    counter++;
                offset = counter;
            } else
                offset = atoi(arg0);
            fprintf(outFilePtr, "%d\n",offset);

        /* For any other opcode command, convert the binary number */
        } else {
            int result = toDecimal(output);
            fprintf(outFilePtr, "%d\n",result);
        }

        pc++;
    }

    return(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0, char *arg1, char *arg2) {
    char line[MAXLINELENGTH];
    char *ptr = line;
    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* reached end of file */
        return(0);
    }

    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label)) {
        /*if (strlen(label)>6) {
            printf("Error! Label too long!");
            exit(1);
        }*/
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]", opcode, arg0, arg1, arg2);
    return(1);
}

int isNumber(char *string) {
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

/**
* Converts a decimal number to a binary number of length 3
* @param: output is an integer array of size 16
* @param: in is an input signed number
*/
void getBinary(int* output, int in, int start, int end) {

    /* Store either all 1s or all 0s depending on the sign of the input number */
    int i;
    for (i=start; i<=end; i++) {
        if (in<0)
            output[i] = 1;
    }

    /* Figure out how much of the 16 bits will be used for the number */
    int bit_len;
	if (in<0) bit_len = ceil(log10(-1*in)/log10(2))+1;
	else bit_len = ceil(log10(in+1)/log10(2));
	
    /* Iterate through to calculate the binary number as an unsigned int */
    int total = abs(in);
	for (i=bit_len-1+start; i>=start; i--) {
		if (pow(2,i-start)<=total) {
            output[i] = 1;
			total -= pow(2,i-start);
		} else
            output[i] = 0;
	}

    /* If the number needs to be a negative, convert to 2s complement by flipping bits and adding 1 */
	if (in<0) {
		int carry = 1;
		for (i=start; i<bit_len; i++) {
			if (output[i]) {
				output[i] = carry;
				carry = 0;
			} else
				output[i] = !carry;
		}
	}
}

/**
* Converts the opcode to a binary representation
* @param: in is a string representing the opcode
* @return: string of length 3 of the opcode's binary representation
*/
void getOpCode(int* o, char* in) {
    int s = 24;
	if (strcmp(in,"add")==0) {
        o[s-0] = o[s-1] = o[s-2] = 0;
    } else if (strcmp(in,"nand")==0) {
		o[s-0] = o[s-1] = 0;
        o[s-2] = 1;
	} else if (strcmp(in,"lw")==0) {
        o[s-0] = o[s-2] = 0;
        o[s-1] = 1;
    } else if (strcmp(in,"sw")==0) {
        o[s-0] = 0;
        o[s-1] = o[s-2] = 1;
    } else if (strcmp(in,"beq")==0) {
        o[s-0] = 1;
        o[s-1] = o[s-2] = 0;
    } else if (strcmp(in,"jalr")==0) {
        o[s-0] = o[s-2] = 1;
        o[s-1] = 0;
    } else if (strcmp(in,"halt")==0) {
        o[s-0] = o[s-1] = 1;
        o[s-2] = 0;
    } else if (strcmp(in,"noop")==0) {
        o[s-0] = o[s-1] = o[s-2] = 1;
    } else if (strcmp(in,".fill")==0) {
        /* Do nothing - will be fixed in the next function */
    } else {
        printf("Error! Invalid opcode!\n");
		exit(1);
    }
}

int toDecimal(int* input) {
    int i;
    int sum = 0;
    for (i=0;i<32; i++)
        sum += input[i]*pow(2,i);
    return sum;
}