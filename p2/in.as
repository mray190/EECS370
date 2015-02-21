        lw      0       1       five    load reg 1 with 5
        lw      0       2       neg1    load reg 2 with -1
        beq		1		2		done	go to the end if reg 1 equals reg 2 (which it doesnt - so continue running)
start   add     1       2       3       add reg 1 to reg 2 and store to reg 3 (reg 3 should now be -4)
		sw		0		3		1		store reg 3 into memory address 1
		lw		0		4		neg1	load reg 4 with -1
		lw		0		6		test	load reg 6 with the address of label test
		jalr	6		5				reg 5 should now hold value 8 and execution should go to the beq line (line index 9)
		add 	1		1		7		add reg 1 to reg 1 and store to reg 7 (reg 7 should still b 0 because this line should never run due to the jalr above it)
addr	beq		2		4		done	go to the end if reg 2 equals reg 4 (which it does)
		add 	1		2		4		add reg 1 to reg 2 and store to reg 4 (reg 4 should still be -1 because this line should never run due to the beq above it)	
        noop
done    halt                            end of program
five    .fill   5
neg1    .fill   -1
one		.fill	1
test	.fill	addr
