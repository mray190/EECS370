		lw		0		2		mcand		Insert the mcand into reg 1
		lw		0		3		mplier		Insert the mplier into reg 2
		lw		0		4		one			Insert the #1 into reg 4
		lw		0		6 		end			Load a integer size into reg 6
iter	beq		4		6 		final
		nand	4		3		5			Nand the iterator (reg 3) with the mplier into reg 4
		nand	5		5		5			Nand reg 4 with itself
		beq		0		5		off
		add 	2		1		1
off 	add 	2		2		2
		add		4		4		4
		beq		0		0		iter		Repeat the process
final	halt
mcand	.fill	3
mplier	.fill	7
one		.fill	1
end		.fill	65536
