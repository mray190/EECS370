start   add     1       2       1       decrement reg1
test 	nand    1       2       1       decrement reg1        
		lw      0       1       five    load reg1 with 5 (symbolic address)
        lw      1       2       3       load reg2 with -1 (numeric address)
        sw      0       1       five    save
        sw      1       2       3       save
        beq     0       0       start   go back to the beginning of the loop
        beq     0       1       2       goto end of program when reg1==0		
        jalr	0 		1				J-type command
        noop
done    halt                            end of program
five    .fill   5
neg1    .fill   -1
stAddr  .fill   start                   will contain the address of start (2)
